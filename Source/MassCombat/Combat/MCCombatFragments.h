#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCCombatFragments.generated.h"

USTRUCT()
struct FMCEngagementFragment : public FMassFragment
{
	GENERATED_BODY()

	FMassEntityHandle LastAttackerUnit;

	float LastDamagedTime = -1.e6f;

	float LastHitReactTime = -1.e6f;

	float LastAttackTime = -1.e6f;
};

USTRUCT()
struct FMCDeadTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FMCDeathFragment : public FMassFragment
{
	GENERATED_BODY()

	float DestroyTime = 0.f;
};
