#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MCPlayerTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Player"))
class MASSCOMBAT_API UMCPlayerTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
