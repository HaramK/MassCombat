#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCNpcDeathProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCNpcDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCNpcDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
