#pragma once

#include "CoreMinimal.h"
#include "Mass/StateTree/MCNpcMoveToTargetTask.h"
#include "MCNpcMoveToTargetSlotTask.generated.h"

USTRUCT(meta = (DisplayName = "MC Npc Move To Target Slot"))
struct FMCNpcMoveToTargetSlotTask : public FMCNpcMoveToTargetTask
{
	GENERATED_BODY()

protected:
	virtual FVector GetGoalLocation(const FMCNpcCombatFragment& Combat) const override { return Combat.SlotLocation; }
	virtual float GetGoalDistance(const FMCNpcCombatFragment& Combat) const override { return Combat.DistanceToSlot; }
	virtual FColor GetDebugColor() const override { return FColor::Cyan; }
};
