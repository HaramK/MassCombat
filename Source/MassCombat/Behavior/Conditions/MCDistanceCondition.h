#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Targeting/MCTargetingFragments.h"
#include "MCDistanceCondition.generated.h"

UENUM()
enum class EMCDistanceSource : uint8
{
	NearestEnemy,
	TargetSlot,
	Target,
};

USTRUCT()
struct FMCDistanceConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EMCDistanceSource Source = EMCDistanceSource::Target;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Distance = 200.f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "MC Within Distance"))
struct FMCDistanceCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMCDistanceConditionInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	TStateTreeExternalDataHandle<FMCTargetingFragment> TargetingHandle;
};
