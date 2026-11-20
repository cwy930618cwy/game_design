// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

#include "GameplayAbilitySpecHandle.h"
#include "TenkaichiAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiGameplayAbility;
class UObject;


/**
 * FTenkaichiAbilitySet_GameplayAbility
 *
 *	这个能力集要授予的一个「技能」的数据：技能类 + 等级 + 输入标签
 */
USTRUCT(BlueprintType)
struct FTenkaichiAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:

	// 要授予的技能类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTenkaichiGameplayAbility> Ability = nullptr;

	// 授予的技能等级
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// 用于给这个技能处理输入的标签
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/**
 * FTenkaichiAbilitySet_GameplayEffect
 *
 *	这个能力集要授予的一个「效果」的数据：效果类 + 等级
 */
USTRUCT(BlueprintType)
struct FTenkaichiAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	// 要授予的效果类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// 授予的效果等级
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 * FTenkaichiAbilitySet_AttributeSet
 *
 *	这个能力集要授予的一个「属性集」的数据：属性集类
 */
USTRUCT(BlueprintType)
struct FTenkaichiAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// 要授予的属性集类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

};

/**
 * FTenkaichiAbilitySet_GrantedHandles
 *
 *	记录这个能力集已经授予了哪些技能/效果/属性集（用句柄/指针），方便之后一次性收回
 */
USTRUCT(BlueprintType)
struct FTenkaichiAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:

	// 记下已授予的技能句柄
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	// 记下已授予的效果句柄
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	// 记下已授予的属性集指针
	void AddAttributeSet(UAttributeSet* Set);

	// 把记录的所有技能/效果/属性集从 ASC 上收回
	void TakeFromAbilitySystem(UTenkaichiAbilitySystemComponent* LyraASC);

protected:

	// 已授予的技能句柄列表
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// 已授予的效果句柄列表
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// 已授予的属性集指针列表
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * UTenkaichiAbilitySet
 *
 *	不可变的数据资产：一次性授予一组技能/效果/属性集
 */
UCLASS(BlueprintType, Const)
class UTenkaichiAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// 构造函数
	UTenkaichiAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 把这个能力集授予指定的 ASC；返回的句柄收集器可用于之后收回所有已授予内容
	void GiveToAbilitySystem(UTenkaichiAbilitySystemComponent* LyraASC, FTenkaichiAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:

	// 授予这个能力集时要授予的技能列表
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FTenkaichiAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// 授予这个能力集时要授予的效果列表
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FTenkaichiAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// 授予这个能力集时要授予的属性集列表
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FTenkaichiAbilitySet_AttributeSet> GrantedAttributes;
};