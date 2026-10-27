// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "TenkaichiAbilityCost.h"
#include "ScalableFloat.h"

#include "TenkaichiAbilityCost_PlayerTagStack.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;

class UTenkaichiGameplayAbility;
class UObject;
struct FGameplayAbilityActorInfo;

/** 代表一个"需要消耗玩家状态身上某个标签栈数量"的代价 */
UCLASS(meta=(DisplayName="Player Tag Stack"))
class UTenkaichiAbilityCost_PlayerTagStack : public UTenkaichiAbilityCost
{
	GENERATED_BODY()

public:
	UTenkaichiAbilityCost_PlayerTagStack();

	//~UTenkaichiAbilityCost 接口开始
	virtual bool CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~UTenkaichiAbilityCost 接口结束

protected:
	// 要消耗的标签数量（按技能等级缩放）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	// 要消耗哪个标签的一些数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;
};