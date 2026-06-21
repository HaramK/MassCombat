#include "Mass/StateTree/MCNpcHasTargetCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCNpcHasTargetCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCNpcHasTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bHas = Combat.bHasTarget != 0;
	return Data.bInvert ? !bHas : bHas;
}
