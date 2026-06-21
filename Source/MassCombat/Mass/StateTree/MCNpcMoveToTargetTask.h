#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcMoveToTargetTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

struct FTransformFragment;
struct FMassMoveTargetFragment;
struct FAgentRadiusFragment;
struct FMassDesiredMovementFragment;
struct FMassMovementParameters;
struct FMassNavMeshCachedPathFragment;
struct FMassNavMeshShortPathFragment;
class UMassSignalSubsystem;

USTRUCT()
struct FMCNpcMoveToTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Move")
	float AcceptanceRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float RepathDistanceThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float RepathInterval = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Path")
	float CorridorWidth = 600.f;

	UPROPERTY(EditAnywhere, Category = "Path")
	float OffsetFromBoundaries = 10.f;

	UPROPERTY(EditAnywhere, Category = "Path")
	float EndDistanceThreshold = 20.f;

	UPROPERTY(EditAnywhere, Category = "Path")
	float SpeedScale = 1.f;

	FVector LastRepathTargetLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "MC Npc Move To Target"))
struct FMCNpcMoveToTargetTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcMoveToTargetInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	bool RequestPath(FStateTreeExecutionContext& Context) const;
	bool UpdateShortPath(FStateTreeExecutionContext& Context) const;
	void ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const;

	virtual bool HasGoal(const FMCNpcCombatFragment& Combat) const;
	virtual FVector GetGoalLocation(const FMCNpcCombatFragment& Combat) const;
	virtual float GetGoalDistance(const FMCNpcCombatFragment& Combat) const;
	virtual bool IsGoalReached(const FMCNpcCombatFragment& Combat, float AcceptanceRadius) const;
	virtual FColor GetDebugColor() const;

	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FAgentRadiusFragment> AgentRadiusHandle;
	TStateTreeExternalDataHandle<FMassDesiredMovementFragment> DesiredMovementHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamsHandle;
	TStateTreeExternalDataHandle<FMassNavMeshCachedPathFragment> CachedPathHandle;
	TStateTreeExternalDataHandle<FMassNavMeshShortPathFragment> ShortPathHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
