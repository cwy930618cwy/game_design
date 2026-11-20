// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "TenkaichiAttributeSet.h"
#include "NativeGameplayTags.h"

#include "TenkaichiHealthSet.generated.h"

#define UE_API TENKAICHIGAME_API

class UObject;
struct FFrame;

TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Tenkaichi_Damage_Message);

struct FGameplayEffectModCallbackData;


/**
 * UTenkaichiHealthSet
 *
 *	定义承受伤害所必需的属性的类。
 *	属性示例包括：生命值、护盾和抗性。
 */
UCLASS(MinimalAPI, BlueprintType)
class UTenkaichiHealthSet : public UTenkaichiAttributeSet
{
	GENERATED_BODY()

public:

	// 构造函数
	UE_API UTenkaichiHealthSet();

	// 为 Health 生成访问函数（Get/Set/Init/GetAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Health);
	// 为 MaxHealth 生成访问函数（Get/Set/Init/GetAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, MaxHealth);
	// 为 Healing 生成访问函数（Get/Set/Init/GetAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Healing);
	// 为 Damage 生成访问函数（Get/Set/Init/GetAttribute）
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Damage);

	// 生命值因伤害/治疗而变化时的委托（客户端上部分信息可能缺失）
	mutable FTenkaichiAttributeEvent OnHealthChanged;

	// 最大生命值变化时的委托
	mutable FTenkaichiAttributeEvent OnMaxHealthChanged;

	// 生命值归零时要广播的委托
	mutable FTenkaichiAttributeEvent OnOutOfHealth;

protected:

	// Health 复制回来时的回调
	UFUNCTION()
	UE_API void OnRep_Health(const FGameplayAttributeData& OldValue);

	// MaxHealth 复制回来时的回调
	UFUNCTION()
	UE_API void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	// 在游戏效果实际执行之前调用（处理伤害/治疗前的时机）
	UE_API virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	// 在游戏效果实际执行之后调用（把伤害/治疗算进生命值）
	UE_API virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 在属性的基础值即将改变之前调用
	UE_API virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// 在属性值即将改变之前调用
	UE_API virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// 在属性值改变之后调用
	UE_API virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// 把属性值钳制到允许的范围内
	UE_API void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

	// 当前生命值属性。生命值会被最大生命值封顶。生命值对修饰器隐藏，只能由执行（Executions）修改。
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Tenkaichi|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	// 当前最大生命值属性。最大生命值也是属性，因为游戏效果可以修改它。
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Tenkaichi|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	// 用来跟踪生命值是否归零
	bool bOutOfHealth;

	// 在任何变化之前保存生命值
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;

	// -------------------------------------------------------------------
	//	元属性（请把不"有状态"的属性放在下面）
	// -------------------------------------------------------------------

	// 传入的治疗量。直接映射到 +Health
	UPROPERTY(BlueprintReadOnly, Category="Tenkaichi|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// 传入的伤害量。直接映射到 -Health
	UPROPERTY(BlueprintReadOnly, Category="Tenkaichi|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};

#undef UE_API