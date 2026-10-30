// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Inventory/TenkaichiInventoryItemDefinition.h"

#include "InventoryFragment_SetStats.generated.h"

// 前置声明：物品实例（P08.2 已教，这里只需类名）
class UTenkaichiInventoryItemInstance;
// 前置声明：UObject（引擎基类）
class UObject;
// 前置声明：FGameplayTag（引擎结构体）
struct FGameplayTag;

// 属性碎片：给物品定义添加"初始属性表"，实例出生时把初始数据灌进去
UCLASS()
class UInventoryFragment_SetStats : public UTenkaichiInventoryItemFragment
{
	GENERATED_BODY()

protected:
	// 初始属性表：Tag → 初始数量（弹夹、耐久等），仅编辑器默认值可改
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TMap<FGameplayTag, int32> InitialItemStats;

public:
	// 覆写钩子：物品实例创建时调用，把 InitialItemStats 灌进实例
	virtual void OnInstanceCreated(UTenkaichiInventoryItemInstance* Instance) const override;

	// 按 Tag 查初始属性表里的值（查不到返回 0）
	int32 GetItemStatByTag(FGameplayTag Tag) const;
};