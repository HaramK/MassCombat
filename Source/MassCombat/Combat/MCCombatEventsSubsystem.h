#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCCombatEventsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMCOnPlayerDied);

UCLASS()
class MASSCOMBAT_API UMCCombatEventsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "MassCombat")
	FMCOnPlayerDied OnPlayerDied;
};
