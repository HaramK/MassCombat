#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "AnimToTextureDataAsset.h"
#include "MCRepresentationFragments.generated.h"

class UAnimMontage;

USTRUCT()
struct FMCAnimStateFragment : public FMassFragment
{
	GENERATED_BODY()

	TWeakObjectPtr<UAnimToTextureDataAsset> AnimData;

	TWeakObjectPtr<UAnimMontage> ActiveMontage;

	float GlobalStartTime = 0.f;

	float PlayRate = 1.f;

	float CurrentFrame = 0.f;

	int32 StateIndex = 0;

	int32 ActiveMontageStateIndex = INDEX_NONE;

	uint8 bSwappedThisFrame : 1 = 0;
};

USTRUCT()
struct FMCAnimParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Anim")
	float WalkSpeedThresholdSq = 100.f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float WalkAnimReferenceSpeed = 140.f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float MinWalkPlayRate = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	float MaxWalkPlayRate = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Anim")
	int32 IdleStateIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Anim")
	int32 WalkStateIndex = 1;

	UPROPERTY(EditAnywhere, Category = "Anim")
	TObjectPtr<UAnimToTextureDataAsset> DefaultAnimData;
};
