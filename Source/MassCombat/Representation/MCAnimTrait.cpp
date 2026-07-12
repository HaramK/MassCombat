#include "Representation/MCAnimTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityUtils.h"
#include "Animation/AnimSequence.h"

namespace
{
	int32 ResolveAnimIndex(const UAnimToTextureDataAsset* AnimData, const UAnimSequence* Sequence, int32 Fallback)
	{
		if (!AnimData || !Sequence)
		{
			return Fallback;
		}

		int32 BakedIndex = 0;
		for (const FAnimToTextureAnimSequenceInfo& Info : AnimData->AnimSequences)
		{
			if (!Info.bEnabled)
			{
				continue;
			}
			if (Info.AnimSequence == Sequence)
			{
				return BakedIndex;
			}
			++BakedIndex;
		}

		UE_LOG(LogTemp, Warning, TEXT("UMCAnimTrait: '%s' is not baked into '%s'; falling back to index %d."),
			*Sequence->GetName(), *AnimData->GetName(), Fallback);
		return Fallback;
	}
}

void UMCAnimTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FMassVelocityFragment>();

	BuildContext.AddFragment<FMCAnimStateFragment>();

	FMCAnimParams Resolved = Params;
	Resolved.IdleStateIndex = ResolveAnimIndex(Params.DefaultAnimData, Params.IdleAnim, Resolved.IdleStateIndex);
	Resolved.WalkStateIndex = ResolveAnimIndex(Params.DefaultAnimData, Params.WalkAnim, Resolved.WalkStateIndex);

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(Resolved));
}
