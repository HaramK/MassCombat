#include "Combat/MCDeathProcessor.h"
#include "Combat/MCCombatFragments.h"
#include "Combat/MCCombatEventsSubsystem.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UMCDeathProcessor::UMCDeathProcessor()
	: DetectNpcQuery(*this)
	, DetectPlayerQuery(*this)
	, CleanupQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	DetectNpcQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	DetectNpcQuery.AddRequirement<FMCDeathFragment>(EMassFragmentAccess::ReadWrite);
	DetectNpcQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);

	DetectPlayerQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	DetectPlayerQuery.AddTagRequirement<FMCPlayerTag>(EMassFragmentPresence::All);
	DetectPlayerQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);

	CleanupQuery.AddRequirement<FMCDeathFragment>(EMassFragmentAccess::ReadOnly);
	CleanupQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::All);
}

void UMCDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const float Duration = DeathDuration;

	DetectNpcQuery.ForEachEntityChunk(Context, [Now, Duration](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FMCUnitFragment> Units = Ctx.GetFragmentView<FMCUnitFragment>();
		const TArrayView<FMCDeathFragment> Deaths = Ctx.GetMutableFragmentView<FMCDeathFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Units[i].Health <= 0.f)
			{
				Deaths[i].DestroyTime = Now + Duration;
				Ctx.Defer().AddTag<FMCDeadTag>(Ctx.GetEntity(i));
			}
		}
	});

	UMCCombatEventsSubsystem* Events = World ? World->GetSubsystem<UMCCombatEventsSubsystem>() : nullptr;
	DetectPlayerQuery.ForEachEntityChunk(Context, [Events](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FMCUnitFragment> Units = Ctx.GetFragmentView<FMCUnitFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Units[i].Health <= 0.f)
			{
				if (Events)
				{
					Events->OnPlayerDied.Broadcast();
				}
				Ctx.Defer().AddTag<FMCDeadTag>(Ctx.GetEntity(i));
			}
		}
	});

	CleanupQuery.ForEachEntityChunk(Context, [Now](FMassExecutionContext& Ctx)
	{
		const TConstArrayView<FMCDeathFragment> Deaths = Ctx.GetFragmentView<FMCDeathFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Now >= Deaths[i].DestroyTime)
			{
				Ctx.Defer().DestroyEntity(Ctx.GetEntity(i));
			}
		}
	});
}
