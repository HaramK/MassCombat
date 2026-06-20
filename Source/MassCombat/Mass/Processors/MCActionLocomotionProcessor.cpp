#include "Mass/Processors/MCActionLocomotionProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

UMCActionLocomotionProcessor::UMCActionLocomotionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCActionLocomotionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

void UMCActionLocomotionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (!World)
	{
		return;
	}
	const float DeltaTime = World->GetDeltaSeconds();
	const float Now = World->GetTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context, [World, DeltaTime, Now](FMassExecutionContext& Ctx)
	{
		const TArrayView<FTransformFragment> Transforms = Ctx.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FMCNpcCombatFragment> Combats = Ctx.GetFragmentView<FMCNpcCombatFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
		const bool bHasMoveTarget = MoveTargets.Num() > 0;

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			const FMCNpcCombatFragment& Combat = Combats[i];

			if (Combat.bMovementBlocked && bHasMoveTarget)
			{
				FMassMoveTargetFragment& MoveTarget = MoveTargets[i];
				if (MoveTarget.GetCurrentAction() != EMassMovementAction::Animate)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Animate, *World);
					MoveTarget.DesiredSpeed.Set(0.f);
				}
			}

			if (Combat.FaceTargetEndTime > 0.f && Combat.bHasTarget)
			{
				FTransform& Xf = Transforms[i].GetMutableTransform();

				FVector ToTarget = Combat.TargetLocation - Xf.GetLocation();
				ToTarget.Z = 0.f;
				if (!ToTarget.IsNearlyZero())
				{
					const FQuat DesiredQ = ToTarget.Rotation().Quaternion();
					const float Remaining = Combat.FaceTargetEndTime - Now;
					const float Alpha = (Remaining > DeltaTime) ? (DeltaTime / Remaining) : 1.f;
					const FQuat NewQ = FQuat::Slerp(Xf.GetRotation(), DesiredQ, Alpha).GetNormalized();
					Xf.SetRotation(NewQ);

					if (bHasMoveTarget)
					{
						MoveTargets[i].Forward = NewQ.GetForwardVector();
					}
				}
			}
		}
	});
}
