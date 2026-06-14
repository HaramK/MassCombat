#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MCNpcCharacter.generated.h"

class UAnimMontage;

UCLASS()
class MASSCOMBAT_API AMCNpcCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMCNpcCharacter();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackMontage();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
};
