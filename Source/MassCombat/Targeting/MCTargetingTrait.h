#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Targeting/MCTargetingFragments.h"
#include "MCTargetingTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Targeting"))
class MASSCOMBAT_API UMCTargetingTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	FMCTargetingParams Params;
};
