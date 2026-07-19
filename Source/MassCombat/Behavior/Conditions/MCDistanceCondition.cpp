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

	bool bValid;
	float Dist;
	switch (Data.Source)
	{
	case EMCDistanceSource::NearestEnemy:
		bValid = Targeting.bHasNearestEnemy != 0;
		Dist = Targeting.DistanceToNearestEnemy;
		break;
	case EMCDistanceSource::TargetSlot:
		bValid = Targeting.bHasTarget != 0;
		Dist = Targeting.DistanceToSlot;
		break;
	default:
		bValid = Targeting.bHasTarget != 0;
		Dist = Targeting.DistanceToTarget;
		break;
	}

	// No subject: the condition is false regardless of invert ("near"/"far" both presuppose one).
	if (!bValid)
	{
		return false;
	}

	const bool bClose = Dist <= Data.Distance;
	return Data.bInvert ? !bClose : bClose;
}
