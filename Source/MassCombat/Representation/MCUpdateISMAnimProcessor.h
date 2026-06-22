#pragma once

#include "CoreMinimal.h"
#include "MassUpdateISMProcessor.h"
#include "MCUpdateISMAnimProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCUpdateISMAnimProcessor : public UMassUpdateISMProcessor
{
	GENERATED_BODY()

public:
	UMCUpdateISMAnimProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
