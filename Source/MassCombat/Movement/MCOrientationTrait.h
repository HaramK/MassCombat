#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MCOrientationTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Orientation"))
class MASSCOMBAT_API UMCOrientationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
