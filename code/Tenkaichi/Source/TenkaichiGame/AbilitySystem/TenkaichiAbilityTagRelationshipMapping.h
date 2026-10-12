// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "TenkaichiAbilityTagRelationshipMapping.generated.h"

class UObject;

/** 定义「不同技能标签之间的关系」的结构体 */
USTRUCT()
struct FTenkaichiAbilityTagRelationship
{
	GENERATED_BODY()

	/** 这条「容器关系」所关心的标签。单个标签，但一个技能可以有多个这样的关系 */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** 使用「这个标签」的技能，会阻断「这些标签」的技能 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** 使用「这个标签」的技能，会取消「这些标签」的技能 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** 如果某技能带有「这个标签」，这些标签会被隐式加到它的「激活要求标签」里 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** 如果某技能带有「这个标签」，这些标签会被隐式加到它的「激活禁止标签」里 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};


/** 描述「技能标签如何阻断或取消其他技能」的映射 */
UCLASS()
class UTenkaichiAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

private:
	/** 不同 gameplay 标签之间的关系列表（哪些阻断、哪些取消） */
	UPROPERTY(EditAnywhere, Category = Ability, meta=(TitleProperty="AbilityTag"))
	TArray<FTenkaichiAbilityTagRelationship> AbilityTagRelationships;

public:
	// 给一组技能标签，解析出「要阻断/取消的标签」
	void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	// 给一组技能标签，解析出「激活时额外要求/禁止的标签」
	void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	// 判断「这组技能标签」是否被「传入的动作标签」取消
	bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};