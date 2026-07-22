#include "Debug/MCTargetingDebugProcessor.h"
#include "Targeting/MCTargetingFragments.h"
#include "Combat/MCCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawTargetingState(
	TEXT("mc.DrawTargetingState"),
	false,
	TEXT("Color-code each entity's targeting state (blue=slot, cyan=target only, yellow=aware, grey=blind) and show totals on screen."));

UMCTargetingDebugProcessor::UMCTargetingDebugProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCTargetingDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCTargetingFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);
}

void UMCTargetingDebugProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (!CVarDrawTargetingState.GetValueOnGameThread())
	{
		return;
	}

	UWorld* World = EntityManager.GetWorld();
	if (!World)
	{
		return;
	}

	int32 Total = 0, Slot = 0, TargetOnly = 0, Aware = 0, Blind = 0;

	EntityQuery.ForEachEntityChunk(Context, [World, &Total, &Slot, &TargetOnly, &Aware, &Blind](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMCTargetingFragment> Targetings = Ctx.GetFragmentView<FMCTargetingFragment>();

		const int32 Num = Ctx.GetNumEntities();
		Total += Num;
		for (int32 i = 0; i < Num; ++i)
		{
			const FMCTargetingFragment& Targeting = Targetings[i];

			FColor Color;
			if (Targeting.bHasTarget && Targeting.SlotIndex != INDEX_NONE)
			{
				Color = FColor(0, 110, 255);
				++Slot;
			}
			else if (Targeting.bHasTarget)
			{
				Color = FColor(90, 220, 255);
				++TargetOnly;
			}
			else if (Targeting.bHasNearestEnemy)
			{
				Color = FColor(255, 220, 0);
				++Aware;
			}
			else
			{
				Color = FColor(150, 150, 150);
				++Blind;
			}

			const FVector Base = Transforms[i].GetTransform().GetLocation();
			DrawDebugLine(World, Base + FVector(0.f, 0.f, 100.f), Base + FVector(0.f, 0.f, 180.f), Color, false, 0.f, 0, 6.f);
		}
	});

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0x4D435447, 1.f, FColor::Cyan, FString::Printf(
			TEXT("[MC Targeting] total=%d | slot=%d target-only=%d aware=%d blind=%d"),
			Total, Slot, TargetOnly, Aware, Blind));
	}
}
