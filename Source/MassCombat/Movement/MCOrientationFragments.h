#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCOrientationFragments.generated.h"

USTRUCT()
struct FMCOrientationFragment : public FMassFragment
{
	GENERATED_BODY()

	float FaceTargetEndTime = 0.f;

	float LookAtTurnRate = 0.f;

	uint8 bLookAtNearestEnemy : 1 = 0;

	uint8 bMovementBlocked : 1 = 0;
};
