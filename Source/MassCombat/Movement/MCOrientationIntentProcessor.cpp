#include "Movement/MCOrientationIntentProcessor.h"
#include "Debug/MCStats.h"
#include "Movement/MCOrientationFragments.h"
#include "Targeting/MCTargetingFragments.h"
#include "Combat/MCCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"

UMCOrientationIntentProcessor::UMCOrientationIntentProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCOrientationIntentProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCTargetingFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCOrientationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

DECLARE_CYCLE_STAT(TEXT("OrientationIntent Execute"), STAT_MC_OrientationExecute, STATGROUP_MassCombat);

void UMCOrientationIntentProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_MC_OrientationExecute);

	UWorld* World = EntityManager.GetWorld();
	if (!World)
	{
		return;
	}
	const float DeltaTime = World->GetDeltaSeconds();
	const float Now = World->GetTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context, [DeltaTime, Now](FMassExecutionContext& Ctx)
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

			const bool bFaceTarget = Orientation.FaceTargetEndTime > Now && Targeting.bHasTarget;
			const bool bLookAt = !bFaceTarget && Orientation.bLookAtNearestEnemy
				&& (Targeting.bHasTarget || Targeting.bHasNearestEnemy);
			if (!bFaceTarget && !bLookAt)
			{
				continue;
			}

			FTransform& Xf = Transforms[i].GetMutableTransform();
			const FVector Anchor = bFaceTarget ? Targeting.TargetLocation
				: (Targeting.bHasTarget ? Targeting.TargetLocation : Targeting.NearestEnemyLocation);
			FVector ToAnchor = Anchor - Xf.GetLocation();
			ToAnchor.Z = 0.f;
			if (ToAnchor.IsNearlyZero())
			{
				continue;
			}

			const bool bExternallyDriven = !bHasMoveTarget
				|| MoveTargets[i].GetCurrentAction() == EMassMovementAction::Animate;

			if (bExternallyDriven)
			{
				FQuat NewQ;
				if (bFaceTarget)
				{
					const FQuat DesiredQ = ToAnchor.Rotation().Quaternion();
					const float Remaining = Orientation.FaceTargetEndTime - Now;
					const float Alpha = (Remaining > DeltaTime) ? (DeltaTime / Remaining) : 1.f;
					NewQ = FQuat::Slerp(Xf.GetRotation(), DesiredQ, Alpha).GetNormalized();
				}
				else
				{
					const FRotator NewRot = FMath::RInterpConstantTo(Xf.Rotator(), ToAnchor.Rotation(), DeltaTime, Orientation.LookAtTurnRate);
					NewQ = NewRot.Quaternion();
				}

				Xf.SetRotation(NewQ);
				if (bHasMoveTarget)
				{
					MoveTargets[i].Forward = NewQ.GetForwardVector();
				}
			}
			else
			{
				MoveTargets[i].Forward = ToAnchor.GetSafeNormal();
			}
		}
	});
}
