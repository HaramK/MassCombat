#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCTargetingFragments.generated.h"

USTRUCT()
struct FMCTargetingParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bPreferPlayerTarget = false;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float PlayerTargetRadius = 1000.f;
};
