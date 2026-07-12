#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCCombatFragments.generated.h"

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
struct FMCEngagementFragment : public FMassFragment
{
	GENERATED_BODY()

	FMassEntityHandle LastAttackerUnit;

	float LastDamagedTime = -1.e6f;

	float LastHitReactTime = -1.e6f;

	float LastAttackTime = -1.e6f;
};

