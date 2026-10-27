// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAbilitySystemGlobals.h"

#include "TenkaichiGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilitySystemGlobals)

struct FGameplayEffectContext;

// 构造函数：全局能力系统单例的默认构造（无特殊初始化）
UTenkaichiAbilitySystemGlobals::UTenkaichiAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 重写：让 GAS 分配「游戏效果上下文」时，分配我们自定义的 FTenkaichiGameplayEffectContext
FGameplayEffectContext* UTenkaichiAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FTenkaichiGameplayEffectContext();
}