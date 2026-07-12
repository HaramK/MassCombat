#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/MCMoveToTargetTask.h"
#include "MCMoveToTargetSlotTask.generated.h"

USTRUCT(meta = (DisplayName = "MC Move To Target Slot"))
struct FMCMoveToTargetSlotTask : public FMCMoveToTargetTask
{
	GENERATED_BODY()

protected:
	virtual FVector GetGoalLocation(FStateTreeExecutionContext& Context, const FMCTargetingFragment& Targeting) const override { return Targeting.SlotLocation; }
	virtual FColor GetDebugColor() const override { return FColor::Cyan; }
};
