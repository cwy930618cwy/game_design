// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiGamePhaseAbility.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "TenkaichiGamePhaseSubsystem.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGamePhaseAbility)

#define LOCTEXT_NAMESPACE "UTenkaichiGamePhaseAbility"

// 构造函数：把阶段技能设为服务器权威、不复制、按 Actor 实例化
UTenkaichiGamePhaseAbility::UTenkaichiGamePhaseAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

// 技能激活时：在服务器权威端通知阶段子系统，让新阶段开始
void UTenkaichiGamePhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UTenkaichiGamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UTenkaichiGamePhaseSubsystem>(World);
		PhaseSubsystem->OnBeginPhase(this, Handle);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

// 技能结束时：在服务器权威端通知阶段子系统，让当前阶段结束
void UTenkaichiGamePhaseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UTenkaichiGamePhaseSubsystem* PhaseSubsystem = UWorld::GetSubsystem<UTenkaichiGamePhaseSubsystem>(World);
		PhaseSubsystem->OnEndPhase(this, Handle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

#if WITH_EDITOR
// 编辑器数据校验：阶段标签未配置则判定为无效并报错
EDataValidationResult UTenkaichiGamePhaseAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!GamePhaseTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("GamePhaseTagNotSet", "GamePhaseTag must be set to a tag representing the current phase."));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE