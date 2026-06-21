#include "Mass/StateTree/MCPerformActionTask.h"
#include "Mass/Actions/MCActionLib.h"
#include "Mass/Actions/MCActionDef.h"
#include "Mass/Actions/MCActionStep.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassEntityUtils.h"
#include "Engine/World.h"

namespace
{
	FMCActionStepContext MakeStepContext(UWorld* World, FMCNpcAnimStateFragment* Anim, FMCNpcCombatFragment* Combat, float Now)
	{
		FMCActionStepContext Ctx;
		Ctx.World = World;
		Ctx.EntityManager = World ? &UE::Mass::Utils::GetEntityManagerChecked(*World) : nullptr;
		Ctx.Anim = Anim;
		Ctx.Combat = Combat;
		Ctx.Now = Now;
		return Ctx;
	}

	void StartStep(const FMCActionTrack& Track, FMCActionTrackCursor& Cur, const FMCActionStepContext& Ctx, float Now)
	{
		if (const FMCActionStep* Step = Track.Steps[Cur.StepIndex].GetPtr())
		{
			Step->OnStart(Ctx);
			Cur.StepEndTime = Now + FMath::Max(0.f, Step->GetDuration(Ctx));
		}
		else
		{
			Cur.StepEndTime = Now;
		}
	}

	void TickTrack(const FMCActionTrack& Track, FMCActionTrackCursor& Cur, const FMCActionStepContext& Ctx, float Now)
	{
		while (!Cur.bDone && Cur.StepEndTime <= Now)
		{
			if (const FMCActionStep* Step = Track.Steps[Cur.StepIndex].GetPtr())
			{
				Step->OnEnd(Ctx);
			}

			++Cur.StepIndex;
			if (Cur.StepIndex >= Track.Steps.Num())
			{
				Cur.bDone = true;
			}
			else
			{
				StartStep(Track, Cur, Ctx, Now);
			}
		}
	}

	void BeginTrack(const FMCActionTrack& Track, FMCActionTrackCursor& Cur, const FMCActionStepContext& Ctx, float Now)
	{
		Cur.StepIndex = 0;
		Cur.bDone = false;

		if (Track.Steps.Num() == 0)
		{
			Cur.bDone = true;
			return;
		}

		StartStep(Track, Cur, Ctx, Now);
		TickTrack(Track, Cur, Ctx, Now);
	}

	bool AllDone(const TArray<FMCActionTrackCursor>& Cursors)
	{
		for (const FMCActionTrackCursor& Cur : Cursors)
		{
			if (!Cur.bDone)
			{
				return false;
			}
		}
		return true;
	}

	float SoonestDelay(const TArray<FMCActionTrackCursor>& Cursors, float Now)
	{
		float Soonest = TNumericLimits<float>::Max();
		for (const FMCActionTrackCursor& Cur : Cursors)
		{
			if (!Cur.bDone)
			{
				Soonest = FMath::Min(Soonest, Cur.StepEndTime);
			}
		}
		return Soonest == TNumericLimits<float>::Max() ? 0.f : FMath::Max(0.f, Soonest - Now);
	}
}

bool FMCPerformActionTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ActionHandle);
	Linker.LinkExternalData(ActionSetHandle);
	Linker.LinkExternalData(AnimHandle);
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FMCPerformActionTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCActionFragment>();
	Builder.AddReadWrite<FMCNpcAnimStateFragment>();
	Builder.AddReadWrite<FMCNpcCombatFragment>();
}

void FMCPerformActionTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCPerformActionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	FMCActionFragment& ActionFrag = Context.GetExternalData(ActionHandle);
	const FMCActionSetParams& Set = Context.GetExternalData(ActionSetHandle);
	FMCNpcAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	UWorld* World = SignalSubsystem.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	if (!FMCActionLib::TryStart(ActionFrag, Set, ActionTag, Now))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.ActionIndex = FMCActionLib::FindActionIndex(Set, ActionTag);
	const UMCActionDef* Def = Set.Actions.IsValidIndex(Data.ActionIndex) ? Set.Actions[Data.ActionIndex] : nullptr;
	if (!Def)
	{
		FMCActionLib::Stop(ActionFrag, Set, Data.ActionIndex, Now);
		return EStateTreeRunStatus::Failed;
	}

	if (Def->bBlockMovementWhileActive)
	{
		Combat.bMovementBlocked = true;
	}

	const FMCActionStepContext Ctx = MakeStepContext(World, &Anim, &Combat, Now);

	Data.Cursors.SetNum(Def->Tracks.Num());
	for (int32 t = 0; t < Def->Tracks.Num(); ++t)
	{
		BeginTrack(Def->Tracks[t], Data.Cursors[t], Ctx, Now);
	}

	if (AllDone(Data.Cursors))
	{
		FMCActionLib::Stop(ActionFrag, Set, Data.ActionIndex, Now);
		return EStateTreeRunStatus::Succeeded;
	}

	ScheduleNextTick(Context, SoonestDelay(Data.Cursors, Now));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCPerformActionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	FMCActionFragment& ActionFrag = Context.GetExternalData(ActionHandle);
	const FMCActionSetParams& Set = Context.GetExternalData(ActionSetHandle);
	FMCNpcAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	UWorld* World = SignalSubsystem.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	const UMCActionDef* Def = Set.Actions.IsValidIndex(Data.ActionIndex) ? Set.Actions[Data.ActionIndex] : nullptr;
	if (!Def)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FMCActionStepContext Ctx = MakeStepContext(World, &Anim, &Combat, Now);

	const int32 Count = FMath::Min(Data.Cursors.Num(), Def->Tracks.Num());
	for (int32 t = 0; t < Count; ++t)
	{
		TickTrack(Def->Tracks[t], Data.Cursors[t], Ctx, Now);
	}

	if (AllDone(Data.Cursors))
	{
		FMCActionLib::Stop(ActionFrag, Set, Data.ActionIndex, Now);
		return EStateTreeRunStatus::Succeeded;
	}

	ScheduleNextTick(Context, SoonestDelay(Data.Cursors, Now));
	return EStateTreeRunStatus::Running;
}

void FMCPerformActionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	FMCActionFragment& ActionFrag = Context.GetExternalData(ActionHandle);
	const FMCActionSetParams& Set = Context.GetExternalData(ActionSetHandle);
	FMCNpcAnimStateFragment& Anim = Context.GetExternalData(AnimHandle);
	FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	UWorld* World = SignalSubsystem.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	const UMCActionDef* Def = Set.Actions.IsValidIndex(Data.ActionIndex) ? Set.Actions[Data.ActionIndex] : nullptr;

	if (Def && Def->bBlockMovementWhileActive)
	{
		Combat.bMovementBlocked = false;
	}

	if (!FMCActionLib::IsRunning(ActionFrag, Data.ActionIndex, Now))
	{
		return;
	}

	if (Def)
	{
		const FMCActionStepContext Ctx = MakeStepContext(World, &Anim, &Combat, Now);
		const int32 Count = FMath::Min(Data.Cursors.Num(), Def->Tracks.Num());
		for (int32 t = 0; t < Count; ++t)
		{
			const FMCActionTrackCursor& Cur = Data.Cursors[t];
			if (!Cur.bDone && Def->Tracks[t].Steps.IsValidIndex(Cur.StepIndex))
			{
				if (const FMCActionStep* Step = Def->Tracks[t].Steps[Cur.StepIndex].GetPtr())
				{
					Step->OnEnd(Ctx);
				}
			}
		}
	}

	FMCActionLib::Stop(ActionFrag, Set, Data.ActionIndex, Now);
}
