#include "Mass/Processors/MCNpcTargetingProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "Mass/Fragments/MCUnitFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"

UMCNpcTargetingProcessor::UMCNpcTargetingProcessor()
	: GatherQuery(*this)
	, EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	bRequiresGameThreadExecution = true;
}

void UMCNpcTargetingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	GatherQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddRequirement<FMCUnitFragment>(EMassFragmentAccess::ReadOnly);
	GatherQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMCUnitInfoFragment>();
}

namespace
{
	struct FMCTargetCandidate
	{
		FMassEntityHandle Handle;
		FVector Location;
		uint8 Faction;
	};
}

void UMCNpcTargetingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float DeltaTime = World ? World->GetDeltaSeconds() : 0.f;

	TArray<FMCTargetCandidate> Candidates;

	GatherQuery.ForEachEntityChunk(Context, [&Candidates](FMassExecutionContext& Ctx)
	{
		const uint8 Faction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMCUnitFragment> Units = Ctx.GetFragmentView<FMCUnitFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			if (Units[i].Health > 0.f)
			{
				Candidates.Add({ Ctx.GetEntity(i), Transforms[i].GetTransform().GetLocation(), Faction });
			}
		}
	});

	EntityQuery.ForEachEntityChunk(Context, [&Candidates, DeltaTime](FMassExecutionContext& Ctx)
	{
		const uint8 MyFaction = Ctx.GetConstSharedFragment<FMCUnitInfoFragment>().Faction;
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCNpcCombatFragment> Combats = Ctx.GetMutableFragmentView<FMCNpcCombatFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCNpcCombatFragment& Combat = Combats[i];
			const FVector Location = Transforms[i].GetTransform().GetLocation();

			float BestDistSq = TNumericLimits<float>::Max();
			FMassEntityHandle BestHandle;
			FVector BestLocation = FVector::ZeroVector;

			for (const FMCTargetCandidate& Candidate : Candidates)
			{
				if (Candidate.Faction == MyFaction)
				{
					continue;
				}

				const float DistSq = FVector::DistSquared(Location, Candidate.Location);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestHandle = Candidate.Handle;
					BestLocation = Candidate.Location;
				}
			}

			const bool bFound = BestHandle.IsSet();
			Combat.bHasTarget = bFound;
			Combat.CurrentTarget = BestHandle;
			Combat.TargetLocation = bFound ? BestLocation : FVector::ZeroVector;
			Combat.DistanceToTarget = bFound ? FMath::Sqrt(BestDistSq) : TNumericLimits<float>::Max();

			Combat.AttackCooldownRemaining = FMath::Max(0.f, Combat.AttackCooldownRemaining - DeltaTime);
		}
	});
}
