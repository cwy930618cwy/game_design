// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAttributeSet.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAttributeSet)

class UWorld;


// 构造函数（基类无需初始化成员）
UTenkaichiAttributeSet::UTenkaichiAttributeSet()
{
}

// 拿到这个属性集所属的世界
UWorld* UTenkaichiAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

// 拿到这个属性集所属的项目的 AbilitySystemComponent
UTenkaichiAbilitySystemComponent* UTenkaichiAttributeSet::GetTenkaichiAbilitySystemComponent() const
{
	return Cast<UTenkaichiAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}