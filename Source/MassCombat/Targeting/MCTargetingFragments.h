#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCTargetingFragments.generated.h"

USTRUCT()
struct FMCTargetingFragment : public FMassFragment
{
	GENERATED_BODY()

	FVector TargetLocation = FVector::ZeroVector;

	FVector SlotLocation = FVector::ZeroVector;

	float DistanceToTarget = TNumericLimits<float>::Max();

	float DistanceToSlot = TNumericLimits<float>::Max();

	FVector NearestEnemyLocation = FVector::ZeroVector;

	float DistanceToNearestEnemy = TNumericLimits<float>::Max();

	int32 SlotIndex = INDEX_NONE;

	float NextRetargetTime = 0.f;

	FMassEntityHandle CurrentTarget;

	uint8 bHasTarget : 1 = 0;

	uint8 bHasNearestEnemy : 1 = 0;
};

USTRUCT()
struct FMCTargetingParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bPreferPlayerTarget = false;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float PlayerTargetRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float TargetSearchRadius = 3000.f;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float NearTargetSearchRadius = 800.f;
};
