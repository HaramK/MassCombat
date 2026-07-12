#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "AnimToTextureDataAsset.h"
#include "MCCombatFragments.generated.h"

class UAnimMontage;

USTRUCT()
struct FMCCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector TargetLocation = FVector::ZeroVector;

	FVector SlotLocation = FVector::ZeroVector;

	float DistanceToTarget = TNumericLimits<float>::Max();

	float DistanceToSlot = TNumericLimits<float>::Max();

	FVector NearestEnemyLocation = FVector::ZeroVector;

	FVector LoiterLocation = FVector::ZeroVector;

	FVector LoiterOffset = FVector::ZeroVector;

	float DistanceToNearestEnemy = TNumericLimits<float>::Max();

	float DistanceToLoiter = TNumericLimits<float>::Max();

	float LoiterMinRadius = 0.f;

	float LoiterMaxRadius = 0.f;

	float LookAtTurnRate = 0.f;

	int32 SlotIndex = INDEX_NONE;

	FMassEntityHandle LastAttackerUnit;

	float LastDamagedTime = -1.e6f;

	float LastHitReactTime = -1.e6f;

	float LastAttackTime = -1.e6f;

	float NextRetargetTime = 0.f;

	float FaceTargetEndTime = 0.f;

	FMassEntityHandle CurrentTarget;

	uint8 bHasTarget : 1 = 0;

	uint8 bHasNearestEnemy : 1 = 0;

	uint8 bHasLoiterOffset : 1 = 0;

	uint8 bLookAtNearestEnemy : 1 = 0;

	uint8 bMovementBlocked : 1 = 0;

	uint8 bReverseSlot : 1 = 0;
};

USTRUCT()
struct FMCAnimStateFragment : public FMassFragment
{
	GENERATED_BODY()

	TWeakObjectPtr<UAnimToTextureDataAsset> AnimData;

	TWeakObjectPtr<UAnimMontage> ActiveMontage;

	float GlobalStartTime = 0.f;

	float PlayRate = 1.f;

	float CurrentFrame = 0.f;

	int32 StateIndex = 0;

	int32 ActiveMontageStateIndex = INDEX_NONE;

	uint8 bSwappedThisFrame : 1 = 0;
};

USTRUCT()
struct FMCCombatParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bPreferPlayerTarget = false;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float PlayerTargetRadius = 1000.f;

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
	TObjectPtr<UAnimToTextureDataAsset> DefaultAnimData;
};
