// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiAbilityCost_PlayerTagStack.h"

#include "GameFramework/Controller.h"
#include "TenkaichiGameplayAbility.h"
#include "Player/TenkaichiPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAbilityCost_PlayerTagStack)

UTenkaichiAbilityCost_PlayerTagStack::UTenkaichiAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool UTenkaichiAbilityCost_PlayerTagStack::CheckCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (ATenkaichiPlayerState* PS = Cast<ATenkaichiPlayerState>(PC->PlayerState))
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return PS->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void UTenkaichiAbilityCost_PlayerTagStack::ApplyCost(const UTenkaichiGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (ATenkaichiPlayerState* PS = Cast<ATenkaichiPlayerState>(PC->PlayerState))
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				PS->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}