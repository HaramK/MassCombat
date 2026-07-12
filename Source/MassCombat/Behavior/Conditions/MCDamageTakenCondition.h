#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Combat/MCCombatFragments.h"
#include "MCDamageTakenCondition.generated.h"

USTRUCT()
struct FMCDamageTakenConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Damage Taken"))
struct FMCDamageTakenCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCDamageTakenConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const override;
#endif

	TStateTreeExternalDataHandle<FMCEngagementFragment> EngagementHandle;
};
