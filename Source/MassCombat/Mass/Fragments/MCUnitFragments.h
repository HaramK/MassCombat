#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MCUnitFragments.generated.h"

USTRUCT()
struct FMCUnitFragment : public FMassFragment
{
	GENERATED_BODY()

	float Health = 0.f;
};

USTRUCT()
struct FMCUnitInfoFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Unit")
	uint8 Faction = 0;

	UPROPERTY(EditAnywhere, Category = "Unit")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 MaxAttackerCounts = 3;

	UPROPERTY(EditAnywhere, Category = "Unit")
	float AttackerSlotRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Unit")
	bool bRecalcSlotsOnAttackerLoss = false;
};
