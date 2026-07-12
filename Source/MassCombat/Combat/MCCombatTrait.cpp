#include "Combat/MCCombatTrait.h"
#include "Combat/MCCombatFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityUtils.h"

void UMCCombatTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();

	BuildContext.AddFragment<FMCCombatFragment>();
	BuildContext.AddFragment<FMCEngagementFragment>();
	BuildContext.AddFragment<FMCAnimStateFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(TargetingParams));
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(AnimParams));
}
