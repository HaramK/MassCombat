#include "Action/MCActionStep.h"
#include "Combat/MCCombatFragments.h"
#include "Movement/MCOrientationFragments.h"
#include "Representation/MCRepresentationFragments.h"
#include "Unit/MCUnitFragments.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
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
	if (!Ctx.EntityManager || !Ctx.Engagement)
	{
		return;
	}

	Ctx.Engagement->LastAttackTime = Ctx.Now;

	const FMassEntityHandle Target = Ctx.LockedTarget;
	if (!Ctx.EntityManager->IsEntityValid(Target))
	{
		return;
	}

	FMCUnitFragment* TargetUnit = Ctx.EntityManager->GetFragmentDataPtr<FMCUnitFragment>(Target);
	if (!TargetUnit || TargetUnit->Health <= 0.f)
	{
		return;
	}
	TargetUnit->Health -= Damage;

	if (FMCEngagementFragment* TargetEngagement = Ctx.EntityManager->GetFragmentDataPtr<FMCEngagementFragment>(Target))
	{
		TargetEngagement->LastAttackerUnit = Ctx.SelfEntity;
		TargetEngagement->LastDamagedTime = Ctx.Now;
	}
}

void FMCActionStep_MarkHitReacted::OnStart(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Engagement)
	{
		Ctx.Engagement->LastHitReactTime = Ctx.Now;
	}
}
