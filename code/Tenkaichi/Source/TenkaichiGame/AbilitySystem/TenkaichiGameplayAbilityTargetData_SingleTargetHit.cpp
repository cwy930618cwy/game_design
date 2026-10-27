// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbilityTargetData_SingleTargetHit.h"

#include "TenkaichiGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbilityTargetData_SingleTargetHit)

struct FGameplayEffectContextHandle;

//////////////////////////////////////////////////////////////////////

// 先把命中数据写入效果上下文（基类负责），再把游戏特定的弹夹 ID 写进去
void FTenkaichiGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// 添加游戏特定的数据
	if (FTenkaichiGameplayEffectContext* TypedContext = FTenkaichiGameplayEffectContext::ExtractEffectContext(Context))
	{
		TypedContext->CartridgeID = CartridgeID;
	}
}

// 网络序列化：先序列化基类，再序列化弹夹 ID
bool FTenkaichiGameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	Ar << CartridgeID;

	return true;
}