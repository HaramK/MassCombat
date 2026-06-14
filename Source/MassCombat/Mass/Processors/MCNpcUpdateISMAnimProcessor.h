#pragma once

#include "CoreMinimal.h"
#include "MassUpdateISMProcessor.h"
#include "MCNpcUpdateISMAnimProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCNpcUpdateISMAnimProcessor : public UMassUpdateISMProcessor
{
	GENERATED_BODY()

public:
	UMCNpcUpdateISMAnimProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
