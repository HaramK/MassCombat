#pragma once

#include "CoreMinimal.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MCSpiralSpawnPointsGenerator.generated.h"

UCLASS(meta = (DisplayName = "MC Spiral Spawn Points"))
class MASSCOMBAT_API UMCSpiralSpawnPointsGenerator : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()

public:
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	float Spacing = 120.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bProjectToNavmesh = true;
};
