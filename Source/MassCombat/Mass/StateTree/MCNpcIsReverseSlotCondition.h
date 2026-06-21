#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcIsReverseSlotCondition.generated.h"

USTRUCT()
struct FMCNpcIsReverseSlotConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Npc Is Reverse Slot"))
struct FMCNpcIsReverseSlotCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcIsReverseSlotConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;
};
