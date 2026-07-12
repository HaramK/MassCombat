#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Targeting/MCTargetingFragments.h"
#include "Representation/MCRepresentationFragments.h"
#include "MCCombatTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Combat"))
class MASSCOMBAT_API UMCCombatTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	FMCTargetingParams TargetingParams;

	UPROPERTY(EditAnywhere, Category = "Anim")
	FMCAnimParams AnimParams;
};
