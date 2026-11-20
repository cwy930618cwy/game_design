// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TenkaichiGameplayAbility.h"

#include "TenkaichiGameplayAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UTenkaichiGameplayAbility_Jump
 *
 *	用于角色跳跃的技能
 */
UCLASS(Abstract)
class UTenkaichiGameplayAbility_Jump : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:

	// 构造函数：设为按 Actor 实例化 + 本地预测（客户端立即响应）
	UTenkaichiGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 判断当前是否允许激活本技能（角色存在、可跳、且基类允许）
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	// 技能结束时兜底停止跳跃
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 让角色开始跳跃
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	void CharacterJumpStart();

	// 让角色停止跳跃
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Ability")
	void CharacterJumpStop();
};