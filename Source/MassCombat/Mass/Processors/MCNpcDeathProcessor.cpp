#include "Mass/Processors/MCNpcDeathProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "Mass/Fragments/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

UMCNpcDeathProcessor::UMCNpcDeathProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCNpcDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadOnly);
}

void UMCNpcDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FMCUnitFragment> Units = Ctx.GetFragmentView<FMCUnitFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Units[i].Health <= 0.f)
			{
				Ctx.Defer().DestroyEntity(Ctx.GetEntity(i));
			}
		}
	});
}
