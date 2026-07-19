#include "Targeting/MCTargetingProcessor.h"
#include "Targeting/MCTargetingFragments.h"
#include "Combat/MCCombatFragments.h"
#include "Unit/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Core/MCTargetingSettings.h"
#include "MassNavigationSubsystem.h"
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
	GatherQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);
	GatherQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCTargetingFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCEngagementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMCDeadTag>(EMassFragmentPresence::None);
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

	NavSubsystem = World ? World->GetSubsystem<UMassNavigationSubsystem>() : nullptr;

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
	WriteResults(Context, World, bDrawSlots);
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
		const TConstArrayView<FMCTargetingFragment> Targetings = Ctx.GetFragmentView<FMCTargetingFragment>();
		const TConstArrayView<FMCEngagementFragment> Engagements = Ctx.GetFragmentView<FMCEngagementFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			Attackers.Add({ Transforms[i].GetTransform().GetLocation(), Faction, Ctx.GetEntity(i).Index, INDEX_NONE, Targetings[i].SlotIndex, INDEX_NONE, Targetings[i].NextRetargetTime,
				Ctx.GetEntity(i), Targetings[i].CurrentTarget,
				Engagements[i].LastAttackerUnit, Engagements[i].LastDamagedTime, Engagements[i].LastAttackTime, Params.bPreferPlayerTarget, PlayerRadiusSq, Params.TargetSearchRadius });
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
		if (At.PrevTarget != PlayerHandle)
		{
			At.SlotIndex = INDEX_NONE;
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

		const FMassEntityHandle OldTarget = At.PrevTarget;
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
				At.SlotIndex = INDEX_NONE;
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

		if (!bOldValid || Now >= At.NextRetargetTime)
		{
			At.NextRetargetTime = Now + RetargetInterval + FMath::FRandRange(0.f, RetargetInterval);
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
	TArray<FMassNavigationObstacleItem, TInlineAllocator<64>> Nearby;

	for (FMCAttacker& At : Attackers)
	{
		if (At.TargetIdx != INDEX_NONE)
		{
			continue;
		}

		const float SearchRadiusSq = At.SearchRadius * At.SearchRadius;

		int32 BestOpen = INDEX_NONE;
		float BestOpenDistSq = SearchRadiusSq;
		int32 BestAny = INDEX_NONE;
		float BestAnyDistSq = SearchRadiusSq;

		if (NavSubsystem)
		{
			Nearby.Reset();
			const FVector Extent(At.SearchRadius, At.SearchRadius, 0.f);
			const FBox QueryBox(At.Location - Extent, At.Location + Extent);
			NavSubsystem->GetObstacleGrid().Query(QueryBox, Nearby);

			for (const FMassNavigationObstacleItem& Item : Nearby)
			{
				const int32* Ci = HandleToCand.Find(Item.Entity);
				if (!Ci)
				{
					continue;
				}

				const int32 c = *Ci;
				if (Candidates[c].Faction == At.Faction)
				{
					continue;
				}

				const float DistSq = FVector::DistSquared(At.Location, Candidates[c].Location);
				if (DistSq >= SearchRadiusSq)
				{
					continue;
				}

				if (DistSq < BestAnyDistSq)
				{
					BestAnyDistSq = DistSq;
					BestAny = c;
				}

				if (!CandTargetsPlayer[c] && AssignedCount[c] < Candidates[c].MaxAttackers && DistSq < BestOpenDistSq)
				{
					BestOpenDistSq = DistSq;
					BestOpen = c;
				}
			}
		}

		At.NearestEnemyIdx = BestAny;

		const int32 Chosen = BestOpen;
		if (Chosen != INDEX_NONE)
		{
			const FMassEntityHandle OldTarget = At.PrevTarget;
			const int32* OldFound = OldTarget.IsSet() ? HandleToCand.Find(OldTarget) : nullptr;

			At.TargetIdx = Chosen;
			++AssignedCount[Chosen];
			if (!OldFound || *OldFound != Chosen)
			{
				At.SlotIndex = INDEX_NONE;
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
				Attackers[Group[r]].SlotIndex = r;
			}
		}
		else
		{
			SlotOccupied.Reset();
			SlotOccupied.SetNumZeroed(SlotCount);

			for (int32 a : Group)
			{
				int32& Slot = Attackers[a].SlotIndex;
				if (Slot >= 0 && Slot < SlotCount && !SlotOccupied[Slot])
				{
					SlotOccupied[Slot] = true;
				}
				else
				{
					Slot = INDEX_NONE;
				}
			}

			for (int32 a : Group)
			{
				int32& Slot = Attackers[a].SlotIndex;
				if (Slot != INDEX_NONE)
				{
					continue;
				}

				int32 BestSlot = INDEX_NONE;
				float BestSlotDistSq = TNumericLimits<float>::Max();
				for (int32 k = 0; k < SlotCount; ++k)
				{
					if (SlotOccupied[k])
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
					Slot = BestSlot;
					SlotOccupied[BestSlot] = true;
				}
				else
				{
					Slot = 0;
				}
			}
		}
	}
}

void UMCTargetingProcessor::DetectMutualCycles()
{
	const int32 Num = Attackers.Num();

	CycleNext.Reset();
	CycleNext.SetNumUninitialized(Num);
	InCycle.Reset();
	InCycle.SetNumZeroed(Num);
	CycleClassified.Reset();
	CycleClassified.SetNumZeroed(Num);
	CyclePathMark.Reset();
	CyclePathMark.SetNumZeroed(Num);

	for (int32 a = 0; a < Num; ++a)
	{
		int32 Next = INDEX_NONE;
		if (Attackers[a].TargetIdx != INDEX_NONE)
		{
			if (const int32* OppPtr = AttackerByHandle.Find(Candidates[Attackers[a].TargetIdx].Handle))
			{
				Next = *OppPtr;
			}
		}
		CycleNext[a] = Next;
	}

	int32 Mark = 0;
	for (int32 Start = 0; Start < Num; ++Start)
	{
		if (CycleClassified[Start])
		{
			continue;
		}

		++Mark;
		CyclePath.Reset();

		int32 Cur = Start;
		while (Cur != INDEX_NONE && !CycleClassified[Cur] && CyclePathMark[Cur] != Mark)
		{
			CyclePathMark[Cur] = Mark;
			CyclePath.Add(Cur);
			Cur = CycleNext[Cur];
		}

		if (Cur != INDEX_NONE && CyclePathMark[Cur] == Mark)
		{
			int32 CycleStart = 0;
			while (CyclePath[CycleStart] != Cur)
			{
				++CycleStart;
			}
			for (int32 i = CycleStart; i < CyclePath.Num(); ++i)
			{
				InCycle[CyclePath[i]] = 1;
			}
		}

		for (int32 Node : CyclePath)
		{
			CycleClassified[Node] = 1;
		}
	}
}

void UMCTargetingProcessor::WriteResults(FMassExecutionContext& Context, UWorld* World, bool bDrawSlots)
{
	AttackerByHandle.Reset();
	AttackerByHandle.Reserve(Attackers.Num());
	for (int32 a = 0; a < Attackers.Num(); ++a)
	{
		AttackerByHandle.Add(Attackers[a].Handle, a);
	}

	DetectMutualCycles();

	int32 a = 0;
	EntityQuery.ForEachEntityChunk(Context, [this, World, bDrawSlots, &a](FMassExecutionContext& Ctx)
	{
		const TArrayView<FMCTargetingFragment> Targetings = Ctx.GetMutableFragmentView<FMCTargetingFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i, ++a)
		{
			const FMCAttacker& At = Attackers[a];
			checkf(At.Handle == Ctx.GetEntity(i), TEXT("Attacker order diverged from GatherAttackers chunk iteration"));
			FMCTargetingFragment& Targeting = Targetings[i];

			Targeting.NextRetargetTime = At.NextRetargetTime;

			if (At.TargetIdx != INDEX_NONE)
			{
				const FMCTargetCandidate& Cand = Candidates[At.TargetIdx];
				Targeting.SlotIndex = At.SlotIndex;
				const FVector SlotLocation = InCycle[a] ? Cand.Location : ComputeSlotLocation(Cand, FMath::Max(0, At.SlotIndex));

				Targeting.bHasTarget = true;
				Targeting.CurrentTarget = Cand.Handle;
				Targeting.TargetLocation = Cand.Location;
				Targeting.SlotLocation = SlotLocation;
				Targeting.DistanceToTarget = FVector::Dist(At.Location, Cand.Location);
				Targeting.DistanceToSlot = FVector::Dist(At.Location, SlotLocation);
				Targeting.bHasNearestEnemy = true;
				Targeting.NearestEnemyLocation = Cand.Location;
				Targeting.DistanceToNearestEnemy = Targeting.DistanceToTarget;

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
				Targeting.bHasTarget = false;
				Targeting.CurrentTarget = FMassEntityHandle();
				Targeting.SlotIndex = INDEX_NONE;
				Targeting.TargetLocation = FVector::ZeroVector;
				Targeting.SlotLocation = FVector::ZeroVector;
				Targeting.DistanceToTarget = TNumericLimits<float>::Max();
				Targeting.DistanceToSlot = TNumericLimits<float>::Max();

				const int32 BestEnemy = At.NearestEnemyIdx;
				if (BestEnemy != INDEX_NONE)
				{
					Targeting.bHasNearestEnemy = true;
					Targeting.NearestEnemyLocation = Candidates[BestEnemy].Location;
					Targeting.DistanceToNearestEnemy = FVector::Dist(At.Location, Candidates[BestEnemy].Location);
				}
				else
				{
					Targeting.bHasNearestEnemy = false;
					Targeting.NearestEnemyLocation = FVector::ZeroVector;
					Targeting.DistanceToNearestEnemy = TNumericLimits<float>::Max();
				}
			}
		}
	});
}
