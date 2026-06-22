#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Combat/MCCombatFragments.h"
#include "MCHasTargetCondition.generated.h"

USTRUCT()
struct FMCHasTargetConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Has Target"))
struct FMCHasTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCHasTargetConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCCombatFragment> CombatHandle;
};
