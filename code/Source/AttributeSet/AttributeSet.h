// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CodeAttributeSet.generated.h"

// 【重点】UCLASS 宏：告诉 UE 这是一个 GAS 属性集类
UCLASS()
class CODE_API UCodeAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	// 【重点】声明一个属性：生命值
	// FGameplayAttributeData 是 GAS 的属性类型
	// Health 是我们给这个属性起的名字
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	// 【重点】宏：自动生成 Get/Set/Init 函数
	// 以后用 GetHealth() 读值，SetHealth(100) 设值
	ATTRIBUTE_ACCESSORS(UCodeAttributeSet, Health);

protected:
	// 网络复制回调（多人游戏同步血量用，单机先不管）
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
};
