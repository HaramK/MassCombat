#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Combat/MCCombatFragments.h"
#include "MCAttackTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

class UMassSignalSubsystem;

USTRUCT()
struct FMCAttackInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	float ElapsedTime = 0.f;

	UPROPERTY()
	float Duration = 1.f;
};

USTRUCT(meta = (DisplayName = "MC Attack"))
struct FMCAttackTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCAttackInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	void ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const;

	TStateTreeExternalDataHandle<FMCAnimStateFragment> AnimHandle;
	TStateTreeExternalDataHandle<FMCCombatFragment> CombatHandle;
	TStateTreeExternalDataHandle<FMCCombatParams> ParamsHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float AttackCooldown = 1.5f;
};
