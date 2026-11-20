// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"

#include "TenkaichiGameplayAbility_FromEquipment.generated.h"

class UTenkaichiEquipmentInstance;
class UTenkaichiInventoryItemInstance;

/**
 * UTenkaichiGameplayAbility_FromEquipment
 *
 * 由装备授予、并与某个装备实例关联的技能
 */
UCLASS()
class UTenkaichiGameplayAbility_FromEquipment : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:

	// 构造函数
	UTenkaichiGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取关联的装备实例（授予这个技能的装备）
	UFUNCTION(BlueprintCallable, Category="Tenkaichi|Ability")
	UTenkaichiEquipmentInstance* GetAssociatedEquipment() const;

	// 取关联的物品实例（装备对应的物品）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	UTenkaichiInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	// 编辑器数据校验：装备技能必须是实例化策略
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

};