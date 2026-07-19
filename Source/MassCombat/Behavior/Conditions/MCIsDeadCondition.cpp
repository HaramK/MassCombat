#include "Behavior/Conditions/MCIsDeadCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#if WITH_EDITOR
FText FMCIsDeadCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceDataView.GetPtr<FInstanceDataType>();
	check(Data);
	return Data->bInvert
		? NSLOCTEXT("MassCombat", "MCIsDeadConditionDescInv", "Is Alive")
		: NSLOCTEXT("MassCombat", "MCIsDeadConditionDesc", "Is Dead");
}
#endif

bool FMCIsDeadCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(UnitHandle);
	return true;
}

bool FMCIsDeadCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCUnitFragment& Unit = Context.GetExternalData(UnitHandle);
	const bool bDead = Unit.Health <= 0.f;
	return Data.bInvert ? !bDead : bDead;
}
