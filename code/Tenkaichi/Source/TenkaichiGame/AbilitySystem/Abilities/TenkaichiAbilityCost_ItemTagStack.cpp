// Copyright Epic Games, Inc. All Rights Reserved.

#if 0  // ⚠️【注释挂起】依赖 P09 Equipment（TenkaichiGameplayAbility_FromEquipment）+ P08 Inventory（TenkaichiInventoryItemInstance），工程均无此文件 → 编译不过。
       // P05.3-B GameplayAbility ✅ 已就位；等 P08/P09 就位后放开（见注释备忘录 ⑳）
#include "TenkaichiAbilityCost_ItemTagStack.h"

#include "Equipment/TenkaichiGameplayAbility_FromEquipment.h"
#include "Inventory/TenkaichiInventoryItemInstance.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilityCost_ItemTagStack)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UTenkaichiAbilityCost_ItemTagStack::UTenkaichiAbilityCost_ItemTagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UTenkaichiAbilityCost_ItemTagStack::CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UTenkaichiGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UTenkaichiGameplayAbility_FromEquipment>(Ability))
	{
		if (UTenkaichiInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
			const bool bCanApplyCost = ItemInstance->GetStatTagStackCount(Tag) >= NumStacks;

			// 当这个代价无法施加时，告知其他技能为什么
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}
			return bCanApplyCost;
		}
	}
	return false;
}

void UTenkaichiAbilityCost_ItemTagStack::ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UTenkaichiGameplayAbility_FromEquipment* EquipmentAbility = Cast<const UTenkaichiGameplayAbility_FromEquipment>(Ability))
		{
			if (UTenkaichiInventoryItemInstance* ItemInstance = EquipmentAbility->GetAssociatedItem())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				ItemInstance->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
#endif  // 依赖就位后放开