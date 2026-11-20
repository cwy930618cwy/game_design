// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"

#include "CommonGameInstance.generated.h"

#define UE_API COMMONGAME_API

enum class ECommonUserAvailability : uint8;
enum class ECommonUserPrivilege : uint8;

class FText;
class UCommonUserInfo;
class UCommonSession_SearchResult;
struct FOnlineResultInformation;
class ULocalPlayer;
class USocialManager;
class UObject;
struct FFrame;
struct FGameplayTag;

UCLASS(MinimalAPI, Abstract, Config = Game)
class UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UE_API UCommonGameInstance(const FObjectInitializer& ObjectInitializer);
	
	/** 处理来自 CommonUser 的错误/警告，每个游戏可各自重写 */
	UFUNCTION()
	UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

	UFUNCTION()
	UE_API virtual void HandlePrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);

	UFUNCTION()
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	/** 调用它来重置用户和会话状态，通常是因为某个玩家断线了 */
	UE_API virtual void ResetUserAndSessionState();

	/**
	 * 请求会话流程
	 *   有东西请求用户加入某个特定会话（例如，平台覆盖层通过 OnUserRequestedSession 发起）。
	 *   这个请求在 SetRequestedSession 里被处理。
	 *   检查我们是否能立即加入所请求的会话（CanJoinRequestedSession）。如果能，就加入该会话（JoinRequestedSession）。
	 *   如果不能，就缓存所请求的会话，并指示游戏进入一个能够加入该会话的状态（ResetGameAndJoinRequestedSession）。
	 */
	/** 处理用户从外部来源（例如平台覆盖层）接受会话邀请的情况。预期由每个游戏各自重写。 */
	UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** 处理 OSS 请求销毁会话的情况 */
	UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);

	/** 获取所请求的会话 */
	UCommonSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	/** 设置（或清除）所请求的会话。设置后，请求会话流程就会开始。 */
	UE_API virtual void SetRequestedSession(UCommonSession_SearchResult* InRequestedSession);
	/** 检查所请求的会话是否能加入。可由每个游戏各自重写。 */
	UE_API virtual bool CanJoinRequestedSession() const;
	/** 加入所请求的会话 */
	UE_API virtual void JoinRequestedSession();
	/** 让游戏进入能够加入所请求会话的状态 */
	UE_API virtual void ResetGameAndJoinRequestedSession();
	
	UE_API virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	UE_API virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

private:
	/** 这是主玩家 */
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	/** 玩家请求加入的会话 */
	UPROPERTY()
	TObjectPtr<UCommonSession_SearchResult> RequestedSession;
};

#undef UE_API
