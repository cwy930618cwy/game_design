// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "TenkaichiAbilityCost.generated.h"

class UTenkaichiGameplayAbility;

/** 技能的代价抽象基类（比如弹药、充能次数） */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UTenkaichiAbilityCost : public UObject
{
	GENERATED_BODY()

public:
	UTenkaichiAbilityCost()
	{
	}

	// 检查这个代价我们是否付得起。失败原因标签可加入 OptionalRelevantTags（若非空），
	// 其他地方可据此给出用户反馈（例如武器没弹药时播放咔哒声）。
	// 进入时 Ability 和 ActorInfo 保证非空，但 OptionalRelevantTags 可能是空指针。
	// @return true 表示付得起，false 表示付不起。
	virtual bool CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	// 把技能的代价施加到目标身上。
	// 注意：你的实现不需要检查 ShouldOnlyApplyCostOnHit()，调用方会替你检查。
	// 进入时 Ability 和 ActorInfo 保证非空。
	virtual void ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}

	// 若为 true，这个代价只在技能命中成功时才施加
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	// 若为 true，这个代价只在技能命中成功时才施加
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	bool bOnlyApplyCostOnHit = false;
};