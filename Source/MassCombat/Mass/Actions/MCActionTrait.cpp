#include "Mass/Actions/MCActionTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"

void UMCActionTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMCActionFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct ActionSetFragment = EntityManager.GetOrCreateConstSharedFragment(ActionSet);
	BuildContext.AddConstSharedFragment(ActionSetFragment);
}
