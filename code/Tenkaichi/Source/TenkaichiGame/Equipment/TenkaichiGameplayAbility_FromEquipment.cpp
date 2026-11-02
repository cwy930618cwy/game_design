// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbility_FromEquipment.h"
#include "TenkaichiEquipmentInstance.h"
#include "Inventory/TenkaichiInventoryItemInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility_FromEquipment)

// 构造函数：只调用父类构造
UTenkaichiGameplayAbility_FromEquipment::UTenkaichiGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 取关联的装备实例：从技能描述符的 SourceObject 里拿
UTenkaichiEquipmentInstance* UTenkaichiGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UTenkaichiEquipmentInstance>(Spec->SourceObject.Get());
	}

	return nullptr;
}

// 取关联的物品实例：装备的 Instigator 就是物品
UTenkaichiInventoryItemInstance* UTenkaichiGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	if (UTenkaichiEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UTenkaichiInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}


#if WITH_EDITOR
// 编辑器数据校验：装备技能必须是实例化策略
EDataValidationResult UTenkaichiGameplayAbility_FromEquipment::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	{
		Context.AddError(NSLOCTEXT("Tenkaichi", "EquipmentAbilityMustBeInstanced", "装备技能必须是实例化策略"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif