#include "Mass/Representation/MCNpcRepresentationActorManagement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationTypes.h"

AActor* UMCNpcRepresentationActorManagement::GetOrSpawnActor(UMassRepresentationSubsystem& RepresentationSubsystem
	, FMassEntityManager& EntityManager, const FMassEntityHandle MassAgent, const FTransform& Transform
	, const int16 TemplateActorIndex, FMassActorSpawnRequestHandle& SpawnRequestHandle, const float Priority) const
{
	FTransform RootTransform = Transform;

	// 엔티티 트랜스폼은 발 위치 기준 → 캡슐 중심까지 들어 올려 스폰
	if (const AActor* DefaultActor = RepresentationSubsystem.GetTemplateActorClass(TemplateActorIndex).GetDefaultObject())
	{
		if (const UCapsuleComponent* CapsuleComp = DefaultActor->FindComponentByClass<UCapsuleComponent>())
		{
			RootTransform.AddToTranslation(FVector(0.0f, 0.0f, CapsuleComp->GetScaledCapsuleHalfHeight()));
		}
	}

	return Super::GetOrSpawnActor(RepresentationSubsystem, EntityManager, MassAgent, RootTransform, TemplateActorIndex, SpawnRequestHandle, Priority);
}

void UMCNpcRepresentationActorManagement::TeleportActor(const FTransform& Transform, AActor& Actor, FMassCommandBuffer& CommandBuffer) const
{
	FTransform RootTransform = Transform;

	if (const UCapsuleComponent* CapsuleComp = Actor.FindComponentByClass<UCapsuleComponent>())
	{
		RootTransform.AddToTranslation(FVector(0.0f, 0.0f, CapsuleComp->GetScaledCapsuleHalfHeight()));
	}

	Super::TeleportActor(RootTransform, Actor, CommandBuffer);
}

void UMCNpcRepresentationActorManagement::SetActorEnabled(const EMassActorEnabledType EnabledType, AActor& Actor, const int32 EntityIdx, FMassCommandBuffer& CommandBuffer) const
{
	Super::SetActorEnabled(EnabledType, Actor, EntityIdx, CommandBuffer);

	const bool bEnabled = EnabledType != EMassActorEnabledType::Disabled;

	// 비활성 액터의 스켈레탈 메시 가시성/틱을 꺼서 ISM로 표현되는 동안 비용을 없앤다
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
