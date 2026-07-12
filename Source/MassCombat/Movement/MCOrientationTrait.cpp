#include "Movement/MCOrientationTrait.h"
#include "Movement/MCOrientationFragments.h"
#include "MassEntityTemplateRegistry.h"

void UMCOrientationTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMCOrientationFragment>();
}
