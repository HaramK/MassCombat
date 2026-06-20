#include "Mass/Processors/MCNpcUpdateISMAnimProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MassExecutionContext.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "AnimToTextureDataAsset.h"
#include "AnimToTextureInstancePlaybackHelpers.h"
#include "DrawDebugHelpers.h" // TODO(debug): 임시 — 확인 후 제거

UMCNpcUpdateISMAnimProcessor::UMCNpcUpdateISMAnimProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcAnimStateProcessor"));
}

void UMCNpcUpdateISMAnimProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadOnly);
}

void UMCNpcUpdateISMAnimProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();

	EntityQuery.ForEachEntityChunk(Context, [World](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
	
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TConstArrayView<FMCNpcAnimStateFragment> AnimList = Context.GetFragmentView<FMCNpcAnimStateFragment>();
	
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIt];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIt];
			FMassRepresentationFragment& Representation = RepresentationList[EntityIt];
			const FMCNpcAnimStateFragment& AnimationDataList = AnimList[EntityIt];
	
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
				if (ISMInfo.IsValidIndex(ISMInfoIndex))
				{
					UpdateISMTransform(Context.GetEntity(EntityIt), ISMInfo[ISMInfoIndex]
						, TransformFragment.GetTransform(), Representation.PrevTransform
						, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
	
					FAnimToTextureAutoPlayData AutoPlay;
					const bool bGotData = UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(
							AnimationDataList.AnimData.Get(), AnimationDataList.StateIndex, AutoPlay, 0.f, AnimationDataList.PlayRate);
					{
	
						ISMInfo[ISMInfoIndex].AddBatchedCustomData<FAnimToTextureAutoPlayData>(
							AutoPlay, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					}

					// TODO(debug): 임시 — 엔티티별로 push되는 custom data 값 화면 표시. 확인 후 제거.
					if (World)
					{
						DrawDebugString(World, TransformFragment.GetTransform().GetLocation() + FVector(0.f, 0.f, 150.f),
							bGotData
								? FString::Printf(TEXT("idx=%d SF=%.0f EF=%.0f TO=%.2f PR=%.2f"), AnimationDataList.StateIndex, AutoPlay.StartFrame, AutoPlay.EndFrame, AutoPlay.TimeOffset, AutoPlay.PlayRate)
								: FString::Printf(TEXT("idx=%d NO DATA (AnimData/StateIndex invalid)"), AnimationDataList.StateIndex),
							nullptr, FColor::Yellow, 0.f);
					}
				}
			}
			Representation.PrevTransform = TransformFragment.GetTransform();
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}

