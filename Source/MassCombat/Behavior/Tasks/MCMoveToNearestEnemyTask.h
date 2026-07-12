#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/MCMoveToTargetTask.h"
#include "MCMoveToNearestEnemyTask.generated.h"

USTRUCT()
struct FMCMoveToNearestEnemyInstanceData : public FMCMoveToTargetInstanceData
{
	GENERATED_BODY()

	FVector LoiterOffset = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "MC Move To Nearest Enemy"))
struct FMCMoveToNearestEnemyTask : public FMCMoveToTargetTask
{
	GENERATED_BODY()

	using FInstanceDataType = FMCMoveToNearestEnemyInstanceData;

protected:
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual bool HasGoal(const FMCTargetingFragment& Targeting) const override { return Targeting.bHasNearestEnemy; }
	virtual FVector GetGoalLocation(FStateTreeExecutionContext& Context, const FMCTargetingFragment& Targeting) const override;
	virtual FColor GetDebugColor() const override { return FColor::Magenta; }

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MinRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Loiter")
	float MaxRadius = 400.f;
};
