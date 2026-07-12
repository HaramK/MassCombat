#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCOrientationIntentProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCOrientationIntentProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCOrientationIntentProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
