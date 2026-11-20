// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "InteractionQuery.generated.h"


/**  */
USTRUCT(BlueprintType)
struct FInteractionQuery
{
	GENERATED_BODY()

public:
	/** 发起交互的 Pawn（请求交互的那具身体） */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** 允许我们指定一个控制器，不需要与发起交互的 Pawn 的拥有者匹配 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;

	/** 一个用来塞入交互所需额外数据的通用 UObject 槽 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> OptionalObjectData;
};