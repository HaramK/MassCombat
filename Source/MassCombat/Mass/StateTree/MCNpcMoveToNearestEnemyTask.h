#pragma once

#include "CoreMinimal.h"
#include "Mass/StateTree/MCNpcMoveToTargetTask.h"
#include "MCNpcMoveToNearestEnemyTask.generated.h"

USTRUCT(meta = (DisplayName = "MC Npc Move To Nearest Enemy"))
struct FMCNpcMoveToNearestEnemyTask : public FMCNpcMoveToTargetTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MinRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MaxRadius = 400.f;

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual bool HasGoal(const FMCNpcCombatFragment& Combat) const override { return Combat.bHasNearestEnemy; }
	virtual FVector GetGoalLocation(const FMCNpcCombatFragment& Combat) const override { return Combat.LoiterLocation; }
	virtual float GetGoalDistance(const FMCNpcCombatFragment& Combat) const override { return Combat.DistanceToLoiter; }
	virtual FColor GetDebugColor() const override { return FColor::Magenta; }
};
