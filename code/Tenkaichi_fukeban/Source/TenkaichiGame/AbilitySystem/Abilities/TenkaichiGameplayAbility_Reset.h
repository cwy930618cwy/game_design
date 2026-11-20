// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/TenkaichiGameplayAbility.h"

#include "TenkaichiGameplayAbility_Reset.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UTenkaichiGameplayAbility_Reset
 *
 *	用于把玩家快速重置回初始出生状态的技能。
 *	该技能通过 "GameplayEvent.RequestReset" 技能触发标签自动激活（仅服务器）。
 */
UCLASS(MinimalAPI)
class UTenkaichiGameplayAbility_Reset : public UTenkaichiGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API UTenkaichiGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	// 激活时取消其他所有技能、禁止自身被取消，然后调用角色自身的 Reset 并广播重置消息
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};



// 玩家重置消息：通知外界"某个玩家的 PlayerState 被重置了"
USTRUCT(BlueprintType)
struct FTenkaichiPlayerResetMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> OwnerPlayerState = nullptr;
};

#undef UE_API