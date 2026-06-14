#include "Mass/Processors/MCNpcAnimStateProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "MassActorSubsystem.h"

UMCNpcAnimStateProcessor::UMCNpcAnimStateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCNpcAnimStateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FMCNpcCombatParams>();
}

void UMCNpcAnimStateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float GlobalTime = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [GlobalTime](FMassExecutionContext& Ctx)
	{
		const FMCNpcCombatParams& Params = Ctx.GetConstSharedFragment<FMCNpcCombatParams>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Ctx.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMCNpcAnimStateFragment> Anims = Ctx.GetMutableFragmentView<FMCNpcAnimStateFragment>();
		const TConstArrayView<FMassRepresentationFragment> Reps = Ctx.GetFragmentView<FMassRepresentationFragment>();
		const TArrayView<FMassActorFragment> Actors = Ctx.GetMutableFragmentView<FMassActorFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCNpcAnimStateFragment& Anim = Anims[i];

			if (!Anim.AnimData.IsValid() && !Params.DefaultAnimData.IsNull())
			{
				Anim.AnimData = Params.DefaultAnimData.LoadSynchronous();
			}

			const int32 PrevState = Anim.StateIndex;

			if (Anim.bAttacking)
			{
				Anim.StateIndex = Params.AttackStateIndex;
				Anim.PlayRate = 1.f;
			}
			else if (Velocities[i].Value.SizeSquared() > Params.WalkSpeedThresholdSq)
			{
				Anim.StateIndex = Params.WalkStateIndex;
				Anim.PlayRate = FMath::Clamp(Velocities[i].Value.Size() / Params.WalkAnimReferenceSpeed, Params.MinWalkPlayRate, Params.MaxWalkPlayRate);
			}
			else
			{
				Anim.StateIndex = Params.IdleStateIndex;
				Anim.PlayRate = 1.f;
			}

			if (Anim.StateIndex != PrevState)
			{
				Anim.GlobalStartTime = GlobalTime;

				if (Anim.bAttacking
					&& Reps[i].CurrentRepresentation == EMassRepresentationType::HighResSpawnedActor
					&& Actors.Num() > 0)
				{
					if (ACharacter* Character = Cast<ACharacter>(Actors[i].GetMutable()))
					{
						if (UMCNpcAnimInstance* AnimInstance = Cast<UMCNpcAnimInstance>(Character->GetMesh()->GetAnimInstance()))
						{
							AnimInstance->PlayAttackMontage(Params.AttackMontage);
						}
					}
				}
			}
		}
	});
}
