#include "Action/MCActionStep.h"
#include "Combat/MCCombatFragments.h"
#include "Movement/MCOrientationFragments.h"
#include "Representation/MCRepresentationFragments.h"
#include "Animation/AnimMontage.h"
#include "Combat/MCDamageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntityManager.h"

float FMCActionStep_PlayMontage::GetDuration(const FMCActionStepContext& Ctx) const
{
	return Montage ? Montage->GetPlayLength() : 0.f;
}

void FMCActionStep_PlayMontage::OnStart(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Anim)
	{
		Ctx.Anim->ActiveMontage = Montage.Get();
		Ctx.Anim->ActiveMontageStateIndex = StateIndex;
	}
}

void FMCActionStep_PlayMontage::OnEnd(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Anim && Ctx.Anim->ActiveMontage.Get() == Montage.Get())
	{
		Ctx.Anim->ActiveMontage = nullptr;
		Ctx.Anim->ActiveMontageStateIndex = INDEX_NONE;
	}
}

void FMCActionStep_Print::OnStart(const FMCActionStepContext& Ctx) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, ScreenTime, FColor::Cyan, FString::Printf(TEXT("[Action] %s"), *Message));
	}
}

void FMCActionStep_RotateToTarget::OnStart(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Orientation)
	{
		Ctx.Orientation->FaceTargetEndTime = Ctx.Now + Duration;
	}
}

void FMCActionStep_RotateToTarget::OnEnd(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Orientation)
	{
		Ctx.Orientation->FaceTargetEndTime = 0.f;
	}
}

void FMCActionStep_ApplyDamage::OnStart(const FMCActionStepContext& Ctx) const
{
	if (!Ctx.Engagement)
	{
		return;
	}

	Ctx.Engagement->LastAttackTime = Ctx.Now;

	if (UMCDamageSubsystem* DamageSubsystem = Ctx.World ? Ctx.World->GetSubsystem<UMCDamageSubsystem>() : nullptr)
	{
		DamageSubsystem->QueueDamage(Ctx.LockedTarget, Ctx.SelfEntity, Damage);
	}
}

void FMCActionStep_MarkHitReacted::OnStart(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Engagement)
	{
		Ctx.Engagement->LastHitReactTime = Ctx.Now;
	}
}
