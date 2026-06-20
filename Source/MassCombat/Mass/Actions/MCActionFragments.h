#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassExternalSubsystemTraits.h"
#include "GameplayTagContainer.h"
#include "MCActionFragments.generated.h"

class UMCActionDef;

USTRUCT()
struct FMCActionRuntime
{
	GENERATED_BODY()

	float CooldownEnd = 0.f;

	float ActiveUntil = 0.f;
};

USTRUCT()
struct FMCActionFragment : public FMassFragment
{
	GENERATED_BODY()

	TArray<FMCActionRuntime> Runtimes;

	FGameplayTagContainer ActiveTags;
};

template<>
struct TMassFragmentTraits<FMCActionFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
};

USTRUCT()
struct FMCActionSetParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Action")
	TArray<TObjectPtr<UMCActionDef>> Actions;
};
