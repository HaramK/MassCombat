#include "Mass/Processors/MCNpcTargetingProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "Mass/Fragments/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Core/MCCombatSettings.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

static TAutoConsoleVariable<bool> CVarDrawTargetSlots(
	TEXT("mc.DrawTargetSlots"),
	false,
	TEXT("Draw attacker slot positions / target links for MassCombat targeting."));

UMCNpcTargetingProcessor::UMCNpcTargetingProcessor()
	: GatherQuery(*this)
	, EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCNpcTargetingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	GatherQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();
	EntityQuery.AddConstSharedRequirement<FMCNpcCombatParams>();
}

namespace
{
	FVector ComputeSlotLocation(const FMCTargetCandidate& Cand, int32 SlotIndex)
	{
		const int32 SlotCount = FMath::Max(1, Cand.MaxAttackers);
		const float BaseAngle = Cand.Handle.Index * 2.3999632f;
		const float Angle = BaseAngle + (2.f * UE_PI / SlotCount) * SlotIndex;
		return Cand.Location + FVector(FMath::Cos(Angle) * Cand.SlotRadius, FMath::Sin(Angle) * Cand.SlotRadius, 0.f);
	}
}

void UMCNpcTargetingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float DeltaTime = World ? World->GetDeltaSeconds() : 0.f;
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const bool bDrawSlots = CVarDrawTargetSlots.GetValueOnGameThread();

	const UMCCombatSettings* Settings = GetDefault<UMCCombatSettings>();
	const float CombatWindow = Settings->CombatWindow;
	const float RetargetInterval = Settings->RetargetInterval;

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

	Attackers.Reset();

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Ctx)
	{
		const uint8 Faction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const FMCNpcCombatParams& Params = Ctx.GetConstSharedFragment<FMCNpcCombatParams>();
		const float PlayerRadiusSq = Params.PlayerTargetRadius * Params.PlayerTargetRadius;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCNpcCombatFragment> Combats = Ctx.GetMutableFragmentView<FMCNpcCombatFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			Attackers.Add({ &Combats[i], Transforms[i].GetTransform().GetLocation(), Faction, Ctx.GetEntity(i).Index, INDEX_NONE, Ctx.GetEntity(i), Combats[i].CurrentTarget, Params.bPreferPlayerTarget, PlayerRadiusSq });
		}
	});

	AssignedCount.Reset();
	AssignedCount.AddZeroed(Candidates.Num());

	int32 PlayerCandIdx = INDEX_NONE;
	for (int32 c = 0; c < Candidates.Num(); ++c)
	{
		if (Candidates[c].bIsPlayer)
		{
			PlayerCandIdx = c;
			break;
		}
	}

	CandTargetsPlayer.Reset();
	CandTargetsPlayer.AddZeroed(Candidates.Num());
	if (PlayerCandIdx != INDEX_NONE)
	{
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

	if (PlayerCandIdx != INDEX_NONE)
	{
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
			if (At.Combat->CurrentTarget != PlayerHandle)
			{
				At.Combat->SlotIndex = INDEX_NONE;
			}
			At.TargetIdx = PlayerCandIdx;
			++AssignedCount[PlayerCandIdx];
		}
	}

	for (FMCAttacker& At : Attackers)
	{
		if (At.TargetIdx != INDEX_NONE)
		{
			continue;
		}

		FMCNpcCombatFragment* Combat = At.Combat;
		const FMassEntityHandle OldTarget = Combat->CurrentTarget;
		const int32* OldFound = OldTarget.IsSet() ? HandleToCand.Find(OldTarget) : nullptr;
		const bool bOldValid = OldFound && (Candidates[*OldFound].Faction != At.Faction) && !CandTargetsPlayer[*OldFound];

		const FMassEntityHandle Attacker = Combat->LastAttackerUnit;
		const int32* AtkFound = (Attacker.IsSet() && Attacker != OldTarget) ? HandleToCand.Find(Attacker) : nullptr;
		const bool bRetaliate = AtkFound && (Candidates[*AtkFound].Faction != At.Faction)
			&& !CandTargetsPlayer[*AtkFound]
			&& ((Now - Combat->LastDamagedTime) < CombatWindow)
			&& (AssignedCount[*AtkFound] < Candidates[*AtkFound].MaxAttackers);

		if (bRetaliate)
		{
			if (!bOldValid || *AtkFound != *OldFound)
			{
				Combat->SlotIndex = INDEX_NONE;
			}
			At.TargetIdx = *AtkFound;
			++AssignedCount[*AtkFound];
			continue;
		}

		const bool bRealCombat = bOldValid && (((Now - Combat->LastAttackTime) < CombatWindow) || ((Now - Combat->LastDamagedTime) < CombatWindow));
		if (bRealCombat && AssignedCount[*OldFound] < Candidates[*OldFound].MaxAttackers)
		{
			At.TargetIdx = *OldFound;
			++AssignedCount[*OldFound];
			continue;
		}

		if (!bOldValid || Now >= Combat->NextRetargetTime)
		{
			Combat->NextRetargetTime = Now + RetargetInterval + (At.EntityIndex % 257) / 257.f * RetargetInterval;
			continue;
		}

		if (AssignedCount[*OldFound] < Candidates[*OldFound].MaxAttackers)
		{
			At.TargetIdx = *OldFound;
			++AssignedCount[*OldFound];
		}
	}

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
			const FMassEntityHandle OldTarget = At.Combat->CurrentTarget;
			const int32* OldFound = OldTarget.IsSet() ? HandleToCand.Find(OldTarget) : nullptr;

			At.TargetIdx = Chosen;
			++AssignedCount[Chosen];
			if (!OldFound || *OldFound != Chosen)
			{
				At.Combat->SlotIndex = INDEX_NONE;
			}
		}
	}

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
				Attackers[Group[r]].Combat->SlotIndex = r;
			}
		}
		else
		{
			uint32 Occupied = 0;
			for (int32 a : Group)
			{
				FMCNpcCombatFragment* Combat = Attackers[a].Combat;
				if (Combat->SlotIndex >= 0 && Combat->SlotIndex < SlotCount && Combat->SlotIndex < 32 && !(Occupied & (1u << Combat->SlotIndex)))
				{
					Occupied |= (1u << Combat->SlotIndex);
				}
				else
				{
					Combat->SlotIndex = INDEX_NONE;
				}
			}

			for (int32 a : Group)
			{
				FMCNpcCombatFragment* Combat = Attackers[a].Combat;
				if (Combat->SlotIndex != INDEX_NONE)
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
					Combat->SlotIndex = BestSlot;
					Occupied |= (1u << BestSlot);
				}
				else
				{
					Combat->SlotIndex = 0;
				}
			}
		}
	}

	AttackerByHandle.Reset();
	AttackerByHandle.Reserve(Attackers.Num());
	for (int32 a = 0; a < Attackers.Num(); ++a)
	{
		AttackerByHandle.Add(Attackers[a].Handle, a);
	}

	for (FMCAttacker& At : Attackers)
	{
		FMCNpcCombatFragment* Combat = At.Combat;
		if (At.TargetIdx != INDEX_NONE)
		{
			const FMCTargetCandidate& Cand = Candidates[At.TargetIdx];
			const int32 SlotIndex = FMath::Max(0, Combat->SlotIndex);
			FVector SlotLocation = ComputeSlotLocation(Cand, SlotIndex);

			bool bReverse = false;
			if (const int32* OppPtr = AttackerByHandle.Find(Cand.Handle))
			{
				const FMCAttacker& Opp = Attackers[*OppPtr];
				if (Opp.TargetIdx != INDEX_NONE && Candidates[Opp.TargetIdx].Handle == At.Handle)
				{
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
						bReverse = Combat->bReverseSlot != 0;
					}
					else
					{
						bReverse = At.EntityIndex > Opp.EntityIndex;
					}

					if (bReverse)
					{
						if (const int32* MeCandPtr = HandleToCand.Find(At.Handle))
						{
							const FVector AnchorOffset = ComputeSlotLocation(Candidates[*MeCandPtr], FMath::Max(0, Opp.Combat->SlotIndex)) - At.Location;
							SlotLocation = Cand.Location - AnchorOffset;
						}
					}
				}
			}
			Combat->bReverseSlot = bReverse;

			Combat->bHasTarget = true;
			Combat->CurrentTarget = Cand.Handle;
			Combat->TargetLocation = Cand.Location;
			Combat->SlotLocation = SlotLocation;
			Combat->DistanceToTarget = FVector::Dist(At.Location, Cand.Location);
			Combat->DistanceToSlot = FVector::Dist(At.Location, SlotLocation);
			Combat->bHasNearestEnemy = true;
			Combat->bHasLoiterOffset = false;
			Combat->NearestEnemyLocation = Cand.Location;
			Combat->DistanceToNearestEnemy = Combat->DistanceToTarget;
			Combat->LoiterLocation = Cand.Location;
			Combat->DistanceToLoiter = Combat->DistanceToTarget;

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
			Combat->bHasTarget = false;
			Combat->CurrentTarget = FMassEntityHandle();
			Combat->SlotIndex = INDEX_NONE;
			Combat->bReverseSlot = 0;
			Combat->TargetLocation = FVector::ZeroVector;
			Combat->SlotLocation = FVector::ZeroVector;
			Combat->DistanceToTarget = TNumericLimits<float>::Max();
			Combat->DistanceToSlot = TNumericLimits<float>::Max();

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

				if (!Combat->bHasLoiterOffset)
				{
					const float Angle = FMath::FRandRange(0.f, 2.f * UE_PI);
					const float Radius = FMath::FRandRange(Combat->LoiterMinRadius, Combat->LoiterMaxRadius);
					Combat->LoiterOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
					Combat->bHasLoiterOffset = true;
				}

				Combat->bHasNearestEnemy = true;
				Combat->NearestEnemyLocation = Enemy.Location;
				Combat->DistanceToNearestEnemy = FMath::Sqrt(BestEnemyDistSq);
				Combat->LoiterLocation = Enemy.Location + Combat->LoiterOffset;
				Combat->DistanceToLoiter = FVector::Dist(At.Location, Combat->LoiterLocation);
			}
			else
			{
				Combat->bHasNearestEnemy = false;
				Combat->bHasLoiterOffset = false;
				Combat->NearestEnemyLocation = FVector::ZeroVector;
				Combat->DistanceToNearestEnemy = TNumericLimits<float>::Max();
				Combat->LoiterLocation = FVector::ZeroVector;
				Combat->DistanceToLoiter = TNumericLimits<float>::Max();
			}
		}

		Combat->AttackCooldownRemaining = FMath::Max(0.f, Combat->AttackCooldownRemaining - DeltaTime);
	}
}
