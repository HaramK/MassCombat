#include "Behavior/Tasks/MCStandTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"

namespace
{
	// Reevaluate cadence multiplier by LOD, jittered to spread wakeup waves (same policy as MCMoveToTargetTask).
	float GetStandDelayScale(EMassLOD::Type LOD)
	{
		float Scale;
		switch (LOD)
		{
		case EMassLOD::High:   Scale = 1.f; break;
		case EMassLOD::Medium: Scale = 3.f; break;
		case EMassLOD::Low:    Scale = 8.f; break;
		default:               Scale = 16.f; break;
		}
		return Scale * FMath::FRandRange(1.f, 1.25f);
	}
}

bool FMCStandTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(VelocityHandle);
	Linker.LinkExternalData(RepresentationLODHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCStandTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly<FTransformFragment>();
	Builder.AddReadWrite<FMassMoveTargetFragment>();
	Builder.AddReadWrite<FMassVelocityFragment>();
	Builder.AddReadOnly<FMassRepresentationLODFragment>();
}

EStateTreeRunStatus FMCStandTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.Time = 0.f;

	UWorld* World = Context.GetWorld();
	const FVector Location = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	FMassVelocityFragment& Velocity = Context.GetExternalData(VelocityHandle);

	// Hard stop: hand transform control to animation so steering/avoidance won't move it, and clear velocity.
	Velocity.Value = FVector::ZeroVector;
	MoveTarget.Center = Location;
	MoveTarget.CreateNewAction(EMassMovementAction::Animate, *World);
	MoveTarget.DesiredSpeed.Set(0.f);

	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	const float Delay = Data.Duration > 0.f
		? Data.Duration
		: Data.ReevaluateInterval * GetStandDelayScale(Context.GetExternalData(RepresentationLODHandle).LOD);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCStandTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.Time += DeltaTime;

	if (Data.Duration <= 0.f)
	{
		FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
		UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
		SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
			UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(),
			Data.ReevaluateInterval * GetStandDelayScale(Context.GetExternalData(RepresentationLODHandle).LOD));
		return EStateTreeRunStatus::Running;
	}
	return Data.Time < Data.Duration ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}
