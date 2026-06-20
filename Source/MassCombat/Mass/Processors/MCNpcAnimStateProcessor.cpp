#include "Mass/Processors/MCNpcAnimStateProcessor.h"
#include "Mass/Fragments/MCNpcCombatFragments.h"
#include "MCNpcAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationTypes.h"
#include "MassActorSubsystem.h"
#include "DrawDebugHelpers.h" // TODO(debug): 임시 — 확인 후 제거

UMCNpcAnimStateProcessor::UMCNpcAnimStateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone | EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Server);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MCNpcTargetingProcessor"));
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UMCNpcAnimStateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCNpcAnimStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FMCNpcCombatParams>();
}

void UMCNpcAnimStateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	const float GlobalTime = World ? World->GetTimeSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [GlobalTime, World](FMassExecutionContext& Ctx)
	{
		const FMCNpcCombatParams& Params = Ctx.GetConstSharedFragment<FMCNpcCombatParams>();
		const TConstArrayView<FTransformFragment> Transforms = Ctx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Ctx.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMCNpcAnimStateFragment> Anims = Ctx.GetMutableFragmentView<FMCNpcAnimStateFragment>();
		const TConstArrayView<FMassRepresentationFragment> Reps = Ctx.GetFragmentView<FMassRepresentationFragment>();
		const TArrayView<FMassActorFragment> Actors = Ctx.GetMutableFragmentView<FMassActorFragment>();

		const int32 Num = Ctx.GetNumEntities();
		for (int32 i = 0; i < Num; ++i)
		{
			FMCNpcAnimStateFragment& Anim = Anims[i];
			const FMassRepresentationFragment& Rep = Reps[i];

			if (!Anim.AnimData.IsValid() && !Params.DefaultAnimData.IsNull())
			{
				Anim.AnimData = Params.DefaultAnimData.LoadSynchronous();
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
					Anim.GlobalStartTime = GlobalTime - PrevPlayRate * (GlobalTime - Anim.GlobalStartTime) / Anim.PlayRate;
				}
			}
			else
			{
				Anim.StateIndex = Params.IdleStateIndex;
				Anim.PlayRate = 1.f;
			}

			// TODO(debug): temporary - verify walk triggers while moving. Remove after checking.
			if (World)
			{
				DrawDebugString(World, Transforms[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 120.f),
					FString::Printf(TEXT("v=%.0f idx=%d"), Velocities[i].Value.Size(), Anim.StateIndex),
					nullptr, FColor::White, 0.f);
			}

			const bool bStateChanged = Anim.StateIndex != PrevState;
			if (bStateChanged)
			{
				Anim.GlobalStartTime = GlobalTime;
			}

			if (bIsActor && Actors.Num() > 0)
			{
				if (ACharacter* Character = Cast<ACharacter>(Actors[i].GetMutable()))
				{
					if (UMCNpcAnimInstance* AnimInstance = Cast<UMCNpcAnimInstance>(Character->GetMesh()->GetAnimInstance()))
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
