#include "Representation/MCAnimStateProcessor.h"
#include "Representation/MCRepresentationFragments.h"
#include "Representation/MCAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "MassActorSubsystem.h"
#include "VisualLogger/VisualLogger.h"

UMCAnimStateProcessor::UMCAnimStateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCTargetingProcessor"));
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UMCAnimStateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCAnimStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FMCAnimParams>();
}

void UMCAnimStateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float GlobalTime = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [this, GlobalTime](FMassExecutionContext& Ctx)
	{
		const FMCAnimParams& Params = Ctx.GetConstSharedFragment<FMCAnimParams>();
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Ctx.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMCAnimStateFragment> Anims = Ctx.GetMutableFragmentView<FMCAnimStateFragment>();
		const TConstArrayView<FMassRepresentationFragment> Reps = Ctx.GetFragmentView<FMassRepresentationFragment>();
		const TArrayView<FMassActorFragment> Actors = Ctx.GetMutableFragmentView<FMassActorFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCAnimStateFragment& Anim = Anims[i];
			const FMassRepresentationFragment& Rep = Reps[i];

			if (!Anim.AnimData.IsValid() && Params.DefaultAnimData)
			{
				Anim.AnimData = Params.DefaultAnimData.Get();
			}

			const bool bWasActor = Rep.PrevRepresentation == EMassRepresentationType::HighResSpawnedActor
				|| Rep.PrevRepresentation == EMassRepresentationType::LowResSpawnedActor;
			const bool bIsActor = Rep.CurrentRepresentation == EMassRepresentationType::HighResSpawnedActor
				|| Rep.CurrentRepresentation == EMassRepresentationType::LowResSpawnedActor;
			Anim.bSwappedThisFrame = (bWasActor != bIsActor);

			const int32 PrevState = Anim.StateIndex;

			const bool bMontageActive = Anim.ActiveMontage.IsValid();
			if (bMontageActive)
			{
				Anim.StateIndex = Anim.ActiveMontageStateIndex;
				Anim.PlayRate = 1.f;
			}
			else if (Velocities[i].Value.SizeSquared() > Params.WalkSpeedThresholdSq)
			{
				Anim.StateIndex = Params.WalkStateIndex;
				const float PrevPlayRate = Anim.PlayRate;
				Anim.PlayRate = FMath::Clamp(Velocities[i].Value.Size() / Params.WalkAnimReferenceSpeed, Params.MinWalkPlayRate, Params.MaxWalkPlayRate);

				if (Anim.StateIndex == PrevState && Anim.PlayRate > 0.f)
				{
					// Rebase start time so the current animation frame is preserved when the play rate changes mid-state.
					Anim.GlobalStartTime = GlobalTime - PrevPlayRate * (GlobalTime - Anim.GlobalStartTime) / Anim.PlayRate;
				}
			}
			else
			{
				Anim.StateIndex = Params.IdleStateIndex;
				Anim.PlayRate = 1.f;
			}

#if ENABLE_VISUAL_LOG
			UE_VLOG_LOCATION(this, LogTemp, Log,
				Transforms[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 120.f), 20.f, FColor::White,
				TEXT("v=%.0f idx=%d"), Velocities[i].Value.Size(), Anim.StateIndex);
#endif

			const bool bStateChanged = Anim.StateIndex != PrevState;
			if (bStateChanged)
			{
				Anim.GlobalStartTime = GlobalTime;
			}

			if (bIsActor && Actors.Num() > 0)
			{
				if (ACharacter* Character = Cast<ACharacter>(Actors[i].GetMutable()))
				{
					if (UMCAnimInstance* AnimInstance = Cast<UMCAnimInstance>(Character->GetMesh()->GetAnimInstance()))
					{
						const FVector CurrentVelocity = Velocities[i].Value;

						AnimInstance->Velocity = CurrentVelocity;
						AnimInstance->GroundSpeed = CurrentVelocity.Size2D();
						AnimInstance->ActorRotation = Transforms[i].GetTransform().Rotator();

						const float StartPosition = (GlobalTime - Anim.GlobalStartTime) * Anim.PlayRate;

						AnimInstance->bSwappedThisFrame = Anim.bSwappedThisFrame;
						AnimInstance->LocomotionStartPosition = StartPosition;

						if (bMontageActive
							&& Rep.CurrentRepresentation == EMassRepresentationType::HighResSpawnedActor
							&& (bStateChanged || Anim.bSwappedThisFrame))
						{
							AnimInstance->PlayMontageSynced(Anim.ActiveMontage.Get(), StartPosition, Anim.bSwappedThisFrame);
						}
					}
				}
			}
		}
	});
}
