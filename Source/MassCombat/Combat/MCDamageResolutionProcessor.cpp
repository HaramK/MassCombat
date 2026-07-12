#include "Combat/MCDamageResolutionProcessor.h"
#include "Combat/MCDamageSubsystem.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "Engine/World.h"

UMCDamageResolutionProcessor::UMCDamageResolutionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MCTargetingProcessor"));
	bRequiresGameThreadExecution = true;
}

void UMCDamageResolutionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCEngagementFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCDamageResolutionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	UMCDamageSubsystem* DamageSubsystem = World ? World->GetSubsystem<UMCDamageSubsystem>() : nullptr;
	if (!DamageSubsystem)
	{
		return;
	}

	TArray<FMCDamageEvent> Events;
	DamageSubsystem->ConsumeEvents(Events);
	if (Events.Num() == 0)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();

	TArray<FMassEntityHandle> DamagedEntities;
	DamagedEntities.Reserve(Events.Num());

	for (const FMCDamageEvent& Event : Events)
	{
		if (!EntityManager.IsEntityValid(Event.Target))
		{
			continue;
		}

		FMCUnitFragment* Unit = EntityManager.GetFragmentDataPtr<FMCUnitFragment>(Event.Target);
		if (!Unit || Unit->Health <= 0.f)
		{
			continue;
		}
		Unit->Health -= Event.Amount;

		if (FMCEngagementFragment* Engagement = EntityManager.GetFragmentDataPtr<FMCEngagementFragment>(Event.Target))
		{
			Engagement->LastAttackerUnit = Event.Instigator;
			Engagement->LastDamagedTime = Now;
		}

		DamagedEntities.Add(Event.Target);
	}

	if (DamagedEntities.Num() > 0)
	{
		if (UMassSignalSubsystem* SignalSubsystem = World->GetSubsystem<UMassSignalSubsystem>())
		{
			SignalSubsystem->SignalEntities(UE::Mass::Signals::NewStateTreeTaskRequired, DamagedEntities);
		}
	}
}
