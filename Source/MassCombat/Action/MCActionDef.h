#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Action/MCActionStep.h"
#include "MCActionDef.generated.h"

USTRUCT()
struct FMCActionTrack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	TArray<TInstancedStruct<FMCActionStep>> Steps;
};

UCLASS(BlueprintType)
class MASSCOMBAT_API UMCActionDef : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FGameplayTag ActivationTag;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Tags")
	FGameplayTagContainer GrantsTags;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float CooldownTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bBlockMovementWhileActive = true;

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	TArray<FMCActionTrack> Tracks;
};
