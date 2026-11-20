// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/TenkaichiInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "InventoryFragment_EquippableItem.generated.h"

// 前置声明：装备定义（P09 才实现，这里只需类型名）
class ULyraEquipmentDefinition;
// 前置声明：UObject（引擎基类）
class UObject;

// 可装备碎片：声明"这件物品可以装备，装备后对应的装备定义是 X"
UCLASS()
class UInventoryFragment_EquippableItem : public UTenkaichiInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 装备定义类引用：蓝图里选一个"装备定义"类（P09 教）
	UPROPERTY(EditAnywhere, Category=Lyra)
	TSubclassOf<ULyraEquipmentDefinition> EquipmentDefinition;
};