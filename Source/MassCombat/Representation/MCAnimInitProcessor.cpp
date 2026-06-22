#include "Representation/MCAnimInitProcessor.h"
#include "Combat/MCCombatFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"

UMCAnimInitProcessor::UMCAnimInitProcessor()
	: EntityQuery(*this)
{
	ObservedType = FMCAnimStateFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
}

void UMCAnimInitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCAnimStateFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCAnimInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float GlobalTime = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [GlobalTime](FMassExecutionContext& Ctx)
	{
		const TArrayView<FMCAnimStateFragment> Anims = Ctx.GetMutableFragmentView<FMCAnimStateFragment>();
		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			// Pull the start phase back by a random amount so each instance begins on a different frame.
			Anims[i].GlobalStartTime = GlobalTime - FMath::FRandRange(0.f, 10.f);
		}
	});
}
