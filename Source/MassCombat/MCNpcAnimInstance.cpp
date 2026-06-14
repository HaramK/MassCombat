#include "MCNpcAnimInstance.h"
#include "Animation/AnimMontage.h"

void UMCNpcAnimInstance::PlayAttackMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	if (!Montage_IsPlaying(Montage))
	{
		Montage_Play(Montage);
	}
}
