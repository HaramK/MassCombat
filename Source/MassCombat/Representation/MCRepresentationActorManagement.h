#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationActorManagement.h"
#include "MCRepresentationActorManagement.generated.h"

UCLASS()
class MASSCOMBAT_API UMCRepresentationActorManagement : public UMassRepresentationActorManagement
{
	GENERATED_BODY()

public:
	virtual AActor* GetOrSpawnActor(UMassRepresentationSubsystem& RepresentationSubsystem, FMassEntityManager& EntityManager
		, const FMassEntityHandle MassAgent, const FTransform& Transform, const int16 TemplateActorIndex
		, FMassActorSpawnRequestHandle& SpawnRequestHandle, const float Priority) const override;

	virtual void SetActorEnabled(const EMassActorEnabledType EnabledType, AActor& Actor, const int32 EntityIdx, FMassCommandBuffer& CommandBuffer) const override;

	virtual void TeleportActor(const FTransform& Transform, AActor& Actor, FMassCommandBuffer& CommandBuffer) const override;
};
