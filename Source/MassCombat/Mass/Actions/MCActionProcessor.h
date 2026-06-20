#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCActionProcessor.generated.h"


UCLASS()
class MASSCOMBAT_API UMCActionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCActionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
