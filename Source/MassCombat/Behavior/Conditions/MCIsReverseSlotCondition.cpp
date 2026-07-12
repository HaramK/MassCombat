#include "Behavior/Conditions/MCIsReverseSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCIsReverseSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TargetingHandle);
	return true;
}

bool FMCIsReverseSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCTargetingFragment& Targeting = Context.GetExternalData(TargetingHandle);
	const bool bReverse = Targeting.bReverseSlot != 0;
	return Data.bInvert ? !bReverse : bReverse;
}
