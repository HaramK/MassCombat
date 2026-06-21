#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCCombatSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "MC Combat"))
class MASSCOMBAT_API UMCCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Targeting")
	float CombatWindow = 10.f;

	UPROPERTY(config, EditAnywhere, Category = "Targeting")
	float RetargetInterval = 1.f;
};
