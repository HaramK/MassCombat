#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MCAnimInitProcessor.generated.h"

/**
 * Observer that runs once when FMCAnimStateFragment is added (i.e. on NPC spawn).
 * Randomizes GlobalStartTime so instances do not all play in lockstep.
 */
UCLASS()

class MASSCOMBAT_API UMCAnimInitProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMCAnimInitProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
