#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MCPlayerCombatLibrary.generated.h"

UCLASS()
class MASSCOMBAT_API UMCPlayerCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Melee hit from the player actor: damages enemy Mass entities within Range and HalfAngleDeg of the actor's
	// forward. Returns the number of entities hit. Damage feeds the normal combat flow (retaliation + hit reaction).
	UFUNCTION(BlueprintCallable, Category = "MassCombat")
	static int32 PlayerMeleeAttack(AActor* PlayerActor, float Range = 200.f, float HalfAngleDeg = 60.f, float Damage = 25.f);
};
