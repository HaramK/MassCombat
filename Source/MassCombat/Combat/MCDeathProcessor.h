#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MCDeathProcessor.generated.h"

UCLASS()
class MASSCOMBAT_API UMCDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathDuration = 2.f;

	FMassEntityQuery DetectNpcQuery;
	FMassEntityQuery DetectPlayerQuery;
	FMassEntityQuery CleanupQuery;
};
