#include "Mass/Processors/MCNpcTargetingProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassActorSubsystem.h"
#include "MCNpcAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UMCNpcTargetingProcessor::UMCNpcTargetingProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCNpcTargetingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FMCNpcCombatParams>();
}

void UMCNpcTargetingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (!World)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	const bool bHasPlayer = PlayerPawn != nullptr;
	const FVector PlayerLocation = bHasPlayer ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	const float DeltaTime = World->GetDeltaSeconds();

	EntityQuery.ForEachEntityChunk(Context, [bHasPlayer, PlayerLocation, DeltaTime](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Ctx.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMCNpcCombatFragment> Combats = Ctx.GetMutableFragmentView<FMCNpcCombatFragment>();
		const TArrayView<FMassActorFragment> Actors = Ctx.GetMutableFragmentView<FMassActorFragment>();
		const bool bHasActors = Actors.Num() > 0;

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCNpcCombatFragment& Combat = Combats[i];
			const FTransform& Xform = Transforms[i].GetTransform();
			const FVector Location = Xform.GetLocation();

			Combat.bHasTarget = bHasPlayer;
			Combat.TargetLocation = PlayerLocation;
			Combat.DistanceToTarget = bHasPlayer ? FVector::Dist(Location, PlayerLocation) : TNumericLimits<float>::Max();

			Combat.AttackCooldownRemaining = FMath::Max(0.f, Combat.AttackCooldownRemaining - DeltaTime);

			if (bHasActors)
			{
				if (ACharacter* Character = Cast<ACharacter>(Actors[i].GetMutable()))
				{
					if (UMCNpcAnimInstance* AnimInstance = Cast<UMCNpcAnimInstance>(Character->GetMesh()->GetAnimInstance()))
					{
						const FVector CurrentVelocity = Velocities[i].Value;
						AnimInstance->Velocity = CurrentVelocity;
						AnimInstance->GroundSpeed = CurrentVelocity.Size2D();
						AnimInstance->ActorRotation = Xform.Rotator();
					}
				}
			}
		}
	});
}
