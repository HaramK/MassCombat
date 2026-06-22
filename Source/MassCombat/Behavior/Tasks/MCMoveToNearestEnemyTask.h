#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/MCMoveToTargetTask.h"
#include "MCMoveToNearestEnemyTask.generated.h"

USTRUCT(meta = (DisplayName = "MC Move To Nearest Enemy"))
struct FMCMoveToNearestEnemyTask : public FMCMoveToTargetTask
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual bool HasGoal(const FMCCombatFragment& Combat) const override { return Combat.bHasNearestEnemy; }
	virtual FVector GetGoalLocation(const FMCCombatFragment& Combat) const override { return Combat.LoiterLocation; }
	virtual float GetGoalDistance(const FMCCombatFragment& Combat) const override { return Combat.DistanceToLoiter; }
	virtual FColor GetDebugColor() const override { return FColor::Magenta; }

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MinRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MaxRadius = 400.f;
};
