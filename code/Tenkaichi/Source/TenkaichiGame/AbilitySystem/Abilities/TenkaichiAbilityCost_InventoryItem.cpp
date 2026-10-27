// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAbilityCost_InventoryItem.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilityCost_InventoryItem)

UTenkaichiAbilityCost_InventoryItem::UTenkaichiAbilityCost_InventoryItem()
{
	Quantity.SetValue(1.0f);
}

bool UTenkaichiAbilityCost_InventoryItem::CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
#if 0
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (UTenkaichiInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<UTenkaichiInventoryManagerComponent>())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

			return InventoryComponent->GetTotalItemCountByDefinition(ItemDefinition) >= NumItemsToConsume;
		}
	}
#endif
	return false;
}

void UTenkaichiAbilityCost_InventoryItem::ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
#if 0
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (UTenkaichiInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<UTenkaichiInventoryManagerComponent>())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumItemsToConsumeReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

				InventoryComponent->ConsumeItemsByDefinition(ItemDefinition, NumItemsToConsume);
			}
		}
	}
#endif
}