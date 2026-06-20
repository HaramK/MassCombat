#include "Mass/Actions/MCActionProcessor.h"
#include "Mass/Actions/MCActionFragments.h"
#include "Mass/Actions/MCActionDef.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UMCActionProcessor::UMCActionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCActionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCActionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMCActionSetParams>();
}

void UMCActionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [Now](FMassExecutionContext& Ctx)
	{
		const FMCActionSetParams& Set = Ctx.GetConstSharedFragment<FMCActionSetParams>();
		const TArrayView<FMCActionFragment> Frags = Ctx.GetMutableFragmentView<FMCActionFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 e = 0; e < Num; ++e)
		{
			FMCActionFragment& Frag = Frags[e];

			const int32 Count = FMath::Min(Frag.Runtimes.Num(), Set.Actions.Num());
			for (int32 i = 0; i < Count; ++i)
			{
				FMCActionRuntime& RT = Frag.Runtimes[i];

				if (RT.ActiveUntil > 0.f && Now >= RT.ActiveUntil)
				{
					if (const UMCActionDef* Def = Set.Actions[i])
					{
						Frag.ActiveTags.RemoveTags(Def->GrantsTags);

						if (Def->CooldownTime > 0.f)
						{
							RT.CooldownEnd = Now + Def->CooldownTime;
						}
					}

					RT.ActiveUntil = 0.f;
				}
			}
		}
	});
}
