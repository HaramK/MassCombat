#include "Mass/Processors/MCStateTreeDebugProcessor.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassStateTreeFragments.h"
#include "MassStateTreeSubsystem.h"
#include "StateTree.h"
#include "StateTreeInstanceData.h"
#include "StateTreeExecutionTypes.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawStateTreeState(
	TEXT("mc.DrawStateTreeState"),
	false,
	TEXT("Draw the current StateTree active state name above each entity."));

UMCStateTreeDebugProcessor::UMCStateTreeDebugProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCStateTreeDebugProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassStateTreeInstanceFragment>(EMassFragmentAccess::ReadOnly);
}

void UMCStateTreeDebugProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (!CVarDrawStateTreeState.GetValueOnGameThread())
	{
		return;
	}

	UWorld* World = EntityManager.GetWorld();
	if (!World)
	{
		return;
	}

	UMassStateTreeSubsystem* Subsystem = World->GetSubsystem<UMassStateTreeSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	EntityQuery.ForEachEntityChunk(Context, [World, Subsystem](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassStateTreeInstanceFragment> Instances = Ctx.GetFragmentView<FMassStateTreeInstanceFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FStateTreeInstanceData* InstanceData = Subsystem->GetInstanceData(Instances[i].InstanceHandle);
			if (!InstanceData)
			{
				continue;
			}

			const FStateTreeExecutionState* Exec = InstanceData->GetExecutionState();
			if (!Exec || Exec->ActiveFrames.Num() == 0)
			{
				continue;
			}

			const FStateTreeExecutionFrame& Frame = Exec->ActiveFrames.Last();
			if (!Frame.StateTree || Frame.ActiveStates.Num() == 0)
			{
				continue;
			}

			const FCompactStateTreeState* State = Frame.StateTree->GetStateFromHandle(Frame.ActiveStates.Last());
			if (!State)
			{
				continue;
			}

			const FVector Location = Transforms[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 110.f);
			DrawDebugString(World, Location, State->Name.ToString(), nullptr, FColor::White, 0.f, true);
		}
	});
}
