// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbility_Jump.h"

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"
#include "Character/TenkaichiCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility_Jump)

struct FGameplayTagContainer;


// 构造函数：设为按 Actor 实例化 + 本地预测（客户端立即响应，不等服务器）
UTenkaichiGameplayAbility_Jump::UTenkaichiGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

// 判断能否激活：依次检查角色是否存在、角色当前能否跳跃、以及基类是否允许
bool UTenkaichiGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const ATenkaichiCharacter* TenkaichiCharacter = Cast<ATenkaichiCharacter>(ActorInfo->AvatarActor.Get());
	if (!TenkaichiCharacter || !TenkaichiCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

// 技能结束：兜底停止跳跃（防蓝图忘记调用），再交给父类处理
void UTenkaichiGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 万一蓝图没有调用停止跳跃，这里兜底停一次
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// 让本地控制的角色开始跳跃（若尚未处于跳跃状态）
void UTenkaichiGameplayAbility_Jump::CharacterJumpStart()
{
	if (ATenkaichiCharacter* TenkaichiCharacter = GetTenkaichiCharacterFromActorInfo())
	{
		if (TenkaichiCharacter->IsLocallyControlled() && !TenkaichiCharacter->bPressedJump)
		{
			TenkaichiCharacter->UnCrouch();
			TenkaichiCharacter->Jump();
		}
	}
}

// 让本地控制的角色停止跳跃（若正处于跳跃状态）
void UTenkaichiGameplayAbility_Jump::CharacterJumpStop()
{
	if (ATenkaichiCharacter* TenkaichiCharacter = GetTenkaichiCharacterFromActorInfo())
	{
		if (TenkaichiCharacter->IsLocallyControlled() && TenkaichiCharacter->bPressedJump)
		{
			TenkaichiCharacter->StopJumping();
		}
	}
}