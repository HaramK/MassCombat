#include "Mass/StateTree/MCNpcLookAtNearestEnemyTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"

bool FMCNpcLookAtNearestEnemyTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCNpcLookAtNearestEnemyTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCNpcCombatFragment>();
}

void FMCNpcLookAtNearestEnemyTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCNpcLookAtNearestEnemyTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.bLookAtNearestEnemy = true;
	Combat.LookAtTurnRate = TurnRate;

	ScheduleNextTick(Context, TickInterval);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCNpcLookAtNearestEnemyTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	ScheduleNextTick(Context, TickInterval);
	return EStateTreeRunStatus::Running;
}

void FMCNpcLookAtNearestEnemyTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.bLookAtNearestEnemy = false;
}
