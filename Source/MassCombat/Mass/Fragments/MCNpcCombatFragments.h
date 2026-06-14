#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "AnimToTextureDataAsset.h"
#include "MCNpcCombatFragments.generated.h"

class UAnimMontage;

USTRUCT()
struct FMCNpcCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector TargetLocation = FVector::ZeroVector;

	float DistanceToTarget = TNumericLimits<float>::Max();

	float AttackCooldownRemaining = 0.f;

	uint8 bHasTarget : 1 = 0;
};

USTRUCT()
struct FMCNpcAnimStateFragment : public FMassFragment
{
	GENERATED_BODY()

	TWeakObjectPtr<UAnimToTextureDataAsset> AnimData;

	float GlobalStartTime = 0.f;

	float PlayRate = 1.f;

	int32 StateIndex = 0;

	uint8 bAttacking : 1 = 0;

	uint8 bSwappedThisFrame : 1 = 0;
};

USTRUCT()
struct FMCNpcCombatParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float WalkSpeedThresholdSq = 100.f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float WalkAnimReferenceSpeed = 140.f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float MinWalkPlayRate = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float MaxWalkPlayRate = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	int32 IdleStateIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Anim")
	int32 WalkStateIndex = 1;

	UPROPERTY(EditAnywhere, Category = "Anim")
	int32 AttackStateIndex = 2;

	UPROPERTY(EditAnywhere, Category = "Anim")
	TSoftObjectPtr<UAnimToTextureDataAsset> DefaultAnimData;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDurationFallback = 1.0f;
};
