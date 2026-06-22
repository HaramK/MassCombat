#include "Behavior/Conditions/MCDamageTakenCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCDamageTakenCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCDamageTakenCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	// New, not-yet-reacted hit: damage stamped after the last hit-react was started.
	const bool bDamaged = Combat.LastDamagedTime > Combat.LastHitReactTime;
	return Data.bInvert ? !bDamaged : bDamaged;
}
