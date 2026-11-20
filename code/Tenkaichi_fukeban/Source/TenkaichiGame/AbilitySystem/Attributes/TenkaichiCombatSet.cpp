// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiCombatSet.h"

#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCombatSet)

class FLifetimeProperty;


// 构造函数：把两个战斗属性的初始值设为 0
UTenkaichiCombatSet::UTenkaichiCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

// 声明这两个属性的网络复制条件（仅所有者收到，且变化时总是通知）
void UTenkaichiCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

// BaseDamage 复制回来时的回调：通知 GAS 属性值变了
void UTenkaichiCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiCombatSet, BaseDamage, OldValue);
}

// BaseHeal 复制回来时的回调：通知 GAS 属性值变了
void UTenkaichiCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiCombatSet, BaseHeal, OldValue);
}