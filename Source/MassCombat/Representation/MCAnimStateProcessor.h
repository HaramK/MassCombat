#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCAnimStateProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCAnimStateProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCAnimStateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
