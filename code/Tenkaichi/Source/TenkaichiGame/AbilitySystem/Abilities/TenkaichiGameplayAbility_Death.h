// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TenkaichiGameplayAbility.h"

#include "TenkaichiGameplayAbility_Death.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;


/**
 * UTenkaichiGameplayAbility_Death
 *
 *	用于处理死亡的技能。
 *	该技能通过 "GameplayEvent.Death" 技能触发标签自动激活。
 */
UCLASS(Abstract)
class UTenkaichiGameplayAbility_Death : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:

	// 构造函数：设置实例化/网络策略、默认自动开始死亡，并为 CDO 添加死亡触发标签
	UTenkaichiGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 激活时取消其他所有技能、禁止自身被取消，并切换到独占阻塞激活组
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 结束时兜底完成死亡流程（若死亡已开始）
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 开始死亡流程
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	void StartDeath();

	// 完成死亡流程
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	void FinishDeath();

protected:

	// 若启用，技能会自动调用 StartDeath。若死亡已开始，则技能结束时总会调用 FinishDeath。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Death")
	bool bAutoStartDeath;
};