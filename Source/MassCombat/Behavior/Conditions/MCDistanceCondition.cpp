#include "Behavior/Conditions/MCDistanceCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#if WITH_EDITOR
FText FMCDistanceCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceDataView.GetPtr<FInstanceDataType>();
	check(Data);

	const TCHAR* Source = TEXT("Target");
	switch (Data->Source)
	{
	case EMCDistanceSource::NearestEnemy:
		Source = TEXT("Nearest Enemy");
		break;
	case EMCDistanceSource::TargetSlot:
		Source = TEXT("Target Slot");
		break;
	default:
		break;
	}

	return FText::Format(NSLOCTEXT("MassCombat", "MCDistanceCondDesc", "{0} distance {1} {2}"),
		FText::FromString(Source),
		FText::FromString(Data->bInvert ? TEXT(">") : TEXT("≤")),
		FText::AsNumber(Data->Distance));
}
#endif

bool FMCDistanceCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TargetingHandle);
	return true;
}

bool FMCDistanceCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCTargetingFragment& Targeting = Context.GetExternalData(TargetingHandle);

	float Dist;
	switch (Data.Source)
	{
	case EMCDistanceSource::NearestEnemy:
		Dist = Targeting.DistanceToNearestEnemy;
		break;
	case EMCDistanceSource::TargetSlot:
		Dist = Targeting.DistanceToSlot;
		break;
	default:
		Dist = Targeting.DistanceToTarget;
		break;
	}

	const bool bClose = Dist <= Data.Distance;
	return Data.bInvert ? !bClose : bClose;
}
