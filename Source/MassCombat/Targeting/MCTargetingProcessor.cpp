#include "Targeting/MCTargetingProcessor.h"
#include "Targeting/MCTargetingFragments.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Core/MCTargetingSettings.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawTargetSlots(
	TEXT("mc.DrawTargetSlots"),
	false,
	TEXT("Draw attacker slot positions / target links for MassCombat targeting."));

UMCTargetingProcessor::UMCTargetingProcessor()
	: GatherQuery(*this)
	, EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCTargetingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	GatherQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCTargetingFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCEngagementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();
	EntityQuery.AddConstSharedRequirement<FMCTargetingParams>();
}

namespace
{
	FVector ComputeSlotLocation(const FMCTargetCandidate& Cand, int32 SlotIndex)
	{
		const int32 SlotCount = FMath::Max(1, Cand.MaxAttackers);
		// Golden angle (~137.5 deg) seeded by the target's entity index for deterministic, well-spread slot angles.
		const float BaseAngle = Cand.Handle.Index * 2.3999632f;
		const float Angle = BaseAngle + (2.f * UE_PI / SlotCount) * SlotIndex;
		return Cand.Location + FVector(FMath::Cos(Angle) * Cand.SlotRadius, FMath::Sin(Angle) * Cand.SlotRadius, 0.f);
	}
}

void UMCTargetingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const bool bDrawSlots = CVarDrawTargetSlots.GetValueOnGameThread();

	const UMCTargetingSettings* Settings = GetDefault<UMCTargetingSettings>();
	const float CombatWindow = Settings->CombatWindow;
	const float RetargetInterval = Settings->RetargetInterval;

	GatherCandidates(Context);
	GatherAttackers(Context);

	AssignedCount.Reset();
	AssignedCount.AddZeroed(Candidates.Num());

	const int32 PlayerCandIdx = FindPlayerCandidate();
	MarkPlayerTargetedCandidates(PlayerCandIdx);

	AssignPlayerTargets(PlayerCandIdx);
	AssignReturningTargets(Now, CombatWindow, RetargetInterval);
	AssignOpenTargets();
	AssignSlots();
	WriteResults(World, bDrawSlots);
}

void UMCTargetingProcessor::GatherCandidates(FMassExecutionContext& Context)
{
	Candidates.Reset();

	GatherQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Ctx)
	{
		const FMCUnitInfoFragment& Info = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMCUnitFragment> Units = Ctx.GetFragmentView<FMCUnitFragment>();
		const bool bIsPlayer = Ctx.DoesArchetypeHaveTag<FMCPlayerTag>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Units[i].Health > 0.f)
			{
				Candidates.Add({ Ctx.GetEntity(i), Transforms[i].GetTransform().GetLocation(), Info.Faction, Info.MaxAttackerCounts, Info.AttackerSlotRadius, Info.bRecalcSlotsOnAttackerLoss, bIsPlayer });
			}
		}
	});

	HandleToCand.Reset();
	HandleToCand.Reserve(Candidates.Num());
	for (int32 c = 0; c < Candidates.Num(); ++c)
	{
		HandleToCand.Add(Candidates[c].Handle, c);
	}
}

void UMCTargetingProcessor::GatherAttackers(FMassExecutionContext& Context)
{
	Attackers.Reset();

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Ctx)
	{
		const uint8 Faction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const FMCTargetingParams& Params = Ctx.GetConstSharedFragment<FMCTargetingParams>();
		const float PlayerRadiusSq = Params.PlayerTargetRadius * Params.PlayerTargetRadius;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCTargetingFragment> Targetings = Ctx.GetMutableFragmentView<FMCTargetingFragment>();
		const TConstArrayView<FMCEngagementFragment> Engagements = Ctx.GetFragmentView<FMCEngagementFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			Attackers.Add({ &Targetings[i], Transforms[i].GetTransform().GetLocation(), Faction, Ctx.GetEntity(i).Index, INDEX_NONE, Ctx.GetEntity(i), Targetings[i].CurrentTarget,
				Engagements[i].LastAttackerUnit, Engagements[i].LastDamagedTime, Engagements[i].LastAttackTime, Params.bPreferPlayerTarget, PlayerRadiusSq });
		}
	});
}

int32 UMCTargetingProcessor::FindPlayerCandidate() const
{
	for (int32 c = 0; c < Candidates.Num(); ++c)
	{
		if (Candidates[c].bIsPlayer)
		{
			return c;
		}
	}
	return INDEX_NONE;
}

void UMCTargetingProcessor::MarkPlayerTargetedCandidates(int32 PlayerCandIdx)
{
	CandTargetsPlayer.Reset();
	CandTargetsPlayer.AddZeroed(Candidates.Num());

	if (PlayerCandIdx == INDEX_NONE)
	{
		return;
	}

	const FMassEntityHandle PlayerHandle = Candidates[PlayerCandIdx].Handle;
	for (const FMCAttacker& A : Attackers)
	{
		if (A.PrevTarget == PlayerHandle)
		{
			if (const int32* Ci = HandleToCand.Find(A.Handle))
			{
				CandTargetsPlayer[*Ci] = true;
			}
		}
	}
}

void UMCTargetingProcessor::AssignPlayerTargets(int32 PlayerCandIdx)
{
	if (PlayerCandIdx == INDEX_NONE)
	{
		return;
	}

	const FMCTargetCandidate& PlayerCand = Candidates[PlayerCandIdx];
	const FMassEntityHandle PlayerHandle = PlayerCand.Handle;

	Contenders.Reset();
	for (int32 a = 0; a < Attackers.Num(); ++a)
	{
		const FMCAttacker& At = Attackers[a];
		if (At.bPreferPlayer
			&& PlayerCand.Faction != At.Faction
			&& FVector::DistSquared(At.Location, PlayerCand.Location) <= At.PlayerRadiusSq)
		{
			Contenders.Add(a);
		}
	}

	Contenders.Sort([this, PlayerHandle, &PlayerCand](int32 X, int32 Y)
	{
		const bool bXInc = Attackers[X].PrevTarget == PlayerHandle;
		const bool bYInc = Attackers[Y].PrevTarget == PlayerHandle;
		if (bXInc != bYInc)
		{
			return bXInc;
		}
		return FVector::DistSquared(Attackers[X].Location, PlayerCand.Location) < FVector::DistSquared(Attackers[Y].Location, PlayerCand.Location);
	});

	const int32 Cap = FMath::Min(Contenders.Num(), FMath::Max(0, PlayerCand.MaxAttackers));
	for (int32 k = 0; k < Cap; ++k)
	{
		FMCAttacker& At = Attackers[Contenders[k]];
		if (At.Targeting->CurrentTarget != PlayerHandle)
		{
			At.Targeting->SlotIndex = INDEX_NONE;
		}
		At.TargetIdx = PlayerCandIdx;
		++AssignedCount[PlayerCandIdx];
	}
}

void UMCTargetingProcessor::AssignReturningTargets(float Now, float CombatWindow, float RetargetInterval)
{
	for (FMCAttacker& At : Attackers)
	{
		if (At.TargetIdx != INDEX_NONE)
		{
			continue;
		}

		FMCTargetingFragment* Targeting = At.Targeting;
		const FMassEntityHandle OldTarget = Targeting->CurrentTarget;
		const int32* OldFound = OldTarget.IsSet() ? HandleToCand.Find(OldTarget) : nullptr;
		const bool bOldValid = OldFound && (Candidates[*OldFound].Faction != At.Faction) && !CandTargetsPlayer[*OldFound];

		const FMassEntityHandle Attacker = At.LastAttackerUnit;
		const int32* AtkFound = (Attacker.IsSet() && Attacker != OldTarget) ? HandleToCand.Find(Attacker) : nullptr;
		const bool bRetaliate = AtkFound && (Candidates[*AtkFound].Faction != At.Faction)
			&& !CandTargetsPlayer[*AtkFound]
			&& ((Now - At.LastDamagedTime) < CombatWindow)
			&& (AssignedCount[*AtkFound] < Candidates[*AtkFound].MaxAttackers);

		if (bRetaliate)
		{
			if (!bOldValid || *AtkFound != *OldFound)
			{
				Targeting->SlotIndex = INDEX_NONE;
			}
			At.TargetIdx = *AtkFound;
			++AssignedCount[*AtkFound];
			continue;
		}

		const bool bRealCombat = bOldValid && (((Now - At.LastAttackTime) < CombatWindow) || ((Now - At.LastDamagedTime) < CombatWindow));
		if (bRealCombat && AssignedCount[*OldFound] < Candidates[*OldFound].MaxAttackers)
		{
			At.TargetIdx = *OldFound;
			++AssignedCount[*OldFound];
			continue;
		}

		if (!bOldValid || Now >= Targeting->NextRetargetTime)
		{
			// Stagger next retarget by a per-entity offset (index mod 257) to spread retarget spikes across frames.
			Targeting->NextRetargetTime = Now + RetargetInterval + (At.EntityIndex % 257) / 257.f * RetargetInterval;
			continue;
		}

		if (AssignedCount[*OldFound] < Candidates[*OldFound].MaxAttackers)
		{
			At.TargetIdx = *OldFound;
			++AssignedCount[*OldFound];
		}
	}
}

void UMCTargetingProcessor::AssignOpenTargets()
{
	for (FMCAttacker& At : Attackers)
	{
		if (At.TargetIdx != INDEX_NONE)
		{
			continue;
		}

		int32 BestOpen = INDEX_NONE;
		float BestOpenDistSq = TNumericLimits<float>::Max();

		for (int32 c = 0; c < Candidates.Num(); ++c)
		{
			if (Candidates[c].Faction == At.Faction)
			{
				continue;
			}

			if (CandTargetsPlayer[c])
			{
				continue;
			}

			if (AssignedCount[c] < Candidates[c].MaxAttackers)
			{
				const float DistSq = FVector::DistSquared(At.Location, Candidates[c].Location);
				if (DistSq < BestOpenDistSq)
				{
					BestOpenDistSq = DistSq;
					BestOpen = c;
				}
			}
		}

		const int32 Chosen = BestOpen;
		if (Chosen != INDEX_NONE)
		{
			const FMassEntityHandle OldTarget = At.Targeting->CurrentTarget;
			const int32* OldFound = OldTarget.IsSet() ? HandleToCand.Find(OldTarget) : nullptr;

			At.TargetIdx = Chosen;
			++AssignedCount[Chosen];
			if (!OldFound || *OldFound != Chosen)
			{
				At.Targeting->SlotIndex = INDEX_NONE;
			}
		}
	}
}

void UMCTargetingProcessor::AssignSlots()
{
	if (Groups.Num() < Candidates.Num())
	{
		Groups.SetNum(Candidates.Num());
	}
	for (int32 c = 0; c < Candidates.Num(); ++c)
	{
		Groups[c].Reset();
	}
	for (int32 a = 0; a < Attackers.Num(); ++a)
	{
		if (Attackers[a].TargetIdx != INDEX_NONE)
		{
			Groups[Attackers[a].TargetIdx].Add(a);
		}
	}

	for (int32 c = 0; c < Candidates.Num(); ++c)
	{
		TArray<int32>& Group = Groups[c];
		if (Group.Num() == 0)
		{
			continue;
		}

		const FMCTargetCandidate& Cand = Candidates[c];
		const int32 SlotCount = FMath::Max(1, Cand.MaxAttackers);

		if (Cand.bRecalc)
		{
			Group.Sort([this](int32 X, int32 Y)
			{
				return Attackers[X].EntityIndex < Attackers[Y].EntityIndex;
			});
			for (int32 r = 0; r < Group.Num(); ++r)
			{
				Attackers[Group[r]].Targeting->SlotIndex = r;
			}
		}
		else
		{
			// Bitmask of occupied slots (supports up to 32 attackers per target): keep valid existing slots, fill the rest.
			uint32 Occupied = 0;
			for (int32 a : Group)
			{
				FMCTargetingFragment* Targeting = Attackers[a].Targeting;
				if (Targeting->SlotIndex >= 0 && Targeting->SlotIndex < SlotCount && Targeting->SlotIndex < 32 && !(Occupied & (1u << Targeting->SlotIndex)))
				{
					Occupied |= (1u << Targeting->SlotIndex);
				}
				else
				{
					Targeting->SlotIndex = INDEX_NONE;
				}
			}

			for (int32 a : Group)
			{
				FMCTargetingFragment* Targeting = Attackers[a].Targeting;
				if (Targeting->SlotIndex != INDEX_NONE)
				{
					continue;
				}

				int32 BestSlot = INDEX_NONE;
				float BestSlotDistSq = TNumericLimits<float>::Max();
				for (int32 k = 0; k < SlotCount && k < 32; ++k)
				{
					if (Occupied & (1u << k))
					{
						continue;
					}
					const float DistSq = FVector::DistSquared(Attackers[a].Location, ComputeSlotLocation(Cand, k));
					if (DistSq < BestSlotDistSq)
					{
						BestSlotDistSq = DistSq;
						BestSlot = k;
					}
				}

				if (BestSlot != INDEX_NONE)
				{
					Targeting->SlotIndex = BestSlot;
					Occupied |= (1u << BestSlot);
				}
				else
				{
					Targeting->SlotIndex = 0;
				}
			}
		}
	}
}

void UMCTargetingProcessor::WriteResults(UWorld* World, bool bDrawSlots)
{
	AttackerByHandle.Reset();
	AttackerByHandle.Reserve(Attackers.Num());
	for (int32 a = 0; a < Attackers.Num(); ++a)
	{
		AttackerByHandle.Add(Attackers[a].Handle, a);
	}

	for (FMCAttacker& At : Attackers)
	{
		FMCTargetingFragment* Targeting = At.Targeting;
		if (At.TargetIdx != INDEX_NONE)
		{
			const FMCTargetCandidate& Cand = Candidates[At.TargetIdx];
			const int32 SlotIndex = FMath::Max(0, Targeting->SlotIndex);
			FVector SlotLocation = ComputeSlotLocation(Cand, SlotIndex);

			bool bReverse = false;
			if (const int32* OppPtr = AttackerByHandle.Find(Cand.Handle))
			{
				const FMCAttacker& Opp = Attackers[*OppPtr];
				if (Opp.TargetIdx != INDEX_NONE && Candidates[Opp.TargetIdx].Handle == At.Handle)
				{
					// Mutual targeting: decide which side takes the reversed slot.
					// One-sided -> the newcomer reverses; both held it -> keep prior; neither -> higher entity index reverses.
					const bool bMeWasTargeting = At.PrevTarget == Cand.Handle;
					const bool bOppWasTargeting = Opp.PrevTarget == At.Handle;

					if (bOppWasTargeting && !bMeWasTargeting)
					{
						bReverse = true;
					}
					else if (bMeWasTargeting && !bOppWasTargeting)
					{
						bReverse = false;
					}
					else if (bMeWasTargeting && bOppWasTargeting)
					{
						bReverse = Targeting->bReverseSlot != 0;
					}
					else
					{
						bReverse = At.EntityIndex > Opp.EntityIndex;
					}

					if (bReverse)
					{
						if (const int32* MeCandPtr = HandleToCand.Find(At.Handle))
						{
							const FVector AnchorOffset = ComputeSlotLocation(Candidates[*MeCandPtr], FMath::Max(0, Opp.Targeting->SlotIndex)) - At.Location;
							SlotLocation = Cand.Location - AnchorOffset;
						}
					}
				}
			}
			Targeting->bReverseSlot = bReverse;

			Targeting->bHasTarget = true;
			Targeting->CurrentTarget = Cand.Handle;
			Targeting->TargetLocation = Cand.Location;
			Targeting->SlotLocation = SlotLocation;
			Targeting->DistanceToTarget = FVector::Dist(At.Location, Cand.Location);
			Targeting->DistanceToSlot = FVector::Dist(At.Location, SlotLocation);
			Targeting->bHasNearestEnemy = true;
			Targeting->NearestEnemyLocation = Cand.Location;
			Targeting->DistanceToNearestEnemy = Targeting->DistanceToTarget;

			if (bDrawSlots && World)
			{
				const float DrawZ = At.Location.Z;
				const FVector SlotDraw(SlotLocation.X, SlotLocation.Y, DrawZ);
				const FVector TargetDraw(Cand.Location.X, Cand.Location.Y, DrawZ);
				DrawDebugSphere(World, SlotDraw, 20.f, 8, FColor::Green, false, -1.f, 0, 1.f);
				DrawDebugLine(World, At.Location, SlotDraw, FColor::Yellow, false, -1.f, 0, 1.f);
				DrawDebugLine(World, SlotDraw, TargetDraw, FColor::Red, false, -1.f, 0, 0.5f);
			}
		}
		else
		{
			Targeting->bHasTarget = false;
			Targeting->CurrentTarget = FMassEntityHandle();
			Targeting->SlotIndex = INDEX_NONE;
			Targeting->bReverseSlot = 0;
			Targeting->TargetLocation = FVector::ZeroVector;
			Targeting->SlotLocation = FVector::ZeroVector;
			Targeting->DistanceToTarget = TNumericLimits<float>::Max();
			Targeting->DistanceToSlot = TNumericLimits<float>::Max();

			int32 BestEnemy = INDEX_NONE;
			float BestEnemyDistSq = TNumericLimits<float>::Max();
			for (int32 c = 0; c < Candidates.Num(); ++c)
			{
				if (Candidates[c].Faction == At.Faction)
				{
					continue;
				}
				const float DistSq = FVector::DistSquared(At.Location, Candidates[c].Location);
				if (DistSq < BestEnemyDistSq)
				{
					BestEnemyDistSq = DistSq;
					BestEnemy = c;
				}
			}

			if (BestEnemy != INDEX_NONE)
			{
				const FMCTargetCandidate& Enemy = Candidates[BestEnemy];

				Targeting->bHasNearestEnemy = true;
				Targeting->NearestEnemyLocation = Enemy.Location;
				Targeting->DistanceToNearestEnemy = FMath::Sqrt(BestEnemyDistSq);
			}
			else
			{
				Targeting->bHasNearestEnemy = false;
				Targeting->NearestEnemyLocation = FVector::ZeroVector;
				Targeting->DistanceToNearestEnemy = TNumericLimits<float>::Max();
			}
		}
	}
}
