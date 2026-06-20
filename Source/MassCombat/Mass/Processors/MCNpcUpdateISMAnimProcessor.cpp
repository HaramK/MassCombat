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
#include "VisualLogger/VisualLogger.h"

UMCNpcUpdateISMAnimProcessor::UMCNpcUpdateISMAnimProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcAnimStateProcessor"));
}

void UMCNpcUpdateISMAnimProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCNpcUpdateISMAnimProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();

	EntityQuery.ForEachEntityChunk(Context, [this, World](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		const float DeltaTime = World ? World->GetDeltaSeconds() : 0.f;

		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassRepresentationFragment> RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FMCNpcAnimStateFragment> AnimList = Context.GetMutableFragmentView<FMCNpcAnimStateFragment>();
	
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIt];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIt];
			FMassRepresentationFragment& Representation = RepresentationList[EntityIt];
			FMCNpcAnimStateFragment& AnimationDataList = AnimList[EntityIt];
	
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
				if (ISMInfo.IsValidIndex(ISMInfoIndex))
				{
					UpdateISMTransform(Context.GetEntity(EntityIt), ISMInfo[ISMInfoIndex]
						, TransformFragment.GetTransform(), Representation.PrevTransform
						, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
	
					FAnimToTextureAutoPlayData Range;
					const bool bGotData = UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(
							AnimationDataList.AnimData.Get(), AnimationDataList.StateIndex, Range, 0.f, AnimationDataList.PlayRate);

					FAnimToTextureFrameData FrameData;
					if (bGotData)
					{
						const float SampleRate = AnimationDataList.AnimData.IsValid() ? AnimationDataList.AnimData->SampleRate : 30.f;
						const float NumFrames  = Range.EndFrame - Range.StartFrame + 1.f;
						float& CurrentFrame    = AnimationDataList.CurrentFrame;

						if (CurrentFrame < Range.StartFrame || CurrentFrame > Range.EndFrame)
						{
							CurrentFrame = Range.StartFrame;
						}
						else if (NumFrames > 0.f)
						{
							CurrentFrame = Range.StartFrame + FMath::Fmod(CurrentFrame - Range.StartFrame + DeltaTime * AnimationDataList.PlayRate * SampleRate, NumFrames);
						}

						FrameData.Frame     = CurrentFrame;
						FrameData.PrevFrame = FMath::Clamp(CurrentFrame - 1.f, Range.StartFrame, Range.EndFrame);
					}

					{
	
						ISMInfo[ISMInfoIndex].AddBatchedCustomData<FAnimToTextureFrameData>(
							FrameData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					}

#if ENABLE_VISUAL_LOG
					UE_VLOG_LOCATION(this, LogTemp, Log,
						TransformFragment.GetTransform().GetLocation() + FVector(0.f, 0.f, 150.f), 20.f, FColor::Yellow,
						TEXT("idx=%d Frame=%.1f SF=%.0f EF=%.0f PR=%.2f bGotData=%d"),
						AnimationDataList.StateIndex, FrameData.Frame, Range.StartFrame, Range.EndFrame, AnimationDataList.PlayRate, bGotData ? 1 : 0);
#endif
				}
			}
			Representation.PrevTransform = TransformFragment.GetTransform();
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}

