// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichi, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiExperience, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiAbilitySystem, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiTeams, Log, All);

// 根据对象当前运行在网络中的角色（服务器/客户端），返回一个用于日志前缀的上下文字符串
TENKAICHIGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);