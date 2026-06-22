#include "Behavior/Tasks/MCLookAtNearestEnemyTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"

bool FMCLookAtNearestEnemyTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCLookAtNearestEnemyTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCCombatFragment>();
}

void FMCLookAtNearestEnemyTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCLookAtNearestEnemyTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.bLookAtNearestEnemy = true;
	Combat.LookAtTurnRate = TurnRate;

	ScheduleNextTick(Context, TickInterval);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCLookAtNearestEnemyTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	ScheduleNextTick(Context, TickInterval);
	return EStateTreeRunStatus::Running;
}

void FMCLookAtNearestEnemyTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.bLookAtNearestEnemy = false;
}
