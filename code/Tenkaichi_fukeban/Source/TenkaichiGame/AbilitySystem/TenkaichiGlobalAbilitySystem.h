// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpecHandle.h"
#include "Templates/SubclassOf.h"

#include "TenkaichiGlobalAbilitySystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UTenkaichiAbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;

/** 记录「一个全局技能」被授给了哪些 ASC、各自的技能句柄是什么 */
USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UTenkaichiAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	// 把这个全局技能授给指定的 ASC
	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UTenkaichiAbilitySystemComponent* ASC);
	// 从指定的 ASC 上移除这个全局技能
	void RemoveFromASC(UTenkaichiAbilitySystemComponent* ASC);
	// 从所有 ASC 上移除这个全局技能
	void RemoveFromAll();
};

/** 记录「一个全局效果」被加到了哪些 ASC、各自的效果句柄是什么 */
USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UTenkaichiAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	// 把这个全局效果加给指定的 ASC
	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UTenkaichiAbilitySystemComponent* ASC);
	// 从指定的 ASC 上移除这个全局效果
	void RemoveFromASC(UTenkaichiAbilitySystemComponent* ASC);
	// 从所有 ASC 上移除这个全局效果
	void RemoveFromAll();
};

/** 全局技能系统：给世界里的所有 ASC 批量授予/移除技能和效果 */
UCLASS()
class UTenkaichiGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 构造函数
	UTenkaichiGlobalAbilitySystem();

	// 把技能授给所有已注册的 ASC（仅服务器授权）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Tenkaichi")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	// 把效果加给所有已注册的 ASC（仅服务器授权）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Tenkaichi")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	// 把所有 ASC 上的这个技能移除（仅服务器授权）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tenkaichi")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	// 把所有 ASC 上的这个效果移除（仅服务器授权）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tenkaichi")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** 把某个 ASC 注册进全局系统，并补授所有已全局应用的技能/效果 */
	void RegisterASC(UTenkaichiAbilitySystemComponent* ASC);

	/** 把某个 ASC 从全局系统移除，并移除它身上所有全局应用的技能/效果 */
	void UnregisterASC(UTenkaichiAbilitySystemComponent* ASC);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UTenkaichiAbilitySystemComponent>> RegisteredASCs;
};