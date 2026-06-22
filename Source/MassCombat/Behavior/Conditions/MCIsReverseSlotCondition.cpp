#include "Behavior/Conditions/MCIsReverseSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCIsReverseSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCIsReverseSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bReverse = Combat.bReverseSlot != 0;
	return Data.bInvert ? !bReverse : bReverse;
}
