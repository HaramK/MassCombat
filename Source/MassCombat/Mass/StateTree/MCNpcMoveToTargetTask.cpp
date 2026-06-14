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

	FPathFindingQuery Query(nullptr, *NavData, AgentLocation, Combat.TargetLocation);
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
	if (Combat.bHasTarget && RequestPath(Context))
	{
		Data.LastRepathTargetLocation = Combat.TargetLocation;
	}

	ScheduleNextTick(Context, Data.RepathInterval);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMCNpcMoveToTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);
	const FMassNavMeshShortPathFragment& ShortPath = Context.GetExternalData(ShortPathHandle);

	if (Combat.bHasTarget)
	{
		const bool bTargetMoved = FVector::DistSquared(Combat.TargetLocation, Data.LastRepathTargetLocation) > FMath::Square(Data.RepathDistanceThreshold);
		if (bTargetMoved)
		{
			if (RequestPath(Context))
			{
				Data.LastRepathTargetLocation = Combat.TargetLocation;
			}
		}
		else if (ShortPath.IsDone() && ShortPath.bPartialResult)
		{
			UpdateShortPath(Context);
		}

		if (Combat.DistanceToTarget <= Data.AcceptanceRadius)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	ScheduleNextTick(Context, Data.RepathInterval);

	return EStateTreeRunStatus::Running;
}
