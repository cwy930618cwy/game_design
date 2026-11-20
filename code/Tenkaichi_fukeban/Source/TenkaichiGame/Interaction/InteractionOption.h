// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

class IInteractableTarget;
class UUserWidget;

/**  */
USTRUCT(BlueprintType)
struct FInteractionOption
{
	GENERATED_BODY()

public:
	/** 可交互的目标（那个能被你交互的东西） */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IInteractableTarget> InteractableTarget;

	/** 交互显示的主文本（比如"拾取"） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** 交互显示的副文本（比如"铁剑"） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubText;

	// 交互方式
	//--------------------------------------------------------------

	// 方式 1：把技能放在发起者身上，交互时触发

	/** 靠近可交互物时，授予发起者的技能（交互时触发它） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant;

	// - 或 -

	// 方式 2：被交互的目标自己有技能系统，直接触发它身上的技能

	/** 目标身上的技能系统，可用于 TargetInteractionHandle 和发送事件 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** 要在目标身上触发的技能 Spec 句柄 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	// UI
	//--------------------------------------------------------------

	/** 显示这种交互要用的 UI 控件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;

	//--------------------------------------------------------------

public:
	// 重载运算符，让这个结构体能放进数组比较、排序
	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			InteractionAbilityToGrant == Other.InteractionAbilityToGrant &&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &&
			InteractionWidgetClass == Other.InteractionWidgetClass &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};