#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "GameplayTagContainer.h"
#include "Action/MCActionFragments.h"
#include "MCPerformActionTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

class UMassSignalSubsystem;
struct FMCAnimStateFragment;
struct FMCCombatFragment;

USTRUCT()
struct FMCActionTrackCursor
{
	GENERATED_BODY()

	UPROPERTY()
	int32 StepIndex = 0;

	UPROPERTY()
	float StepEndTime = 0.f;

	UPROPERTY()
	bool bDone = false;
};

USTRUCT()
struct FMCPerformActionInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ActionIndex = INDEX_NONE;

	UPROPERTY()
	FMassEntityHandle LockedTarget;

	UPROPERTY()
	TArray<FMCActionTrackCursor> Cursors;
};

USTRUCT(meta = (DisplayName = "MC Perform Action"))
struct FMCPerformActionTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCPerformActionInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	void ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const;

	TStateTreeExternalDataHandle<FMCActionFragment> ActionHandle;
	TStateTreeExternalDataHandle<FMCActionSetParams> ActionSetHandle;
	TStateTreeExternalDataHandle<FMCAnimStateFragment> AnimHandle;
	TStateTreeExternalDataHandle<FMCCombatFragment> CombatHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTag ActionTag;
};
