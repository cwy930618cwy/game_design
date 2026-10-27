// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TenkaichiAbilityCost.h"
#include "ScalableFloat.h"
#include "Templates/SubclassOf.h"

#include "TenkaichiAbilityCost_InventoryItem.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;

class UTenkaichiGameplayAbility;
class UTenkaichiInventoryItemDefinition;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;

/** 代表一个"需要消耗一定数量背包物品"的代价 */
UCLASS(meta=(DisplayName="Inventory Item"))
class UTenkaichiAbilityCost_InventoryItem : public UTenkaichiAbilityCost
{
	GENERATED_BODY()

public:
	UTenkaichiAbilityCost_InventoryItem();

	//~UTenkaichiAbilityCost 接口开始
	virtual bool CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~UTenkaichiAbilityCost 接口结束

protected:
	// 要消耗的物品数量（按技能等级缩放）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	FScalableFloat Quantity;

	// 要消耗哪个物品
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AbilityCost)
	TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDefinition;
};