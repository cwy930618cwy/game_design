// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"
#include "Interaction/InteractionOption.h"

#include "TenkaichiGameplayAbility_Interact.generated.h"

class UIndicatorDescriptor;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UTenkaichiGameplayAbility_Interact
 *
 * 用于角色交互的游戏能力
 */
UCLASS(Abstract)
class UTenkaichiGameplayAbility_Interact : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:

	// 构造函数（在 .cpp 里设置激活/实例化/网络执行策略）
	UTenkaichiGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 能力激活入口：在服务端启动交互扫描并授予附近目标
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable)
	// 接收扫描结果，更新目标头上的交互指示器
	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);

	UFUNCTION(BlueprintCallable)
	// 玩家按下交互键时调用，触发目标上的交互技能
	void TriggerInteraction();

protected:
	UPROPERTY(BlueprintReadWrite)
	// 当前可交互选项列表
	TArray<FInteractionOption> CurrentOptions;

	UPROPERTY()
	// 当前显示在目标头上的指示器
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;

protected:

	UPROPERTY(EditDefaultsOnly)
	// 扫描频率（秒）
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	// 扫描半径
	float InteractionScanRange = 500;

	UPROPERTY(EditDefaultsOnly)
	// 默认指示器 UI（软引用）
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};