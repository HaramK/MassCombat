#pragma once

#include "CoreMinimal.h"
#include "MCActionStep.generated.h"

class UWorld;
class UAnimMontage;
struct FMCNpcAnimStateFragment;
struct FMCNpcCombatFragment;

struct FMCActionStepContext
{
	UWorld* World = nullptr;
	FMCNpcAnimStateFragment* Anim = nullptr;
	FMCNpcCombatFragment* Combat = nullptr;
	float Now = 0.f;
};

USTRUCT(meta = (Hidden))
struct FMCActionStep
{
	GENERATED_BODY()

	virtual ~FMCActionStep() = default;

	virtual float GetDuration(const FMCActionStepContext& Ctx) const { return 0.f; }

	virtual void OnStart(const FMCActionStepContext& Ctx) const {}

	virtual void OnEnd(const FMCActionStepContext& Ctx) const {}
};

USTRUCT(meta = (DisplayName = "Wait"))
struct FMCActionStep_Wait : public FMCActionStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	float WaitTime = 0.f;

	virtual float GetDuration(const FMCActionStepContext& Ctx) const override { return WaitTime; }
};

USTRUCT(meta = (DisplayName = "Play Montage"))
struct FMCActionStep_PlayMontage : public FMCActionStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	int32 StateIndex = INDEX_NONE;

	virtual float GetDuration(const FMCActionStepContext& Ctx) const override;
	virtual void OnStart(const FMCActionStepContext& Ctx) const override;
	virtual void OnEnd(const FMCActionStepContext& Ctx) const override;
};

USTRUCT(meta = (DisplayName = "Print (Debug)"))
struct FMCActionStep_Print : public FMCActionStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	FString Message;

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	float ScreenTime = 2.0f;

	virtual void OnStart(const FMCActionStepContext& Ctx) const override;
};

USTRUCT(meta = (DisplayName = "Rotate To Target"))
struct FMCActionStep_RotateToTarget : public FMCActionStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Step")
	float Duration = 0.f;

	virtual float GetDuration(const FMCActionStepContext& Ctx) const override { return Duration; }
	virtual void OnStart(const FMCActionStepContext& Ctx) const override;
	virtual void OnEnd(const FMCActionStepContext& Ctx) const override;
};
