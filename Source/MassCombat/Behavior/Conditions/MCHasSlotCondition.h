#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Targeting/MCTargetingFragments.h"
#include "MCHasSlotCondition.generated.h"

USTRUCT()
struct FMCHasSlotConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Has Slot"))
struct FMCHasSlotCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCHasSlotConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const override;
#endif

	TStateTreeExternalDataHandle<FMCTargetingFragment> TargetingHandle;
};
