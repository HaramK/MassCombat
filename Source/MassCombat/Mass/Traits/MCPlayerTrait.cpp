#include "Mass/Traits/MCPlayerTrait.h"
#include "Mass/Fragments/MCUnitFragments.h"
#include "MassEntityTemplateRegistry.h"

void UMCPlayerTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMCPlayerTag>();
}
