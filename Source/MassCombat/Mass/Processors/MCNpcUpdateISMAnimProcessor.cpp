#include "Mass/Processors/MCNpcUpdateISMAnimProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"

UMCNpcUpdateISMAnimProcessor::UMCNpcUpdateISMAnimProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcAnimStateProcessor"));
}

void UMCNpcUpdateISMAnimProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UMCNpcUpdateISMAnimProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	Super::Execute(EntityManager, Context);
}
