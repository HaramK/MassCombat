#include "Mass/StateTree/MCNpcMoveToTargetTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "NavCorridor.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawMoveGoal(
	TEXT("mc.DrawMoveGoal"),
	false,
	TEXT("Draw MoveToTarget / MoveToTargetSlot goal per agent (blue = target, cyan = slot)."));

bool FMCNpcMoveToTargetTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(AgentRadiusHandle);
	Linker.LinkExternalData(DesiredMovementHandle);
	Linker.LinkExternalData(MovementParamsHandle);
	Linker.LinkExternalData(CachedPathHandle);
	Linker.LinkExternalData(ShortPathHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

bool FMCNpcMoveToTargetTask::HasGoal(const FMCNpcCombatFragment& Combat) const
{
	return Combat.bHasTarget;
}

FVector FMCNpcMoveToTargetTask::GetGoalLocation(const FMCNpcCombatFragment& Combat) const
{
	return Combat.TargetLocation;
}

float FMCNpcMoveToTargetTask::GetGoalDistance(const FMCNpcCombatFragment& Combat) const
{
	return Combat.DistanceToTarget;
}

bool FMCNpcMoveToTargetTask::IsGoalReached(const FMCNpcCombatFragment& Combat, float AcceptanceRadius) const
{
	return GetGoalDistance(Combat) <= AcceptanceRadius;
}

FColor FMCNpcMoveToTargetTask::GetDebugColor() const
{
	return FColor::Blue;
}

void FMCNpcMoveToTargetTask::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite<FMCNpcCombatFragment>();
	Builder.AddReadOnly<FTransformFragment>();
	Builder.AddReadWrite<FMassMoveTargetFragment>();
	Builder.AddReadOnly<FAgentRadiusFragment>();
	Builder.AddReadOnly<FMassDesiredMovementFragment>();
	Builder.AddReadWrite<FMassNavMeshCachedPathFragment>();
	Builder.AddReadWrite<FMassNavMeshShortPathFragment>();
}

bool FMCNpcMoveToTargetTask::RequestPath(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const FAgentRadiusFragment& AgentRadius = Context.GetExternalData(AgentRadiusHandle);
	const FVector AgentLocation = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();

	UWorld* World = Context.GetWorld();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		return false;
	}

	const FNavAgentProperties NavAgentProps(AgentRadius.Radius);
	const ANavigationData* NavData = NavSys->GetNavDataForProps(NavAgentProps, AgentLocation);
	if (!NavData)
	{
		return false;
	}

	FPathFindingQuery Query(nullptr, *NavData, AgentLocation, GetGoalLocation(Combat));
	if (!Query.NavData.IsValid())
	{
		Query.NavData = NavSys->GetNavDataForProps(NavAgentProps, Query.StartLocation);
	}

	FPathFindingResult Result(ENavigationQueryResult::Error);
	if (Query.NavData.IsValid())
	{
		Result = Query.NavData->FindPath(NavAgentProps, Query);
	}

	if (!Result.IsSuccessful() || !Result.Path.IsValid())
	{
		return false;
	}

	Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
	if (Result.Path->GetPathPoints().Num() <= 1)
	{
		return false;
	}

	FMassNavMeshCachedPathFragment& CachedPath = Context.GetExternalData(CachedPathHandle);
	CachedPath.NavPath = Result.Path;
	CachedPath.PathSource = EMassNavigationPathSource::NavMesh;
	CachedPath.Corridor = MakeShared<FNavCorridor>();

	const FSharedConstNavQueryFilter Filter = Query.QueryFilter ? Query.QueryFilter : NavData->GetDefaultQueryFilter();
	FNavCorridorParams CorridorParams;
	CorridorParams.SetFromWidth(Data.CorridorWidth);
	CorridorParams.PathOffsetFromBoundaries = Data.OffsetFromBoundaries;
	CachedPath.Corridor->BuildFromPath(*CachedPath.NavPath, Filter, CorridorParams);

	FMassNavMeshShortPathFragment& ShortPath = Context.GetExternalData(ShortPathHandle);
	ShortPath.RequestShortPath(CachedPath.Corridor, 0, 0, Data.EndDistanceThreshold);
	CachedPath.NavPathNextStartIndex = (uint16)FMath::Max(ShortPath.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate - FMassNavMeshCachedPathFragment::NumLeadingPoints, 0);

	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassMovementParameters& MovementParams = Context.GetExternalData(MovementParamsHandle);
	const FMassDesiredMovementFragment& DesiredMovement = Context.GetExternalData(DesiredMovementHandle);

	float DesiredSpeed = FMath::Min(MovementParams.DefaultDesiredSpeed * Data.SpeedScale, MovementParams.MaxSpeed);
	DesiredSpeed = FMath::Min(DesiredSpeed, DesiredMovement.DesiredMaxSpeedOverride);
	MoveTarget.DesiredSpeed.Set(DesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);

	return true;
}

bool FMCNpcMoveToTargetTask::UpdateShortPath(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	FMassNavMeshCachedPathFragment& CachedPath = Context.GetExternalData(CachedPathHandle);
	if (!CachedPath.Corridor.IsValid())
	{
		return false;
	}

	FMassNavMeshShortPathFragment& ShortPath = Context.GetExternalData(ShortPathHandle);
	ShortPath.RequestShortPath(CachedPath.Corridor, CachedPath.NavPathNextStartIndex, FMassNavMeshCachedPathFragment::NumLeadingPoints, Data.EndDistanceThreshold);
	CachedPath.NavPathNextStartIndex += (uint16)FMath::Max(ShortPath.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate - FMassNavMeshCachedPathFragment::NumLeadingPoints, 0);

	return true;
}

void FMCNpcMoveToTargetTask::ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const
{
	FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntityDeferred(MassContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::NewStateTreeTaskRequired, MassContext.GetEntity(), Delay);
}

EStateTreeRunStatus FMCNpcMoveToTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);

	Data.LastRepathTargetLocation = FVector(TNumericLimits<float>::Max());
	if (HasGoal(Combat) && RequestPath(Context))
	{
		Data.LastRepathTargetLocation = GetGoalLocation(Combat);
	}

	ScheduleNextTick(Context, Data.RepathInterval);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCNpcMoveToTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const FMassNavMeshShortPathFragment& ShortPath = Context.GetExternalData(ShortPathHandle);

	if (HasGoal(Combat))
	{
		const FVector GoalLocation = GetGoalLocation(Combat);

		if (CVarDrawMoveGoal.GetValueOnGameThread())
		{
			if (UWorld* World = Context.GetWorld())
			{
				const FVector AgentLocation = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
				const FVector GoalDraw(GoalLocation.X, GoalLocation.Y, AgentLocation.Z);
				const FColor Color = GetDebugColor();
				DrawDebugLine(World, AgentLocation, GoalDraw, Color, false, Data.RepathInterval * 1.5f, 0, 2.f);
				DrawDebugSphere(World, GoalDraw, 15.f, 8, Color, false, Data.RepathInterval * 1.5f, 0, 1.f);
			}
		}

		const bool bTargetMoved = FVector::DistSquared(GoalLocation, Data.LastRepathTargetLocation) > FMath::Square(Data.RepathDistanceThreshold);
		if (bTargetMoved)
		{
			if (RequestPath(Context))
			{
				Data.LastRepathTargetLocation = GoalLocation;
			}
		}
		else if (ShortPath.IsDone() && ShortPath.bPartialResult)
		{
			UpdateShortPath(Context);
		}

		if (IsGoalReached(Combat, Data.AcceptanceRadius))
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	ScheduleNextTick(Context, Data.RepathInterval);

	return EStateTreeRunStatus::Running;
}
