#include "Behavior/Conditions/MCDamageTakenCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCDamageTakenCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EngagementHandle);
	return true;
}

bool FMCDamageTakenCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCEngagementFragment& Engagement = Context.GetExternalData(EngagementHandle);
	// New, not-yet-reacted hit: damage stamped after the last hit-react was started.
	const bool bDamaged = Engagement.LastDamagedTime > Engagement.LastHitReactTime;
	return Data.bInvert ? !bDamaged : bDamaged;
}
