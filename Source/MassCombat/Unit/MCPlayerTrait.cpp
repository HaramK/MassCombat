#include "Unit/MCPlayerTrait.h"
#include "Unit/MCUnitFragments.h"
#include "MassEntityTemplateRegistry.h"

void UMCPlayerTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FMCPlayerTag>();
}
