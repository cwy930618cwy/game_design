// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/TenkaichiInventoryItemDefinition.h"
#include "Styling/SlateBrush.h"

#include "InventoryFragment_QuickBarIcon.generated.h"

// 前置声明：UObject（引擎基类）
class UObject;

// 快捷栏图标碎片：声明"这件物品在快捷栏里的样子"（主图标、弹药角标、装备名）
UCLASS()
class UInventoryFragment_QuickBarIcon : public UTenkaichiInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 快捷栏主图标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;

	// 弹药/数量角标图标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush AmmoBrush;

	// 装备时显示的物品名
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayNameWhenEquipped;
};