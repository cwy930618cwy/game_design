// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "TenkaichiVerbMessageHelpers.generated.h"

#define UE_API TENKAICHIGAME_API

class APlayerController;
class APlayerState;
class UObject;
struct FGameplayCueParameters;
struct FTenkaichiVerbMessage;

/**
 * UTenkaichiVerbMessageHelpers
 *
 * 消息相关的静态工具函数库：从任意对象反查玩家状态/控制器，
 * 以及在 VerbMessage 和 GameplayCue 参数之间互相转换。
 */
UCLASS(MinimalAPI)
class UTenkaichiVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 从任意对象（Actor/Pawn/Controller/PlayerState 等）反查它所属的 PlayerState
	UE_API static APlayerState* GetPlayerStateFromObject(UObject* Object);
	// 从任意对象反查它所属的 PlayerController
	UE_API static APlayerController* GetPlayerControllerFromObject(UObject* Object);
	// 把一条 VerbMessage 转换成 GameplayCue 参数（⚠️ 依赖 GAS，.cpp 里挂起）
	UE_API static FGameplayCueParameters VerbMessageToCueParameters(const FTenkaichiVerbMessage& VerbMessage);
	// 把 GameplayCue 参数转换回一条 VerbMessage（⚠️ 依赖 GAS，.cpp 里挂起）
	UE_API static FTenkaichiVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& GameplayCueParameters);
};

#undef UE_API