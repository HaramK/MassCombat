#include "Behavior/Conditions/MCHasSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCHasSlotCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TargetingHandle);
	return true;
}

bool FMCHasSlotCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCTargetingFragment& Targeting = Context.GetExternalData(TargetingHandle);
	const bool bHas = Targeting.bHasTarget != 0 && Targeting.SlotIndex != INDEX_NONE;
	return Data.bInvert ? !bHas : bHas;
}
