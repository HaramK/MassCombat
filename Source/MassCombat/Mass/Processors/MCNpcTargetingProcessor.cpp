#include "Mass/Processors/MCNpcTargetingProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

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
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
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
		const TArrayView<FMCNpcCombatFragment> Combats = Ctx.GetMutableFragmentView<FMCNpcCombatFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCNpcCombatFragment& Combat = Combats[i];
			const FVector Location = Transforms[i].GetTransform().GetLocation();

			Combat.bHasTarget = bHasPlayer;
			Combat.TargetLocation = PlayerLocation;
			Combat.DistanceToTarget = bHasPlayer ? FVector::Dist(Location, PlayerLocation) : TNumericLimits<float>::Max();

			Combat.AttackCooldownRemaining = FMath::Max(0.f, Combat.AttackCooldownRemaining - DeltaTime);
		}
	});
}
