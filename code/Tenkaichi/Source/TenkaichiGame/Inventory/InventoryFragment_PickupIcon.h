// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/TenkaichiInventoryItemDefinition.h"
#include "UObject/ObjectPtr.h"

#include "InventoryFragment_PickupIcon.generated.h"

// 前置声明：UObject（引擎基类）
class UObject;
// 前置声明：骨骼网格体（引擎类，掉落的 3D 模型）
class USkeletalMesh;

// 掉落外观碎片：声明"这件物品掉在地上时的外观"（模型、名字、垫子颜色）
UCLASS()
class UInventoryFragment_PickupIcon : public UTenkaichiInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 构造函数：把垫子默认颜色设为绿色（实现在 .cpp）
	UInventoryFragment_PickupIcon();

	// 掉在地上的 3D 模型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	// 掉落物显示名（如 "AK-47"）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;

	// 掉落物脚下垫子的颜色
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FLinearColor PadColor;
};