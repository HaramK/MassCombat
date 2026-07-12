#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MCTargetingProcessor.generated.h"

struct FMCCombatFragment;
class UWorld;

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
	FMCCombatFragment* Combat;
	FVector Location;
	uint8 Faction;
	int32 EntityIndex;
	int32 TargetIdx;
	FMassEntityHandle Handle;
	FMassEntityHandle PrevTarget;
	bool bPreferPlayer;
	float PlayerRadiusSq;
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
	void AssignOpenTargets();
	void AssignSlots();
	void WriteResults(UWorld* World, bool bDrawSlots);

	FMassEntityQuery GatherQuery;
	FMassEntityQuery EntityQuery;

	TArray<FMCTargetCandidate> Candidates;
	TArray<FMCAttacker> Attackers;
	TMap<FMassEntityHandle, int32> HandleToCand;
	TMap<FMassEntityHandle, int32> AttackerByHandle;
	TArray<int32> AssignedCount;
	TArray<bool> CandTargetsPlayer;
	TArray<int32> Contenders;
	TArray<TArray<int32>> Groups;
};
