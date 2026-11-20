// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/TenkaichiInventoryItemDefinition.h"

#include "InventoryFragment_ReticleConfig.generated.h"

class UTenkaichiReticleWidgetBase;
class UObject;

UCLASS()
class UInventoryFragment_ReticleConfig : public UTenkaichiInventoryItemFragment
{
	GENERATED_BODY()

public:
	// ⚠️ 注释挂起（依赖 P16 UI）：等 `UTenkaichiReticleWidgetBase` 实现后再放开
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
	//TArray<TSubclassOf<UTenkaichiReticleWidgetBase>> ReticleWidgets;
};