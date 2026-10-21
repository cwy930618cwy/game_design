// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 主数据资产基类：能被 AssetManager 管理
#include "Engine/DataAsset.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiGameData.generated.h"

// 用模块导出宏 UE_API 代替本文件内的 UE_API（配合下方类/方法的前缀）
#define UE_API TENKAICHIGAME_API

// 玩法效果类的前置声明（软类引用只需声明，不展开头文件）
class UGameplayEffect;
class UObject;

/**
 * UTenkaichiGameData
 *
 *	不可变数据资产：存放全局游戏数据。
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Tenkaichi Game Data", ShortTooltip = "Data asset containing global game data."))
class UTenkaichiGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiGameData();

	// 返回已加载的游戏数据。
	static UE_API const UTenkaichiGameData& Get();

public:

	// 用来施加伤害的玩法效果。用 SetByCaller 提供伤害数值。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// 用来施加治疗的玩法效果。用 SetByCaller 提供治疗数值。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// 用来增删动态标签的玩法效果。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};

#undef UE_API