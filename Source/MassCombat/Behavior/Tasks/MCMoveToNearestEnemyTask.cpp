#include "Behavior/Tasks/MCMoveToNearestEnemyTask.h"
#include "Targeting/MCTargetingFragments.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMCMoveToNearestEnemyTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);

	const float Angle = FMath::FRandRange(0.f, 2.f * UE_PI);
	const float Radius = FMath::FRandRange(MinRadius, MaxRadius);
	Data.LoiterOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

	return FMCMoveToTargetTask::EnterState(Context, Transition);
}

FVector FMCMoveToNearestEnemyTask::GetGoalLocation(FStateTreeExecutionContext& Context, const FMCTargetingFragment& Targeting) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	return Targeting.NearestEnemyLocation + Data.LoiterOffset;
}
