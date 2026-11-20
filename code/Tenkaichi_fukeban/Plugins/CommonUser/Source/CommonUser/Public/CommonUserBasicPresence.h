// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUserBasicPresence.generated.h"

#define UE_API COMMONUSER_API

class UCommonSessionSubsystem;
enum class ECommonSessionInformationState : uint8;

//////////////////////////////////////////////////////////////////////
// UCommonUserBasicPresence

/**
 * 此子系统接入会话子系统，并将其信息推送到在线状态（presence）接口。
 * 它并不打算成为一个功能完整的富在线状态实现，但可以作为
 * 将信息从会话子系统推送到在线状态系统的概念验证示例。
 */
UCLASS(MinimalAPI, BlueprintType, Config = Engine)
class UCommonUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API UCommonUserBasicPresence();


	/** 实现此方法以初始化该系统的实例 */
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 实现此方法以反初始化该系统的实例 */
	UE_API virtual void Deinitialize() override;

	/** False 是一个通用的总开关，用于阻止此类推送在线状态 */
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** 将在线状态 "In-game" 映射到后端键 */
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** 将在线状态 "Main Menu" 映射到后端键 */
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** 将在线状态 "Matchmaking" 映射到后端键 */
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** 将富在线状态条目 "Game Mode" 映射到后端键 */
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** 将富在线状态条目 "Map Name" 映射到后端键 */
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	UE_API void OnNotifySessionInformationChanged(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	UE_API FString SessionStateToBackendKey(ECommonSessionInformationState SessionStatus);
};

#undef UE_API
