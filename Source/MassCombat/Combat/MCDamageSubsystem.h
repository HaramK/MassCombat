#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MCDamageSubsystem.generated.h"

struct FMCDamageEvent
{
	FMassEntityHandle Target;
	FMassEntityHandle Instigator;
	float Amount = 0.f;
};

UCLASS()
class MASSCOMBAT_API UMCDamageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void QueueDamage(FMassEntityHandle Target, FMassEntityHandle Instigator, float Amount);

	void ConsumeEvents(TArray<FMCDamageEvent>& OutEvents);

private:
	TArray<FMCDamageEvent> PendingEvents;
};
