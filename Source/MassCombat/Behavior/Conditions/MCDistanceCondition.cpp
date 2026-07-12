#include "Behavior/Conditions/MCDistanceCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

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
