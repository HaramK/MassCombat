#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Targeting/MCTargetingFragments.h"
#include "MCIsReverseSlotCondition.generated.h"

USTRUCT()
struct FMCIsReverseSlotConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Is Reverse Slot"))
struct FMCIsReverseSlotCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCIsReverseSlotConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCTargetingFragment> TargetingHandle;
};
