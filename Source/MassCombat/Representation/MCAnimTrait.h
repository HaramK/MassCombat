#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Representation/MCRepresentationFragments.h"
#include "MCAnimTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Anim"))
class MASSCOMBAT_API UMCAnimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Anim")
	FMCAnimParams Params;
};
