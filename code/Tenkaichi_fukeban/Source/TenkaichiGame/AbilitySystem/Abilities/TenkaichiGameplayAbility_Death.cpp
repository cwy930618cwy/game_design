// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGameplayAbility_Death.h"

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "TenkaichiGameplayTags.h"
#include "TenkaichiLogChannels.h"
#include "Trace/Trace.inl"

#include "Character/TenkaichiHealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayAbility_Death)

// 构造函数：设置实例化/网络策略、默认自动开始死亡，并为 CDO 添加死亡触发标签
UTenkaichiGameplayAbility_Death::UTenkaichiGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bAutoStartDeath = true;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// 把技能触发标签作为默认值添加到 CDO
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = TenkaichiGameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

// 激活时：取消其他所有技能（豁免带存活标签的）、禁止自身被取消，并切换到独占阻塞激活组
void UTenkaichiGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UTenkaichiAbilitySystemComponent* TenkaichiASC = CastChecked<UTenkaichiAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(TenkaichiGameplayTags::Ability_Behavior_SurvivesDeath);

	// 取消所有技能，并阻止其他技能启动
	TenkaichiASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	if (!ChangeActivationGroup(ETenkaichiAbilityActivationGroup::Exclusive_Blocking))
	{
		UE_LOG(LogTenkaichiAbilitySystem, Error, TEXT("UTenkaichiGameplayAbility_Death::ActivateAbility: Ability [%s] failed to change activation group to blocking."), *GetName());
	}

	if (bAutoStartDeath)
	{
		StartDeath();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// 结束时：兜底完成死亡流程（若死亡已开始），再交给父类处理
void UTenkaichiGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// 技能结束时总是尝试完成死亡，以防技能自身没有完成。
	// 如果死亡尚未开始，这里不会有任何作用。
	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// 通知生命组件开始死亡流程（仅在尚未死亡时生效）
void UTenkaichiGameplayAbility_Death::StartDeath()
{
	if (UTenkaichiHealthComponent* HealthComponent = UTenkaichiHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == ETenkaichiDeathState::NotDead)
		{
			HealthComponent->StartDeath();
		}
	}
}

// 通知生命组件完成死亡流程（仅在死亡已开始时生效）
void UTenkaichiGameplayAbility_Death::FinishDeath()
{
	if (UTenkaichiHealthComponent* HealthComponent = UTenkaichiHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		if (HealthComponent->GetDeathState() == ETenkaichiDeathState::DeathStarted)
		{
			HealthComponent->FinishDeath();
		}
	}
}