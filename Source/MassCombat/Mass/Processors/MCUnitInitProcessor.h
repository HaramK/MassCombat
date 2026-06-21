#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MCUnitInitProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCUnitInitProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMCUnitInitProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
