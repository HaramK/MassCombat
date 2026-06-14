#include "MCNpcCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

AMCNpcCharacter::AMCNpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMCNpcCharacter::PlayAttackMontage()
{
	if (!AttackMontage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (!AnimInstance->Montage_IsPlaying(AttackMontage))
			{
				AnimInstance->Montage_Play(AttackMontage);
			}
		}
	}
}
