#include "Combat/MCDeathProcessor.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

UMCDeathProcessor::UMCDeathProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCCombatFragment>(EMassFragmentAccess::ReadOnly);
}

void UMCDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
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
