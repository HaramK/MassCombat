#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcAttackTask.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

class UMassSignalSubsystem;

USTRUCT()
struct FMCNpcAttackInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	float ElapsedTime = 0.f;

	UPROPERTY()
	float Duration = 1.f;
};

USTRUCT(meta = (DisplayName = "MC Npc Attack"))
struct FMCNpcAttackTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcAttackInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	void ScheduleNextTick(FStateTreeExecutionContext& Context, float Delay) const;

	TStateTreeExternalDataHandle<FMCNpcAnimStateFragment> AnimHandle;
	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;
	TStateTreeExternalDataHandle<FMCNpcCombatParams> ParamsHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float AttackCooldown = 1.5f;
};
