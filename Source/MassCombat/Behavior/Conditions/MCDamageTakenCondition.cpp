#include "Behavior/Conditions/MCDamageTakenCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#if WITH_EDITOR
FText FMCDamageTakenCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceDataView.GetPtr<FInstanceDataType>();
	check(Data);
	return Data->bInvert
		? NSLOCTEXT("MassCombat", "MCDamageTakenConditionDescInv", "NO unreacted damage")
		: NSLOCTEXT("MassCombat", "MCDamageTakenConditionDesc", "Damage Taken (unreacted)");
}
#endif

bool FMCDamageTakenCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EngagementHandle);
	return true;
}

bool FMCDamageTakenCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCEngagementFragment& Engagement = Context.GetExternalData(EngagementHandle);
	// New, not-yet-reacted hit: damage stamped after the last hit-react was started.
	const bool bDamaged = Engagement.LastDamagedTime > Engagement.LastHitReactTime;
	return Data.bInvert ? !bDamaged : bDamaged;
}
