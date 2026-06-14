#include "Mass/StateTree/MCNpcFindTargetEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"

bool FMCNpcFindTargetEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(CombatHandle);
	return true;
}

void FMCNpcFindTargetEvaluator::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly<FMCNpcCombatFragment>();
}

void FMCNpcFindTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const FMCNpcCombatFragment& Combat = Context.GetExternalData(CombatHandle);

	Data.bHasTarget = Combat.bHasTarget;
	Data.DistanceToTarget = Combat.DistanceToTarget;
	Data.bInAttackRange = Combat.bHasTarget && Combat.DistanceToTarget <= AttackRange;
}
