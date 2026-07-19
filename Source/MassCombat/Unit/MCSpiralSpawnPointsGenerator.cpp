#include "Unit/MCSpiralSpawnPointsGenerator.h"
#include "MassSpawnerTypes.h"
#include "MassSpawnLocationProcessor.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"

void UMCSpiralSpawnPointsGenerator::Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const
{
	TArray<FMassEntitySpawnDataGeneratorResult> Results;
	if (Count <= 0)
	{
		FinishedGeneratingSpawnPointsDelegate.Execute(Results);
		return;
	}

	const AActor* OwnerActor = Cast<AActor>(&QueryOwner);
	const FVector Center = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
	UNavigationSystemV1* NavSys = bProjectToNavmesh ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(QueryOwner.GetWorld()) : nullptr;

	BuildResultsFromEntityTypes(Count, EntityTypes, Results);

	const float GoldenAngle = UE_PI * (3.f - FMath::Sqrt(5.f));
	int32 PointIndex = 0;

	for (FMassEntitySpawnDataGeneratorResult& Result : Results)
	{
		Result.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
		Result.SpawnData.InitializeAs<FMassTransformsSpawnData>();
		FMassTransformsSpawnData& Transforms = Result.SpawnData.GetMutable<FMassTransformsSpawnData>();
		Transforms.Transforms.Reserve(Result.NumEntities);

		for (int32 i = 0; i < Result.NumEntities; ++i, ++PointIndex)
		{
			const float Radius = Spacing * FMath::Sqrt(PointIndex + 0.5f);
			const float Angle = PointIndex * GoldenAngle;
			FVector Point = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

			if (NavSys)
			{
				FNavLocation Projected;
				if (NavSys->ProjectPointToNavigation(Point, Projected, FVector(Spacing, Spacing, 2000.f)))
				{
					Point = Projected.Location;
				}
			}

			Transforms.Transforms.AddDefaulted_GetRef().SetLocation(Point);
		}
	}

	FinishedGeneratingSpawnPointsDelegate.Execute(Results);
}
