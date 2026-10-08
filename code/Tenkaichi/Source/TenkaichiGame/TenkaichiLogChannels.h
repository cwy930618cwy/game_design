// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichi, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiExperience, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiAbilitySystem, Log, All);
TENKAICHIGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiTeams, Log, All);

TENKAICHIGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);