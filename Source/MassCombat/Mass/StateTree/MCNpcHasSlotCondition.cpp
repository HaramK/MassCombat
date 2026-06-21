#include "Mass/StateTree/MCNpcHasSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCNpcHasSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCNpcHasSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bHas = Combat.bHasTarget != 0 && Combat.SlotIndex != INDEX_NONE;
	return Data.bInvert ? !bHas : bHas;
}
