// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemGlobals.h"

#include "TenkaichiAbilitySystemGlobals.generated.h"

class UObject;
struct FGameplayEffectContext;

/** GAS 的全局能力系统单例：定制项目级 GAS 运行时行为 */
UCLASS(Config=Game)
class UTenkaichiAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_UCLASS_BODY()

	//~UAbilitySystemGlobals 接口开始
	// 重写：让 GAS 分配「游戏效果上下文」时，分配我们自己的 FTenkaichiGameplayEffectContext 子类
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	//~UAbilitySystemGlobals 接口结束
};