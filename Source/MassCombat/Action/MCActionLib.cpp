#include "Action/MCActionLib.h"
#include "Action/MCActionFragments.h"
#include "Action/MCActionDef.h"

void FMCActionLib::EnsureRuntimeSize(FMCActionFragment& Frag, const FMCActionSetParams& Set)
{
	if (Frag.Runtimes.Num() != Set.Actions.Num())
	{
		Frag.Runtimes.SetNumZeroed(Set.Actions.Num());
	}
}

int32 FMCActionLib::FindActionIndex(const FMCActionSetParams& Set, FGameplayTag ActionTag)
{
	for (int32 i = 0; i < Set.Actions.Num(); ++i)
	{
		const UMCActionDef* Def = Set.Actions[i];
		if (Def && Def->ActivationTag == ActionTag)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool FMCActionLib::IsRunning(const FMCActionFragment& Frag, int32 Index, float Now)
{
	return Frag.Runtimes.IsValidIndex(Index) && Frag.Runtimes[Index].ActiveUntil > Now;
}

bool FMCActionLib::CanStart(const FMCActionFragment& Frag, const FMCActionSetParams& Set, int32 Index, float Now)
{
	if (!Set.Actions.IsValidIndex(Index) || !Frag.Runtimes.IsValidIndex(Index))
	{
		return false;
	}

	const FMCActionRuntime& RT = Frag.Runtimes[Index];
	if (RT.ActiveUntil > Now) 
	{
		return false;
	}
	if (RT.CooldownEnd > Now)
	{
		return false;
	}

	const UMCActionDef* Def = Set.Actions[Index];
	if (Def && Frag.ActiveTags.HasAny(Def->BlockedTags))
	{
		return false;
	}

	return true;
}

bool FMCActionLib::TryStart(FMCActionFragment& Frag, const FMCActionSetParams& Set, FGameplayTag ActionTag, float Now)
{
	EnsureRuntimeSize(Frag, Set);

	const int32 Index = FindActionIndex(Set, ActionTag);
	if (Index == INDEX_NONE || !CanStart(Frag, Set, Index, Now))
	{
		return false;
	}

	const UMCActionDef* Def = Set.Actions[Index];
	// Infinite duration: the action stays active until explicitly stopped (by the StateTree task on exit).
	Frag.Runtimes[Index].ActiveUntil = TNumericLimits<float>::Max();
	Frag.ActiveTags.AppendTags(Def->GrantsTags);

	return true;
}

void FMCActionLib::Stop(FMCActionFragment& Frag, const FMCActionSetParams& Set, int32 Index, float Now)
{
	if (!Set.Actions.IsValidIndex(Index) || !Frag.Runtimes.IsValidIndex(Index))
	{
		return;
	}

	FMCActionRuntime& RT = Frag.Runtimes[Index];
	if (RT.ActiveUntil <= 0.f)
	{
		return;
	}

	if (const UMCActionDef* Def = Set.Actions[Index])
	{
		if (Def->CooldownTime > 0.f)
		{
			RT.CooldownEnd = Now + Def->CooldownTime;
		}
	}

	RT.ActiveUntil = 0.f;

	RebuildActiveTags(Frag, Set);
}

void FMCActionLib::RebuildActiveTags(FMCActionFragment& Frag, const FMCActionSetParams& Set)
{
	Frag.ActiveTags.Reset();
	for (int32 i = 0; i < Set.Actions.Num(); ++i)
	{
		const UMCActionDef* Def = Set.Actions[i];
		if (Def && Frag.Runtimes.IsValidIndex(i) && Frag.Runtimes[i].ActiveUntil > 0.f)
		{
			Frag.ActiveTags.AppendTags(Def->GrantsTags);
		}
	}
}
