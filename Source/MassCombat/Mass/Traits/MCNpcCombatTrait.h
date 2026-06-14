#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcCombatTrait.generated.h"

UCLASS(meta = (DisplayName = "MC Npc Combat"))
class MASSCOMBAT_API UMCNpcCombatTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FMCNpcCombatParams Params;
};
