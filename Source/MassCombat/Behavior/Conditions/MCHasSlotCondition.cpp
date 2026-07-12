#include "Behavior/Conditions/MCHasSlotCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#if WITH_EDITOR
FText FMCHasSlotCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceDataView.GetPtr<FInstanceDataType>();
	check(Data);
	return Data->bInvert
		? NSLOCTEXT("MassCombat", "MCHasSlotConditionDescInv", "Has NO Slot")
		: NSLOCTEXT("MassCombat", "MCHasSlotConditionDesc", "Has Slot");
}
#endif

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
