// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "TenkaichiInteractionDurationMessage.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_INTERACTION_DURATION_MESSAGE);

/**
 * 交互时长消息：广播"谁正在交互 + 需要持续多久"
 */
USTRUCT(BlueprintType)
struct FTenkaichiInteractionDurationMessage
{
	GENERATED_BODY()

public:
	/** 发起交互的人 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	/** 需要持续的秒数 */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0;
};