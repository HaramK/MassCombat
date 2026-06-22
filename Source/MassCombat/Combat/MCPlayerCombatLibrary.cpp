#include "Combat/MCPlayerCombatLibrary.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassEntitySubsystem.h"
#include "MassActorSubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

int32 UMCPlayerCombatLibrary::PlayerMeleeAttack(AActor* PlayerActor, float Range, float HalfAngleDeg, float Damage)
{
	if (!PlayerActor)
	{
		return 0;
	}

	UWorld* World = PlayerActor->GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UMassActorSubsystem* ActorSubsystem = World ? World->GetSubsystem<UMassActorSubsystem>() : nullptr;
	if (!EntitySubsystem || !ActorSubsystem)
	{
		return 0;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle PlayerEntity = ActorSubsystem->GetEntityHandleFromActor(PlayerActor);

	const FVector Origin = PlayerActor->GetActorLocation();
	FVector Forward = PlayerActor->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward = Forward.GetSafeNormal();

	const float Now = World->GetTimeSeconds();
	const float RangeSq = Range * Range;
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfAngleDeg));

	struct FHitCandidate
	{
		FMCUnitFragment* Unit;
		FMCCombatFragment* Combat;
		FVector Location;
		uint8 Faction;
	};

	TArray<FHitCandidate> Candidates;
	uint8 PlayerFaction = TNumericLimits<uint8>::Max();

	FMassEntityQuery Query(EntityManager.AsShared());
	Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadWrite);
	Query.AddRequirement<FMCCombatFragment>(EMassFragmentAccess::ReadWrite);
	Query.AddConstSharedRequirement<FMCUnitInfoFragment>();

	FMassExecutionContext Context(EntityManager);
	Query.ForEachEntityChunk(Context, [&Candidates, &PlayerFaction, PlayerEntity](FMassExecutionContext& Ctx)
	{
		const uint8 Faction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCUnitFragment> Units = Ctx.GetMutableFragmentView<FMCUnitFragment>();
		const TArrayView<FMCCombatFragment> Combats = Ctx.GetMutableFragmentView<FMCCombatFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Ctx.GetEntity(i) == PlayerEntity)
			{
				PlayerFaction = Faction;
				continue;
			}
			if (Units[i].Health <= 0.f)
			{
				continue;
			}
			Candidates.Add({ &Units[i], &Combats[i], Transforms[i].GetTransform().GetLocation(), Faction });
		}
	});

	int32 HitCount = 0;
	for (const FHitCandidate& Cand : Candidates)
	{
		if (Cand.Faction == PlayerFaction)
		{
			continue;
		}

		FVector ToTarget = Cand.Location - Origin;
		ToTarget.Z = 0.f;
		const float DistSq = ToTarget.SizeSquared();
		if (DistSq > RangeSq || DistSq < KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector Dir = ToTarget * FMath::InvSqrt(DistSq);
		if (FVector::DotProduct(Forward, Dir) < CosHalfAngle)
		{
			continue;
		}

		Cand.Unit->Health -= Damage;
		Cand.Combat->LastAttackerUnit = PlayerEntity;
		Cand.Combat->LastDamagedTime = Now;
		++HitCount;
	}

	return HitCount;
}
