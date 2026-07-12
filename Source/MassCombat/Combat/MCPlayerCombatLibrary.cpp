#include "Combat/MCPlayerCombatLibrary.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassEntitySubsystem.h"
#include "MassActorSubsystem.h"
#include "MassAgentComponent.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawPlayerMelee(
	TEXT("mc.DrawPlayerMelee"),
	false,
	TEXT("Draw the player melee cone (cyan) and hits (green) for MassCombat."));

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
	const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);
	const float CosHalfAngle = FMath::Cos(HalfAngleRad);

	const bool bDraw = CVarDrawPlayerMelee.GetValueOnGameThread();
	if (bDraw)
	{
		DrawDebugCone(World, Origin, Forward, Range, HalfAngleRad, HalfAngleRad, 24, FColor::Cyan, false, 1.0f, 0, 1.5f);
		DrawDebugCircle(World, Origin, Range, 48, FColor::Cyan, false, 1.0f, 0, 1.5f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	struct FHitCandidate
	{
		FMCUnitFragment* Unit;
		FMCEngagementFragment* Engagement;
		FVector Location;
		uint8 Faction;
	};

	if (bDraw && GEngine)
	{
		const bool bValid = EntityManager.IsEntityValid(PlayerEntity);
		const FMCUnitInfoFragment* Info = bValid ? EntityManager.GetConstSharedFragmentDataPtr<FMCUnitInfoFragment>(PlayerEntity) : nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("[Melee] entityValid=%d index=%d unitInfo=%d"), bValid ? 1 : 0, PlayerEntity.Index, Info ? 1 : 0));

		FMassEntityHandle AgentHandle;
		if (const UMassAgentComponent* Agent = PlayerActor->FindComponentByClass<UMassAgentComponent>())
		{
			AgentHandle = Agent->GetEntityHandle();
		}
		const bool bAgentValid = EntityManager.IsEntityValid(AgentHandle);
		const bool bHasActorFrag = bAgentValid && EntityManager.GetFragmentDataPtr<FMassActorFragment>(AgentHandle) != nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
			FString::Printf(TEXT("[Melee] agentIndex=%d agentValid=%d actorFrag=%d"), AgentHandle.Index, bAgentValid ? 1 : 0, bHasActorFrag ? 1 : 0));
	}

	if (!EntityManager.IsEntityValid(PlayerEntity))
	{
		return 0;
	}
	const FMCUnitInfoFragment* PlayerInfo = EntityManager.GetConstSharedFragmentDataPtr<FMCUnitInfoFragment>(PlayerEntity);
	if (!PlayerInfo)
	{
		return 0;
	}
	const uint8 PlayerFaction = PlayerInfo->Faction;

	TArray<FHitCandidate> Candidates;

	FMassEntityQuery Query(EntityManager.AsShared());
	Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadWrite);
	Query.AddRequirement<FMCEngagementFragment>(EMassFragmentAccess::ReadWrite);
	Query.AddConstSharedRequirement<FMCUnitInfoFragment>();

	FMassExecutionContext Context(EntityManager);
	Query.ForEachEntityChunk(Context, [&Candidates, PlayerEntity](FMassExecutionContext& Ctx)
	{
		const uint8 Faction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCUnitFragment> Units = Ctx.GetMutableFragmentView<FMCUnitFragment>();
		const TArrayView<FMCEngagementFragment> Engagements = Ctx.GetMutableFragmentView<FMCEngagementFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Ctx.GetEntity(i) == PlayerEntity)
			{
				continue;
			}
			if (Units[i].Health <= 0.f)
			{
				continue;
			}
			Candidates.Add({ &Units[i], &Engagements[i], Transforms[i].GetTransform().GetLocation(), Faction });
		}
	});

	int32 HitCount = 0;
	for (const FHitCandidate& Cand : Candidates)
	{
		FVector ToTarget = Cand.Location - Origin;
		ToTarget.Z = 0.f;
		const float DistSq = ToTarget.SizeSquared();

		if (Cand.Faction == PlayerFaction)
		{
			if (bDraw)
			{
				DrawDebugSphere(World, Cand.Location, 40.f, 8, FColor::Blue, false, 1.0f, 0, 1.5f);
			}
			continue;
		}

		if (DistSq > RangeSq || DistSq < KINDA_SMALL_NUMBER)
		{
			if (bDraw)
			{
				DrawDebugSphere(World, Cand.Location, 40.f, 8, FColor::Orange, false, 1.0f, 0, 1.5f);
			}
			continue;
		}

		const FVector Dir = ToTarget * FMath::InvSqrt(DistSq);
		if (FVector::DotProduct(Forward, Dir) < CosHalfAngle)
		{
			if (bDraw)
			{
				DrawDebugSphere(World, Cand.Location, 40.f, 8, FColor::Magenta, false, 1.0f, 0, 1.5f);
			}
			continue;
		}

		Cand.Unit->Health -= Damage;
		Cand.Engagement->LastAttackerUnit = PlayerEntity;
		Cand.Engagement->LastDamagedTime = Now;
		++HitCount;

		if (bDraw)
		{
			DrawDebugSphere(World, Cand.Location, 50.f, 12, FColor::Green, false, 1.0f, 0, 2.5f);
			DrawDebugLine(World, Cand.Location, Cand.Location + FVector(0.f, 0.f, 150.f), FColor::Green, false, 1.0f, 0, 3.0f);
		}
	}

	if (bDraw && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White,
			FString::Printf(TEXT("[Melee] candidates=%d hits=%d playerFaction=%d"), Candidates.Num(), HitCount, PlayerFaction));
	}

	return HitCount;
}
