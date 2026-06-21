#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MCNpcTargetingProcessor.generated.h"

struct FMCNpcCombatFragment;

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
	FMCNpcCombatFragment* Combat;
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
class MASSCOMBAT_API UMCNpcTargetingProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCNpcTargetingProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

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
