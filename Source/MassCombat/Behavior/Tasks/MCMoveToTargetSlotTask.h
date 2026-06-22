#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/MCMoveToTargetTask.h"
#include "MCMoveToTargetSlotTask.generated.h"

USTRUCT(meta = (DisplayName = "MC Move To Target Slot"))
struct FMCMoveToTargetSlotTask : public FMCMoveToTargetTask
{
	GENERATED_BODY()

protected:
	virtual FVector GetGoalLocation(const FMCCombatFragment& Combat) const override { return Combat.SlotLocation; }
	virtual float GetGoalDistance(const FMCCombatFragment& Combat) const override { return Combat.DistanceToSlot; }
	virtual FColor GetDebugColor() const override { return FColor::Cyan; }
};
