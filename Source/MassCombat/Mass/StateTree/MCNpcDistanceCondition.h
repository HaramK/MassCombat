#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcDistanceCondition.generated.h"

UENUM()
enum class EMCDistanceSource : uint8
{
	NearestEnemy,
	TargetSlot,
	Target,
};

USTRUCT()
struct FMCNpcDistanceConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EMCDistanceSource Source = EMCDistanceSource::Target;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Distance = 200.f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Npc Within Distance"))
struct FMCNpcDistanceCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCNpcDistanceConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCNpcCombatFragment> CombatHandle;
};
