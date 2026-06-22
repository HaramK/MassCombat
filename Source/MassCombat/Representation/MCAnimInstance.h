#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MCAnimInstance.generated.h"

class UAnimMontage;

UCLASS()
class MASSCOMBAT_API UMCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackMontage(UAnimMontage* Montage);

	void PlayMontageSynced(UAnimMontage* Montage, float StartPosition, bool bInstantBlend);

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	FRotator ActorRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bOrientRotationToMovement = true;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bSwappedThisFrame = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float LocomotionStartPosition = 0.f;

private:
	bool IsMontageAlreadyPlaying(const UAnimMontage* Montage) const;
};
