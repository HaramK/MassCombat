#include "Behavior/Conditions/MCHasTargetCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#if WITH_EDITOR
FText FMCHasTargetCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceDataView.GetPtr<FInstanceDataType>();
	check(Data);
	return Data->bInvert
		? NSLOCTEXT("MassCombat", "MCHasTargetConditionDescInv", "Has NO Target")
		: NSLOCTEXT("MassCombat", "MCHasTargetConditionDesc", "Has Target");
}
#endif

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
