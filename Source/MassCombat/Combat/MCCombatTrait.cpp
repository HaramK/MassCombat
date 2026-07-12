#include "Combat/MCCombatTrait.h"
#include "Combat/MCCombatFragments.h"
#include "MassEntityTemplateRegistry.h"

void UMCCombatTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMCEngagementFragment>();
	BuildContext.AddFragment<FMCDeathFragment>();
}
