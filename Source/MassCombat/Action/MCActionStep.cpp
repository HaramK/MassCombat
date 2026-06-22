#include "Action/MCActionStep.h"
#include "Combat/MCCombatFragments.h"
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
	if (Ctx.Anim)
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
	if (Ctx.Combat)
	{
		Ctx.Combat->FaceTargetEndTime = Ctx.Now + Duration;
	}
}

void FMCActionStep_RotateToTarget::OnEnd(const FMCActionStepContext& Ctx) const
{
	if (Ctx.Combat)
	{
		Ctx.Combat->FaceTargetEndTime = 0.f;
	}
}

void FMCActionStep_ApplyDamage::OnStart(const FMCActionStepContext& Ctx) const
{
	if (!Ctx.EntityManager || !Ctx.Combat)
	{
		return;
	}

	Ctx.Combat->LastAttackTime = Ctx.Now;

	const FMassEntityHandle Target = Ctx.Combat->CurrentTarget;
	if (!Ctx.EntityManager->IsEntityValid(Target))
	{
		return;
	}

	if (FMCUnitFragment* TargetUnit = Ctx.EntityManager->GetFragmentDataPtr<FMCUnitFragment>(Target))
	{
		TargetUnit->Health -= Damage;
	}

	if (FMCCombatFragment* TargetCombat = Ctx.EntityManager->GetFragmentDataPtr<FMCCombatFragment>(Target))
	{
		TargetCombat->LastAttackerUnit = Ctx.SelfEntity;
		TargetCombat->LastDamagedTime = Ctx.Now;
	}
}
