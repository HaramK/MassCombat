#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MCStandTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

struct FTransformFragment;
struct FMassMoveTargetFragment;
struct FMassVelocityFragment;
struct FMassRepresentationLODFragment;
class UMassSignalSubsystem;

USTRUCT()
struct FMCStandInstanceData
{
	GENERATED_BODY()

	// Duration <= 0 runs until a StateTree transition stops the task.
	UPROPERTY(EditAnywhere, Category = "Stand")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, Category = "Stand")
	float ReevaluateInterval = 0.5f;

	float Time = 0.f;
};

USTRUCT(meta = (DisplayName = "MC Stand"))
struct FMCStandTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCStandInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FMassVelocityFragment> VelocityHandle;
	TStateTreeExternalDataHandle<FMassRepresentationLODFragment> RepresentationLODHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
