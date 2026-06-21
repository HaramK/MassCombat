#include "Mass/StateTree/MCNpcMoveToNearestEnemyTask.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FMCNpcMoveToNearestEnemyTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.LoiterMinRadius = MinRadius;
	Combat.LoiterMaxRadius = MaxRadius;
	Combat.bHasLoiterOffset = false;

	return FMCNpcMoveToTargetTask::EnterState(Context, Transition);
}
