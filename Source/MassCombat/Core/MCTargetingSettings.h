#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCTargetingSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "MC Targeting"))
class MASSCOMBAT_API UMCTargetingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Targeting")
	float CombatWindow = 10.f;

	UPROPERTY(config, EditAnywhere, Category = "Targeting")
	float RetargetInterval = 1.f;
};
