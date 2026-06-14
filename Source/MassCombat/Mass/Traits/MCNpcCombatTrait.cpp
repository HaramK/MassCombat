#include "Mass/Traits/MCNpcCombatTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityUtils.h"

void UMCNpcCombatTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();

	BuildContext.AddFragment<FMCNpcCombatFragment>();
	BuildContext.AddFragment<FMCNpcAnimStateFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	const FConstSharedStruct ParamsFragment = EntityManager.GetOrCreateConstSharedFragment(Params);
	BuildContext.AddConstSharedFragment(ParamsFragment);
}
