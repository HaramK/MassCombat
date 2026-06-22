#include "Unit/MCUnitInitProcessor.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"

UMCUnitInitProcessor::UMCUnitInitProcessor()
	: EntityQuery(*this)
{
	ObservedType = FMCUnitFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
}

void UMCUnitInitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();
}

void UMCUnitInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
	{
		const float MaxHealth = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().MaxHealth;
		const TArrayView<FMCUnitFragment> Units = Ctx.GetMutableFragmentView<FMCUnitFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			Units[i].Health = MaxHealth;
		}
	});
}
