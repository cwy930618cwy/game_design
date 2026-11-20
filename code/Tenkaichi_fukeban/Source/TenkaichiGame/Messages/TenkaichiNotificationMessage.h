// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#include "TenkaichiNotificationMessage.generated.h"

class UObject;

// 声明一个"添加通知"的标签（真正的定义在 .cpp 里用 UE_DEFINE_GAMEPLAY_TAG 完成）
TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Tenkaichi_AddNotification_Message);

class APlayerState;

// 发往"临时日志"的消息（例如击杀播报、物品拾取流）
USTRUCT(BlueprintType)
struct FTenkaichiNotificationMessage
{
	GENERATED_BODY()

	// 目标通道（显示到哪条日志，如"击杀播报"）
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag TargetChannel;

	// 目标玩家（留空 = 显示给所有本地玩家）
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<APlayerState> TargetPlayer = nullptr;

	// 要显示的那句话
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FText PayloadMessage;

	// 目标通道专属的额外载荷（如样式或定义资产）
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	FGameplayTag PayloadTag;

	// 目标通道专属的额外载荷（如样式或定义资产）
	UPROPERTY(BlueprintReadWrite, Category=Notification)
	TObjectPtr<UObject> PayloadObject = nullptr;
};