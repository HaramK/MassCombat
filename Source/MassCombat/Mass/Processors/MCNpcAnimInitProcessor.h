#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MCNpcAnimInitProcessor.generated.h"

/**
 * FMCNpcAnimStateFragment가 추가될 때(=NPC 스폰) 1회 실행되는 옵저버.
 * GlobalStartTime을 무작위로 분산시켜 모든 인스턴스가 같은 박자로 재생되는 것을 막는다.
 */
UCLASS()
class MASSCOMBAT_API UMCNpcAnimInitProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMCNpcAnimInitProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
