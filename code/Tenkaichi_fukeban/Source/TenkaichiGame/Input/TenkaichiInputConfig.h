// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "TenkaichiInputConfig.generated.h"

class UInputAction;
class UObject;
struct FFrame;

/** 一条「输入动作 ↔ 技能标签」的映射 */
USTRUCT(BlueprintType)
struct FTenkaichiInputAction
{
	GENERATED_BODY()

public:

	/** 输入动作（例如 IA_Fire），用只读指针存，避免误改 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	/** 这个输入动作对应的技能标签（例如 InputTag.MouseButton.Left） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** 存放「输入配置」的数据资产：把输入动作和技能标签绑定起来 */
UCLASS(BlueprintType, Const)
class UTenkaichiInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer);

	// 根据标签，在「原生输入」里找出对应的输入动作（找不到可打印错误日志）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// 根据标签，在「技能输入」里找出对应的输入动作（找不到可打印错误日志）
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// 拥有者使用的「原生输入」列表。这些输入动作绑定到游戏标签，需要手动绑定
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTenkaichiInputAction> NativeInputActions;

	// 拥有者使用的「技能输入」列表。这些输入动作绑定到游戏标签，会自动绑定到带匹配标签的技能上
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTenkaichiInputAction> AbilityInputActions;
};