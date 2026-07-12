#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Movement/MCOrientationFragments.h"
#include "MCLookAtNearestEnemyTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

class UMassSignalSubsystem;

USTRUCT()
struct FMCLookAtNearestEnemyInstanceData
{
	GENERATED_BODY()
};

USTRUCT(meta = (DisplayName = "MC Look At Nearest Enemy"))
struct FMCLookAtNearestEnemyTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCLookAtNearestEnemyInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	void ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const;

	TStateTreeExternalDataHandle<FMCOrientationFragment> OrientationHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float TurnRate = 360.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float TickInterval = 0.2f;
};
