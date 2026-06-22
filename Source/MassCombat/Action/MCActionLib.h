#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FMCActionFragment;
struct FMCActionSetParams;

struct FMCActionLib
{
	static void EnsureRuntimeSize(FMCActionFragment& Frag, const FMCActionSetParams& Set);

	static int32 FindActionIndex(const FMCActionSetParams& Set, FGameplayTag ActionTag);

	static bool IsRunning(const FMCActionFragment& Frag, int32 Index, float Now);

	static bool CanStart(const FMCActionFragment& Frag, const FMCActionSetParams& Set, int32 Index, float Now);

	static bool TryStart(FMCActionFragment& Frag, const FMCActionSetParams& Set, FGameplayTag ActionTag, float Now);

	static void Stop(FMCActionFragment& Frag, const FMCActionSetParams& Set, int32 Index, float Now);
};
