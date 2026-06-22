#include "Behavior/Conditions/MCDistanceCondition.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMCDistanceCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

bool FMCDistanceCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);

	float Dist;
	switch (Data.Source)
	{
	case EMCDistanceSource::NearestEnemy:
		Dist = Combat.DistanceToNearestEnemy;
		break;
	case EMCDistanceSource::TargetSlot:
		Dist = Combat.DistanceToSlot;
		break;
	default:
		Dist = Combat.DistanceToTarget;
		break;
	}

	const bool bClose = Dist <= Data.Distance;
	return Data.bInvert ? !bClose : bClose;
}
