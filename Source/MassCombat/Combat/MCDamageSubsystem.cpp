#include "Combat/MCDamageSubsystem.h"

void UMCDamageSubsystem::QueueDamage(FMassEntityHandle Target, FMassEntityHandle Instigator, float Amount)
{
	if (Target.IsSet() && Amount > 0.f)
	{
		PendingEvents.Add({ Target, Instigator, Amount });
	}
}

void UMCDamageSubsystem::ConsumeEvents(TArray<FMCDamageEvent>& OutEvents)
{
	OutEvents.Reset();
	Swap(OutEvents, PendingEvents);
}
