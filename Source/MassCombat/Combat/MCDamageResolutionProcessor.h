#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCDamageResolutionProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCDamageResolutionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCDamageResolutionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
