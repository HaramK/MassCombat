#include "Representation/MCAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

void UMCAnimInstance::PlayAttackMontage(UAnimMontage* Montage)
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

bool UMCAnimInstance::IsMontageAlreadyPlaying(const UAnimMontage* Montage) const
{
	for (const FAnimMontageInstance* MontageInstance : MontageInstances)
	{
		if (MontageInstance && MontageInstance->Montage == Montage && MontageInstance->IsPlaying())
		{
			return true;
		}
	}
	return false;
}

void UMCAnimInstance::PlayMontageSynced(UAnimMontage* Montage, float StartPosition, bool bInstantBlend)
{
	if (!Montage)
	{
		return;
	}

	if (!IsMontageAlreadyPlaying(Montage))
	{
		FAlphaBlendArgs BlendIn = Montage->GetBlendInArgs();
		if (bInstantBlend)
		{
			BlendIn.BlendTime = 0.0f;
		}

		const float ClampedStart = FMath::Clamp(StartPosition, 0.0f, Montage->GetPlayLength());
		Montage_PlayWithBlendIn(Montage, BlendIn, 1.0f, EMontagePlayReturnType::MontageLength, ClampedStart);
	}

	if (bInstantBlend)
	{
		if (USkeletalMeshComponent* OwningComp = GetOwningComponent())
		{
			OwningComp->TickAnimation(0.0f, false);
			OwningComp->RefreshBoneTransforms();
		}
	}
}
