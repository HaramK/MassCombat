#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCNpcTargetingProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCNpcTargetingProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCNpcTargetingProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
