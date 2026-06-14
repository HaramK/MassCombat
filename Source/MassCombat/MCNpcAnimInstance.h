#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MCNpcAnimInstance.generated.h"

class UAnimMontage;

UCLASS()
class MASSCOMBAT_API UMCNpcAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackMontage(UAnimMontage* Montage);

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
};
