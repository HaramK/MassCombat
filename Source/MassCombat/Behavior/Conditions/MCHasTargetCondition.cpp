#include "Behavior/Conditions/MCHasTargetCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCHasTargetCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TargetingHandle);
	return true;
}

bool FMCHasTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCTargetingFragment& Targeting = Context.GetExternalData(TargetingHandle);
	const bool bHas = Targeting.bHasTarget != 0;
	return Data.bInvert ? !bHas : bHas;
}
