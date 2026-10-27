// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"

#include "TenkaichiGameplayAbilityTargetData_SingleTargetHit.generated.h"

class FArchive;
struct FGameplayEffectContextHandle;


// 对 SingleTargetHit 追踪的游戏特定扩展
USTRUCT()
struct FTenkaichiGameplayAbilityTargetData_SingleTargetHit : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	// 构造函数：把弹夹 ID 初始化为 -1（表示未设置）
	FTenkaichiGameplayAbilityTargetData_SingleTargetHit()
		: CartridgeID(-1)
	{ }

	// 把本结构体的命中数据写入效果上下文（含游戏特定的弹夹 ID）
	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	// 用于识别同一弹夹里多颗子弹的 ID
	UPROPERTY()
	int32 CartridgeID;

	// 网络序列化：序列化基类后，再序列化弹夹 ID
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	// 返回本结构体的反射类型，供 GAS 识别
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FTenkaichiGameplayAbilityTargetData_SingleTargetHit::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FTenkaichiGameplayAbilityTargetData_SingleTargetHit> : public TStructOpsTypeTraitsBase2<FTenkaichiGameplayAbilityTargetData_SingleTargetHit>
{
	enum
	{
		WithNetSerializer = true	// 目前这是必需的，否则 FGameplayAbilityTargetDataHandle 的网络序列化无法工作
	};
};