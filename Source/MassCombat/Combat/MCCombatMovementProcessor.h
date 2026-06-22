#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCCombatMovementProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCCombatMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCCombatMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};