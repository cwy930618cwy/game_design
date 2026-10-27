// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "TenkaichiAttributeSet.h"

#include "TenkaichiCombatSet.generated.h"

class UObject;
struct FFrame;


/**
 * UTenkaichiCombatSet
 *
 *	定义应用伤害或治疗所必需的属性的类。
 *	属性示例包括：伤害、治疗、攻击力和护盾穿透。
 */
UCLASS(BlueprintType)
class UTenkaichiCombatSet : public UTenkaichiAttributeSet
{
	GENERATED_BODY()

public:

	// 构造函数
	UTenkaichiCombatSet();

	// 为 BaseDamage 生成访问函数（GetBaseDamage / SetBaseDamage / InitBaseDamage / GetBaseDamageAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiCombatSet, BaseDamage);

	// 为 BaseHeal 生成访问函数（GetBaseHeal / SetBaseHeal / InitBaseHeal / GetBaseHealAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiCombatSet, BaseHeal);

protected:

	// BaseDamage 属性复制回来时的回调（服务器→客户端同步后触发）
	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	// BaseHeal 属性复制回来时的回调（服务器→客户端同步后触发）
	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

	// 在伤害执行中要应用的基础伤害量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Tenkaichi|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// 在治疗执行中要应用的基础治疗量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "Tenkaichi|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};