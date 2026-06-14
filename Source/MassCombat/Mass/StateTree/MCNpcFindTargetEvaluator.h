#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcFindTargetEvaluator.generated.h"

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
}

USTRUCT()
struct FMCNpcFindTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	float DistanceToTarget = 0.f;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bInAttackRange = false;
};

USTRUCT(meta = (DisplayName = "MC Npc Find Target"))
struct FMCNpcFindTargetEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcFindTargetInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float AttackRange = 200.f;
};
