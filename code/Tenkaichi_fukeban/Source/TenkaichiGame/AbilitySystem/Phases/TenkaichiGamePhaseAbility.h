// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"
#include "TenkaichiGamePhaseAbility.generated.h"

/**
 * UTenkaichiGamePhaseAbility
 *
 * 所有用于切换当前游戏阶段的技能的基类
 */
UCLASS(Abstract, HideCategories = Input)
class UTenkaichiGamePhaseAbility : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:

	// 构造函数：设置阶段技能所需的复制/实例化/网络策略
	UTenkaichiGamePhaseAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取本技能代表的阶段标签
	const FGameplayTag& GetGamePhaseTag() const { return GamePhaseTag; }

#if WITH_EDITOR
	// 编辑器数据校验：检查阶段标签是否已配置
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:

	// 激活技能时通知阶段子系统，让新阶段开始
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 结束技能时通知阶段子系统，让当前阶段结束
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	// 定义本阶段技能所属的游戏阶段。举例：
	// 如果你的阶段是 GamePhase.RoundStart，那么它会取消所有同级（sibling）阶段。
	// 比如当前活跃的是 GamePhase.WaitingToStart，启动属于 RoundStart 的技能就会结束 WaitingToStart。
	// 但你也可以通过嵌套来实现层级行为。例如，GamePhase.Playing.NormalPlay 是父阶段
	// GamePhase.Playing 的子阶段，所以把子阶段切换到 GamePhase.Playing.SuddenDeath，
	// 会停止所有绑定 GamePhase.Playing.* 的技能，但不会结束绑定 GamePhase.Playing 阶段的技能。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Game Phase")
	FGameplayTag GamePhaseTag;
};