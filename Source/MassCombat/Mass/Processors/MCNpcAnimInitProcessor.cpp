#include "Mass/Processors/MCNpcAnimInitProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"

UMCNpcAnimInitProcessor::UMCNpcAnimInitProcessor()
	: EntityQuery(*this)
{
	ObservedType = FMCNpcAnimStateFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
}

void UMCNpcAnimInitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCNpcAnimInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float GlobalTime = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [GlobalTime](FMassExecutionContext& Ctx)
	{
		const TArrayView<FMCNpcAnimStateFragment> Anims = Ctx.GetMutableFragmentView<FMCNpcAnimStateFragment>();
		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			// 시작 위상을 무작위로 당겨 인스턴스마다 다른 프레임에서 시작하게 한다
			Anims[i].GlobalStartTime = GlobalTime - FMath::FRandRange(0.f, 10.f);
		}
	});
}
