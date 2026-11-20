// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGlobalAbilitySystem.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGlobalAbilitySystem)

// 把全局技能授给指定的 ASC（若已授过，先移除旧的再授新的）
void FGlobalAppliedAbilityList::AddToASC(TSubclassOf<UGameplayAbility> Ability, UTenkaichiAbilitySystemComponent* ASC)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
	Handles.Add(ASC, AbilitySpecHandle);
}

// 从指定的 ASC 上移除这个全局技能
void FGlobalAppliedAbilityList::RemoveFromASC(UTenkaichiAbilitySystemComponent* ASC)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}

// 从所有 ASC 上移除这个全局技能
void FGlobalAppliedAbilityList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->ClearAbility(KVP.Value);
		}
	}
	Handles.Empty();
}



// 把全局效果加给指定的 ASC（若已加过，先移除旧的再加新的）
void FGlobalAppliedEffectList::AddToASC(TSubclassOf<UGameplayEffect> Effect, UTenkaichiAbilitySystemComponent* ASC)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, /*Level=*/ 1, ASC->MakeEffectContext());
	Handles.Add(ASC, GameplayEffectHandle);
}

// 从指定的 ASC 上移除这个全局效果
void FGlobalAppliedEffectList::RemoveFromASC(UTenkaichiAbilitySystemComponent* ASC)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(ASC);
	}
}

// 从所有 ASC 上移除这个全局效果
void FGlobalAppliedEffectList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
		}
	}
	Handles.Empty();
}

// 构造函数：全局技能系统的默认构造（无特殊初始化）
UTenkaichiGlobalAbilitySystem::UTenkaichiGlobalAbilitySystem()
{
}

// 把技能授给所有已注册的 ASC（仅服务器）
void UTenkaichiGlobalAbilitySystem::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (!AppliedAbilities.Contains(Ability)))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities.Add(Ability);		
		for (ULyraAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Ability, ASC);
		}
	}
}

// 把效果加给所有已注册的 ASC（仅服务器）
void UTenkaichiGlobalAbilitySystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && (!AppliedEffects.Contains(Effect)))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects.Add(Effect);
		for (ULyraAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Effect, ASC);
		}
	}
}

// 把所有 ASC 上的这个技能移除（仅服务器）
void UTenkaichiGlobalAbilitySystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && AppliedAbilities.Contains(Ability))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities[Ability];
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

// 把所有 ASC 上的这个效果移除（仅服务器）
void UTenkaichiGlobalAbilitySystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && AppliedEffects.Contains(Effect))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects[Effect];
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

// 把某个 ASC 注册进全局系统，并补授所有已全局应用的技能/效果
void UTenkaichiGlobalAbilitySystem::RegisterASC(UTenkaichiAbilitySystemComponent* ASC)
{
	check(ASC);

	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}

	RegisteredASCs.AddUnique(ASC);
}

// 把某个 ASC 从全局系统移除，并移除它身上所有全局应用的技能/效果
void UTenkaichiGlobalAbilitySystem::UnregisterASC(UTenkaichiAbilitySystemComponent* ASC)
{
	check(ASC);
	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.RemoveFromASC(ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.RemoveFromASC(ASC);
	}

	RegisteredASCs.Remove(ASC);
}