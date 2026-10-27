// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "TenkaichiAbilitySimpleFailureMessage.generated.h"

// 声明"技能简单失败消息"的标签（定义在 TenkaichiGameplayAbility.cpp 里）
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE);

/** 技能激活失败的简单反馈消息（给玩家看的文字） */
USTRUCT(BlueprintType)
struct FTenkaichiAbilitySimpleFailureMessage
{
	GENERATED_BODY()

public:
	// 失败的玩家控制器
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// 失败原因标签集合
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	// 给玩家看的失败原因文字
	UPROPERTY(BlueprintReadWrite)
	FText UserFacingReason;
};