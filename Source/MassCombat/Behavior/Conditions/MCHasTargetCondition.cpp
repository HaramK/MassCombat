#include "Behavior/Conditions/MCHasTargetCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCHasTargetCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCHasTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const bool bHas = Combat.bHasTarget != 0;
	return Data.bInvert ? !bHas : bHas;
}
