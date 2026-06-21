#include "Mass/Traits/MCUnitTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"

void UMCUnitTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMCUnitFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct InfoFragment = EntityManager.GetOrCreateConstSharedFragment(UnitInfo);
	BuildContext.AddConstSharedFragment(InfoFragment);
}
