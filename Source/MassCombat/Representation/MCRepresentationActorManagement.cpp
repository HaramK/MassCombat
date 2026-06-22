#include "Representation/MCRepresentationActorManagement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationTypes.h"

AActor* UMCRepresentationActorManagement::GetOrSpawnActor(UMassRepresentationSubsystem& RepresentationSubsystem
	, FMassEntityManager& EntityManager, const FMassEntityHandle MassAgent, const FTransform& Transform
	, const int16 TemplateActorIndex, FMassActorSpawnRequestHandle& SpawnRequestHandle, const float Priority) const
{
	FTransform RootTransform = Transform;

	// Entity transform is at foot level; lift it to the capsule center for spawning.
	if (const AActor* DefaultActor = RepresentationSubsystem.GetTemplateActorClass(TemplateActorIndex).GetDefaultObject())
	{
		if (const UCapsuleComponent* CapsuleComp = DefaultActor->FindComponentByClass<UCapsuleComponent>())
		{
			RootTransform.AddToTranslation(FVector(0.0f, 0.0f, CapsuleComp->GetScaledCapsuleHalfHeight()));
		}
	}

	return Super::GetOrSpawnActor(RepresentationSubsystem, EntityManager, MassAgent, RootTransform, TemplateActorIndex, SpawnRequestHandle, Priority);
}

void UMCRepresentationActorManagement::TeleportActor(const FTransform& Transform, AActor& Actor, FMassCommandBuffer& CommandBuffer) const
{
	FTransform RootTransform = Transform;

	if (const UCapsuleComponent* CapsuleComp = Actor.FindComponentByClass<UCapsuleComponent>())
	{
		RootTransform.AddToTranslation(FVector(0.0f, 0.0f, CapsuleComp->GetScaledCapsuleHalfHeight()));
	}

	Super::TeleportActor(RootTransform, Actor, CommandBuffer);
}

void UMCRepresentationActorManagement::SetActorEnabled(const EMassActorEnabledType EnabledType, AActor& Actor, const int32 EntityIdx, FMassCommandBuffer& CommandBuffer) const
{
	Super::SetActorEnabled(EnabledType, Actor, EntityIdx, CommandBuffer);

	const bool bEnabled = EnabledType != EMassActorEnabledType::Disabled;

	// Disable the skeletal mesh visibility/tick on inactive actors to avoid cost while represented as ISM.
	if (USkeletalMeshComponent* SkeletalMeshComponent = Actor.FindComponentByClass<USkeletalMeshComponent>())
	{
		SkeletalMeshComponent->SetVisibility(bEnabled);
		SkeletalMeshComponent->SetComponentTickEnabled(bEnabled);
	}

	if (const ACharacter* Character = Cast<ACharacter>(&Actor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->SetComponentTickEnabled(bEnabled);
		}
	}
}
