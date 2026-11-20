// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "PhysicalMaterialWithTags.generated.h"

class UObject;

/**
 * UPhysicalMaterialWithTags
 *
 * 一种能挂 GameplayTag 的物理材质：让物理材质能被游戏逻辑用标签识别
 */
UCLASS()
class UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 一个 GameplayTag 容器：游戏代码可以用它来判断这个物理材质是啥
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};