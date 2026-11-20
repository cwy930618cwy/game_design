// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_GrantNearbyInteraction.generated.h"

class UGameplayAbility;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;
struct FObjectKey;

// 能力任务：周期性扫描周围空间，找出能交互的目标并给玩家授权对应的交互能力
UCLASS()
class UAbilityTask_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	// 任务启动入口：在这里挂上定时器，周期性触发扫描
	virtual void Activate() override;

	// 工厂方法：创建本任务实例，并指定扫描半径与扫描间隔（蓝图可调用）
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantNearbyInteraction* GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, float InteractionScanRange, float InteractionScanRate);

private:

	// 任务销毁时被调用：负责取消定时器、收尾
	virtual void OnDestroy(bool AbilityEnded) override;

	// 核心扫描逻辑：做一次球形检测，收集可交互目标并授权
	void QueryInteractables();

	// 扫描半径（厘米），默认 100
	float InteractionScanRange = 100;

	// 扫描间隔（秒），默认 0.1 秒扫一次
	float InteractionScanRate = 0.100;

	// 定时器句柄，控制周期性扫描
	FTimerHandle QueryTimerHandle;

	// 缓存：记录「已授权过的交互能力」，避免重复授权
	TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};