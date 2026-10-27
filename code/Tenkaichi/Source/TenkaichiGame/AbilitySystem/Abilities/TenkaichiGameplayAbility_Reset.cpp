// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility_Reset.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "Character/TenkaichiCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "TenkaichiGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility_Reset)

// 构造函数：设置实例化与网络策略，并为 CDO 添加「请求重置」触发标签
UTenkaichiGameplayAbility_Reset::UTenkaichiGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 只在类默认对象（CDO）上配置一次，所有实例自动继承
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// 把触发标签作为默认值添加到 CDO 上
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = TenkaichiGameplayTags::GameplayEvent_RequestReset;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

// 激活时取消其他所有技能、禁止自身被取消，然后调用角色自身的 Reset 并广播重置消息
void UTenkaichiGameplayAbility_Reset::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UTenkaichiAbilitySystemComponent* TenkaichiASC = CastChecked<UTenkaichiAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(TenkaichiGameplayTags::Ability_Behavior_SurvivesDeath);

	// 取消所有技能，并阻止其他技能启动
	TenkaichiASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	// 由角色执行实际的重置
	if (ATenkaichiCharacter* TenkaichiChar = Cast<ATenkaichiCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		TenkaichiChar->Reset();
	}

	// 通知外界已发生重置
	FTenkaichiPlayerResetMessage Message;
	Message.OwnerPlayerState = CurrentActorInfo->OwnerActor.Get();
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TenkaichiGameplayTags::GameplayEvent_Reset, Message);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bReplicateEndAbility = true;
	const bool bWasCanceled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCanceled);
}