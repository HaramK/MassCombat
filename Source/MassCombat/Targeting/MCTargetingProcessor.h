#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MCTargetingProcessor.generated.h"

struct FMCTargetingFragment;
class UWorld;
class UMassNavigationSubsystem;

struct FMCTargetCandidate
{
	FMassEntityHandle Handle;
	FVector Location;
	uint8 Faction;
	int32 MaxAttackers;
	float SlotRadius;
	bool bRecalc;
	bool bIsPlayer;
};

struct FMCAttacker
{
	FVector Location;
	uint8 Faction;
	int32 EntityIndex;
	int32 TargetIdx;
	int32 SlotIndex;
	int32 NearestEnemyIdx;
	float NextRetargetTime;
	FMassEntityHandle Handle;
	FMassEntityHandle PrevTarget;
	FMassEntityHandle LastAttackerUnit;
	float LastDamagedTime;
	float LastAttackTime;
	bool bPreferPlayer;
	float PlayerRadiusSq;
	float SearchRadius;
	bool bSearched = false;
};

UCLASS()
class MASSCOMBAT_API UMCTargetingProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCTargetingProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	void GatherCandidates(FMassExecutionContext& Context);
	void GatherAttackers(FMassExecutionContext& Context);
	int32 FindPlayerCandidate() const;
	void MarkPlayerTargetedCandidates(int32 PlayerCandIdx);
	void AssignPlayerTargets(int32 PlayerCandIdx);
	void AssignReturningTargets(float Now, float CombatWindow, float RetargetInterval);
	void AssignOpenTargets(float Now, float RetargetInterval);
	void AssignSlots();
	void DetectMutualCycles();
	void WriteResults(FMassExecutionContext& Context, UWorld* World, bool bDrawSlots);

	FMassEntityQuery GatherQuery;
	FMassEntityQuery EntityQuery;

	UMassNavigationSubsystem* NavSubsystem = nullptr;

	TArray<FMCTargetCandidate> Candidates;
	TArray<FMCAttacker> Attackers;
	TMap<FMassEntityHandle, int32> HandleToCand;
	TMap<FMassEntityHandle, int32> AttackerByHandle;
	TArray<int32> AssignedCount;
	TArray<bool> CandTargetsPlayer;
	TArray<int32> Contenders;
	TArray<TArray<int32>> Groups;

	TArray<int32> CycleNext;
	TArray<uint8> InCycle;
	TArray<uint8> CycleClassified;
	TArray<int32> CyclePathMark;
	TArray<int32> CyclePath;
	TArray<bool> SlotOccupied;
};
