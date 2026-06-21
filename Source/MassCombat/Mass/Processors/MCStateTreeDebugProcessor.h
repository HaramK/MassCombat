#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCStateTreeDebugProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCStateTreeDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCStateTreeDebugProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
