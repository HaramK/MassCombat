#include "Mass/StateTree/MCNpcAttackTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "Animation/AnimMontage.h"

bool FMCNpcAttackTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AnimHandle);
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(ParamsHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCNpcAttackTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCNpcAnimStateFragment>();
	Builder.AddReadWrite<FMCNpcCombatFragment>();
}

void FMCNpcAttackTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCNpcAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatParams& Params = Context.GetExternalData(ParamsHandle);

	Data.ElapsedTime = 0.f;
	Data.Duration = Params.AttackMontage ? Params.AttackMontage->GetPlayLength() : Params.AttackDurationFallback;

	FMCNpcAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	Anim.bAttacking = true;

	ScheduleNextTick(Context, Data.Duration);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCNpcAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime += DeltaTime;

	if (Data.ElapsedTime >= Data.Duration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	ScheduleNextTick(Context, Data.Duration - Data.ElapsedTime);

	return EStateTreeRunStatus::Running;
}

void FMCNpcAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCNpcAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	Anim.bAttacking = false;

	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.AttackCooldownRemaining = AttackCooldown;
}
