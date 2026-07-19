#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Unit/MCUnitFragments.h"
#include "MCIsDeadCondition.generated.h"

USTRUCT()
struct FMCIsDeadConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Is Dead"))
struct FMCIsDeadCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCIsDeadConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const override;
#endif

	TStateTreeExternalDataHandle<FMCUnitFragment> UnitHandle;
};
