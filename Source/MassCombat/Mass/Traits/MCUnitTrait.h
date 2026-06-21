#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Mass/Fragments/MCUnitFragments.h"
#include "MCUnitTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Unit"))
class MASSCOMBAT_API UMCUnitTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Unit")
	FMCUnitInfoFragment UnitInfo;
};
