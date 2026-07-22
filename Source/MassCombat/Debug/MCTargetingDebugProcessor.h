#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCTargetingDebugProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCTargetingDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCTargetingDebugProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
