// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/TenkaichiAbilityTagRelationshipMapping.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilityTagRelationshipMapping)

// 给一组技能标签，解析出「要阻断/取消的标签」
void UTenkaichiAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
	// 暂时用简单的遍历
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FTenkaichiAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
			}
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
			}
		}
	}
}

// 给一组技能标签，解析出「激活时额外要求/禁止的标签」
void UTenkaichiAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
	// 暂时用简单的遍历
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FTenkaichiAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
			}
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
			}
		}
	}
}

// 判断「这组技能标签」是否被「传入的动作标签」取消
bool UTenkaichiAbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
	// 暂时用简单的遍历
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FTenkaichiAbilityTagRelationship& Tags = AbilityTagRelationships[i];

		if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}

	return false;
}