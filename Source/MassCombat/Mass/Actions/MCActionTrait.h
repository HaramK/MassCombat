#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Mass/Actions/MCActionFragments.h"
#include "MCActionTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Action"))
class MASSCOMBAT_API UMCActionTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Action")
	FMCActionSetParams ActionSet;
};
