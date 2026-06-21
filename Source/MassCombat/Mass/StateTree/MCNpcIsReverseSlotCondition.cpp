#include "Mass/StateTree/MCNpcIsReverseSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCNpcIsReverseSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCNpcIsReverseSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bReverse = Combat.bReverseSlot != 0;
	return Data.bInvert ? !bReverse : bReverse;
}
