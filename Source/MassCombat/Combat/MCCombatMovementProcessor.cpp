#include "Combat/MCCombatMovementProcessor.h"
#include "Targeting/MCTargetingFragments.h"
#include "Movement/MCOrientationFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

UMCCombatMovementProcessor::UMCCombatMovementProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCCombatMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCTargetingFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCOrientationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

void UMCCombatMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
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
		const TConstArrayView<FMCTargetingFragment> Targetings = Ctx.GetFragmentView<FMCTargetingFragment>();
		const TConstArrayView<FMCOrientationFragment> Orientations = Ctx.GetFragmentView<FMCOrientationFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Ctx.GetMutableFragmentView<FMassMoveTargetFragment>();
		const bool bHasMoveTarget = MoveTargets.Num() > 0;

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			const FMCTargetingFragment& Targeting = Targetings[i];
			const FMCOrientationFragment& Orientation = Orientations[i];

			if (Orientation.bMovementBlocked && bHasMoveTarget)
			{
				FMassMoveTargetFragment& MoveTarget = MoveTargets[i];
				if (MoveTarget.GetCurrentAction() != EMassMovementAction::Animate)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Animate, *World);
					MoveTarget.DesiredSpeed.Set(0.f);
				}
			}

			if (Orientation.FaceTargetEndTime > Now && Targeting.bHasTarget)
			{
				FTransform& Xf = Transforms[i].GetMutableTransform();

				FVector ToTarget = Targeting.TargetLocation - Xf.GetLocation();
				ToTarget.Z = 0.f;
				if (!ToTarget.IsNearlyZero())
				{
					const FQuat DesiredQ = ToTarget.Rotation().Quaternion();
					const float Remaining = Orientation.FaceTargetEndTime - Now;
					const float Alpha = (Remaining > DeltaTime) ? (DeltaTime / Remaining) : 1.f;
					const FQuat NewQ = FQuat::Slerp(Xf.GetRotation(), DesiredQ, Alpha).GetNormalized();
					Xf.SetRotation(NewQ);

					if (bHasMoveTarget)
					{
						MoveTargets[i].Forward = NewQ.GetForwardVector();
					}
				}
			}
			else if (Orientation.bLookAtNearestEnemy && Targeting.bHasNearestEnemy)
			{
				FTransform& Xf = Transforms[i].GetMutableTransform();

				FVector ToEnemy = Targeting.NearestEnemyLocation - Xf.GetLocation();
				ToEnemy.Z = 0.f;
				if (!ToEnemy.IsNearlyZero())
				{
					const FRotator NewRot = FMath::RInterpConstantTo(Xf.Rotator(), ToEnemy.Rotation(), DeltaTime, Orientation.LookAtTurnRate);
					const FQuat NewQ = NewRot.Quaternion();
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
