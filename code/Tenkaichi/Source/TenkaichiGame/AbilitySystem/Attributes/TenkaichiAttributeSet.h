// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AttributeSet.h"

#include "TenkaichiAttributeSet.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class UTenkaichiAbilitySystemComponent;
class UObject;
class UWorld;
struct FGameplayEffectSpec;


/**
 * 这个宏定义了一组用于访问和初始化属性的辅助函数。
 *
 * 宏的用法示例：
 *		ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Health)
 * 会生成以下函数：
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 用于广播属性事件的委托，其中一些参数在客户端上可能为 null：
 * @param EffectInstigator	造成这次事件的原始发起者
 * @param EffectCauser		物理上造成这次变化的对象
 * @param EffectSpec		这次变化的完整效果规格
 * @param EffectMagnitude	原始数值大小，这是在做钳制之前的值
 * @param OldValue			属性在改变之前的值
 * @param NewValue			属性在改变之后的值
 */
DECLARE_MULTICAST_DELEGATE_SixParams(FTenkaichiAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

/**
 * UTenkaichiAttributeSet
 *
 *	项目的属性集基类。
 */
UCLASS(MinimalAPI)
class UTenkaichiAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	// 构造函数
	UE_API UTenkaichiAttributeSet();

	// 拿到这个属性集所属的世界
	UE_API UWorld* GetWorld() const override;

	// 拿到这个属性集所属的项目的 AbilitySystemComponent
	UE_API UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
};

#undef UE_API