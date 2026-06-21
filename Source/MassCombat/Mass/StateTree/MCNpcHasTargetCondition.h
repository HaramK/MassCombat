#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcHasTargetCondition.generated.h"

USTRUCT()
struct FMCNpcHasTargetConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Npc Has Target"))
struct FMCNpcHasTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcHasTargetConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;
};
