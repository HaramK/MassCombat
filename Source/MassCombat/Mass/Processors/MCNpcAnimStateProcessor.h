#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCNpcAnimStateProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCNpcAnimStateProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCNpcAnimStateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
