#include "Behavior/Tasks/MCMoveToNearestEnemyTask.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMCMoveToNearestEnemyTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.LoiterMinRadius = MinRadius;
	Combat.LoiterMaxRadius = MaxRadius;
	Combat.bHasLoiterOffset = false;

	return FMCMoveToTargetTask::EnterState(Context, Transition);
}
