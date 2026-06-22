#include "Behavior/Conditions/MCHasSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCHasSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCHasSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bHas = Combat.bHasTarget != 0 && Combat.SlotIndex != INDEX_NONE;
	return Data.bInvert ? !bHas : bHas;
}
