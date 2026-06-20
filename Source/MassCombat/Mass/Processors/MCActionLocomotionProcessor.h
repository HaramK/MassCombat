#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCActionLocomotionProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCActionLocomotionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCActionLocomotionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
