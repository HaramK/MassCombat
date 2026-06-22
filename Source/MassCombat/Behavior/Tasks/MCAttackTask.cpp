#include "Behavior/Tasks/MCAttackTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "Animation/AnimMontage.h"

bool FMCAttackTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AnimHandle);
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(ParamsHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCAttackTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCAnimStateFragment>();
	Builder.AddReadWrite<FMCCombatFragment>();
}

void FMCAttackTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCCombatParams& Params = Context.GetExternalData(ParamsHandle);

	Data.ElapsedTime = 0.f;
	Data.Duration = Params.AttackMontage ? Params.AttackMontage->GetPlayLength() : Params.AttackDurationFallback;

	FMCAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	Anim.ActiveMontage = Params.AttackMontage;
	Anim.ActiveMontageStateIndex = Params.AttackStateIndex;

	ScheduleNextTick(Context, Data.Duration);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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

void FMCAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMCAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	Anim.ActiveMontage = nullptr;
	Anim.ActiveMontageStateIndex = INDEX_NONE;

	FMCCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	Combat.AttackCooldownRemaining = AttackCooldown;
}
