// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectPtr.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/WeakObjectPtr.h"
#include "PartyBeaconClient.h"
#include "PartyBeaconHost.h"
#include "PartyBeaconState.h"
#if! COMMONUSER_OSSV1
#include "Online/Sessions.h"
#endif



class APlayerController;
class AOnlineBeaconHost;
class ULocalPlayer;
namespace ETravelFailure { enum Type : int; }
struct FOnlineResultInformation;

#if COMMONUSER_OSSV1
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#else
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#endif // COMMONUSER_OSSV1

#include "CommonSessionSubsystem.generated.h"

class UWorld;
class FCommonSession_OnlineSessionSettings;

#if COMMONUSER_OSSV1
class FCommonOnlineSearchSettingsOSSv1;
using FCommonOnlineSearchSettings = FCommonOnlineSearchSettingsOSSv1;
#else
class FCommonOnlineSearchSettingsOSSv2;
using FCommonOnlineSearchSettings = FCommonOnlineSearchSettingsOSSv2;
#endif // COMMONUSER_OSSV1


//////////////////////////////////////////////////////////////////////
// UCommonSession_HostSessionRequest

/** 指定游戏会话应使用的在线功能与连接方式 */
UENUM(BlueprintType)
enum class ECommonSessionOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

/** 一个请求对象，存储主持游戏会话时使用的参数 */
UCLASS(MinimalAPI, BlueprintType)
class UCommonSession_HostSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/** 指示该会话是完全在线会话还是其他类型 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	ECommonSessionOnlineMode OnlineMode;

	/** 若为 true，此请求将在可用时创建一个玩家主持的大厅（lobby） */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 若为 true，此请求将在可用时创建一个启用语音聊天的大厅 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbiesVoiceChat;

	/** 若为 true，此请求将创建一个会出现在用户在线状态信息中的会话 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUsePresence;

	/** 匹配过程中用于指定这是什么类型的游戏模式的字符串 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	FString ModeNameForAdvertisement;

	/** 游戏开始时将加载的地图，这必须是一个有效的 Primary Asset 顶层地图 */
	UPROPERTY(BlueprintReadWrite, Category=Session, meta=(AllowedTypes="World"))
	FPrimaryAssetId MapID;

	/** 作为 URL 选项传递给游戏的额外参数 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	TMap<FString, FString> ExtraArgs;

	/** 每个游戏会话允许的最大玩家数 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	int32 MaxPlayerCount = 16;

public:
	/** 返回实际应使用的最大玩家数，可在子类中重写 */
	COMMONUSER_API virtual int32 GetMaxPlayers() const;

	/** 返回游戏过程中将使用的完整地图名 */
	COMMONUSER_API virtual FString GetMapName() const;

	/** 构造将传递给 ServerTravel 的完整 URL */
	COMMONUSER_API virtual FString ConstructTravelURL() const;

	/** 若此请求有效则返回 true，无效则返回 false 并记录错误 */
	COMMONUSER_API virtual bool ValidateAndLogErrors(FText& OutError) const;
};


//////////////////////////////////////////////////////////////////////
// UCommonSession_SearchResult

/** 从在线系统返回的结果对象，描述一个可加入的游戏会话 */
UCLASS(MinimalAPI, BlueprintType)
class UCommonSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** 返回会话的内部描述，不用于人类阅读 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API FString GetDescription() const;

	/** 获取任意字符串设置，若该设置不存在则 bFoundValue 为 false */
	UFUNCTION(BlueprintPure, Category=Sessions)
	COMMONUSER_API void GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const;

	/** 获取任意整数设置，若该设置不存在则 bFoundValue 为 false */
	UFUNCTION(BlueprintPure, Category = Sessions)
	COMMONUSER_API void GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const;

	/** 可用的私有连接数量 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	COMMONUSER_API int32 GetNumOpenPrivateConnections() const;

	/** 可用的公共连接数量 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	COMMONUSER_API int32 GetNumOpenPublicConnections() const;

	/** 可用的最大公共连接数量，包括已被占用的连接 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	COMMONUSER_API int32 GetMaxPublicConnections() const;

	/** 到该搜索结果的 Ping 值，MAX_QUERY_PING 表示不可达 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	COMMONUSER_API int32 GetPingInMs() const;

public:
	/** 指向平台特定实现的指针 */
#if COMMONUSER_OSSV1
	FOnlineSessionSearchResult Result;
#else
	TSharedPtr<const UE::Online::FLobby> Lobby;

	UE::Online::FOnlineSessionId SessionID;
#endif // COMMONUSER_OSSV1

};


//////////////////////////////////////////////////////////////////////
// UCommonSession_SearchSessionRequest

/** 会话搜索完成时调用的委托 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FCommonSession_FindSessionsFinished, bool bSucceeded, const FText& ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCommonSession_FindSessionsFinishedDynamic, bool, bSucceeded, FText, ErrorMessage);

/** 描述会话搜索的请求对象，搜索完成后此对象会被更新 */
UCLASS(MinimalAPI, BlueprintType)
class UCommonSession_SearchSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/** 指示这是在查找完全在线游戏还是其他类型（如 LAN） */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	ECommonSessionOnlineMode OnlineMode;

	/** 若为 true，此请求将在可用时查找玩家主持的大厅；为 false 则只搜索已注册的服务器会话 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 所有已找到会话的列表，在调用 OnSearchFinished 时有效 */
	UPROPERTY(BlueprintReadOnly, Category=Session)
	TArray<TObjectPtr<UCommonSession_SearchResult>> Results;

	/** 会话搜索完成时调用的原生委托 */
	FCommonSession_FindSessionsFinished OnSearchFinished;

	/** 由子系统调用以执行已完成的委托 */
	COMMONUSER_API void NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage);

private:
	/** 会话搜索完成时调用的委托 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Search Finished", AllowPrivateAccess = true))
	FCommonSession_FindSessionsFinishedDynamic K2_OnSearchFinished;
};


//////////////////////////////////////////////////////////////////////
// CommonSessionSubsystem Events

/**
 * 当本地用户从外部来源（例如平台覆盖层）请求加入会话时触发的事件。
 * 通常，游戏应将玩家转入该会话。
 * @param LocalPlatformUserId 接受邀请的本地用户 ID。这是平台用户 ID，因为用户可能尚未登录。
 * @param RequestedSession 所请求的会话。若处理请求时发生错误，可能为 null。
 * @param RequestedSessionResult 所请求会话的处理结果
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UCommonSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UCommonSession_SearchResult*, RequestedSession, const FOnlineResultInformation&, RequestedSessionResult);

/**
 * 会话加入完成后触发的事件，发生在加入底层会话之后、成功则前往服务器之前。
 * 事件参数指示这是否成功，或者是否存在会阻止前往服务器的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnJoinSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonSessionOnJoinSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 用于主持的会话创建完成后触发的事件，发生在前往地图之前。
 * 事件参数指示这是否成功，或者是否存在会阻止前往服务器的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnCreateSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonSessionOnCreateSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 当本地用户从外部来源（例如平台覆盖层）请求销毁会话时触发的事件。
 * 游戏应将玩家移出该会话。
 * @param LocalPlatformUserId 发起销毁请求的本地用户 ID。这是平台用户 ID，因为用户可能尚未登录。
 * @param SessionName 会话的名称标识符。
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FCommonSessionOnDestroySessionRequested, const FPlatformUserId& /*LocalPlatformUserId*/, const FName& /*SessionName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCommonSessionOnDestroySessionRequested_Dynamic, const FPlatformUserId&, LocalPlatformUserId, const FName&, SessionName);

/**
 * 会话加入完成后触发的事件，发生在解析连接字符串之后、客户端前往之前。
 * @param URL 解析出的会话连接字符串，附带任何额外参数
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnPreClientTravel, FString& /*URL*/);

/**
 * 在会话生态系统的不同节点触发的事件，表示一个可呈现给用户的会话状态。
 * 这不应被用于在线功能（这类需求请使用 OnCreateSessionComplete 或 OnJoinSessionComplete），而是用于富在线状态等功能
 */
UENUM(BlueprintType)
enum class ECommonSessionInformationState : uint8
{
	OutOfGame,
	Matchmaking,
	InGame
};
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnSessionInformationChanged, ECommonSessionInformationState /*SessionStatus*/, const FString& /*GameMode*/, const FString& /*MapName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnSessionInformationChanged_Dynamic, ECommonSessionInformationState, SessionStatus, const FString&, GameMode, const FString&, MapName);

//////////////////////////////////////////////////////////////////////
// UCommonSessionSubsystem

/** 
 * 处理主持与加入在线游戏请求的游戏子系统。
 * 每个游戏实例会创建一个子系统，可从蓝图或 C++ 代码访问。
 * 如果存在游戏特定的子类，则不会创建此基础子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UCommonSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonSessionSubsystem() { }

	COMMONUSER_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	COMMONUSER_API virtual void Deinitialize() override;
	COMMONUSER_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 使用在线游戏的默认选项创建主持会话请求，创建后可修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	COMMONUSER_API virtual UCommonSession_HostSessionRequest* CreateOnlineHostSessionRequest();

	/** 使用默认选项创建会话搜索对象以查找默认在线游戏，创建后可修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	COMMONUSER_API virtual UCommonSession_SearchSessionRequest* CreateOnlineSearchSessionRequest();

	/** 使用会话请求信息创建新的在线游戏，若成功将启动硬地图转移 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API virtual void HostSession(APlayerController* HostingPlayer, UCommonSession_HostSessionRequest* Request);

	/** 启动一个流程来查找现有会话，若未找到可用会话则创建新会话 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API virtual void QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UCommonSession_HostSessionRequest* Request);

	/** 启动加入现有会话的流程，若成功将连接到指定服务器 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API virtual void JoinSession(APlayerController* JoiningPlayer, UCommonSession_SearchResult* Request);

	/** 查询在线系统，获取与搜索请求匹配的可加入会话列表 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API virtual void FindSessions(APlayerController* SearchingPlayer, UCommonSession_SearchSessionRequest* Request);

	/** 清理所有活动会话，从返回主菜单等情况调用 */
	UFUNCTION(BlueprintCallable, Category=Session)
	COMMONUSER_API virtual void CleanUpSessions();

	//////////////////////////////////////////////////////////////////////
	// Events

	/** 本地用户接受邀请时的原生委托 */
	FCommonSessionOnUserRequestedSession OnUserRequestedSessionEvent;
	/** 本地用户接受邀请时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On User Requested Session"))
	FCommonSessionOnUserRequestedSession_Dynamic K2_OnUserRequestedSessionEvent;

	/** JoinSession 调用完成时的原生委托 */
	FCommonSessionOnJoinSessionComplete OnJoinSessionCompleteEvent;
	/** JoinSession 调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Join Session Complete"))
	FCommonSessionOnJoinSessionComplete_Dynamic K2_OnJoinSessionCompleteEvent;

	/** CreateSession 调用完成时的原生委托 */
	FCommonSessionOnCreateSessionComplete OnCreateSessionCompleteEvent;
	/** CreateSession 调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Create Session Complete"))
	FCommonSessionOnCreateSessionComplete_Dynamic K2_OnCreateSessionCompleteEvent;

	/** 可呈现的会话信息发生变化时的原生委托 */
	FCommonSessionOnSessionInformationChanged OnSessionInformationChangedEvent;
	/** 可呈现的会话信息发生变化时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Session Information Changed"))
	FCommonSessionOnSessionInformationChanged_Dynamic K2_OnSessionInformationChangedEvent;

	/** 平台会话销毁被请求时的原生委托 */
	FCommonSessionOnDestroySessionRequested OnDestroySessionRequestedEvent;
	/** 平台会话销毁被请求时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Leave Session Requested"))
	FCommonSessionOnDestroySessionRequested_Dynamic K2_OnDestroySessionRequestedEvent;

	/** 用于在客户端前往之前修改连接 URL 的原生委托 */
	FCommonSessionOnPreClientTravel OnPreClientTravelEvent;

	// 配置设置，可在子类或配置文件中重写

	/** 设置会话搜索和主持请求中 bUseLobbies 的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesDefault = true;

	/** 设置会话主持请求中 bUseLobbiesVoiceChat 的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesVoiceChatDefault = false;

	/** 在创建或加入游戏会话时，启用服务器前往之前的预约信标流程 */ 
	UPROPERTY(Config)
	bool bUseBeacons = true;

protected:
	// 在创建或加入会话过程中调用的函数，可重写以提供游戏特定的行为

	/** 被调用以从快速游玩主持设置中填充会话请求，可重写以提供游戏特定的行为 */
	COMMONUSER_API virtual TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettings(UCommonSession_HostSessionRequest* Request, UCommonSession_SearchSessionRequest* QuickPlayRequest);

	/** 快速游玩搜索完成时调用，可重写以提供游戏特定的行为 */
	COMMONUSER_API virtual void HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UCommonSession_HostSessionRequest> HostRequest);

	/** 前往会话失败时调用 */
	COMMONUSER_API virtual void TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	/** 新会话创建成功或失败时调用 */
	COMMONUSER_API virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 被调用以完成会话创建 */
	COMMONUSER_API virtual void FinishSessionCreation(bool bWasSuccessful);

	/** 前往新主持的会话地图后调用 */
	COMMONUSER_API virtual void HandlePostLoadMap(UWorld* World);

protected:
	// 用于初始化和处理在线系统结果的内部函数

	COMMONUSER_API void BindOnlineDelegates();
	COMMONUSER_API void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	COMMONUSER_API void FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FCommonOnlineSearchSettings>& InSearchSettings);
	COMMONUSER_API void JoinSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	COMMONUSER_API void InternalTravelToSession(const FName SessionName);
	COMMONUSER_API void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	COMMONUSER_API void NotifyJoinSessionComplete(const FOnlineResultInformation& Result);
	COMMONUSER_API void NotifyCreateSessionComplete(const FOnlineResultInformation& Result);
	COMMONUSER_API void NotifySessionInformationUpdated(ECommonSessionInformationState SessionStatusStr, const FString& GameMode = FString(), const FString& MapName = FString());
	COMMONUSER_API void NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);
	COMMONUSER_API void SetCreateSessionError(const FText& ErrorText);

#if COMMONUSER_OSSV1
	COMMONUSER_API void BindOnlineDelegatesOSSv1();
	COMMONUSER_API void CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	COMMONUSER_API void FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer);
	COMMONUSER_API void JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	COMMONUSER_API TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv1(UCommonSession_HostSessionRequest* Request, UCommonSession_SearchSessionRequest* QuickPlayRequest);
	COMMONUSER_API void CleanUpSessionsOSSv1();

	COMMONUSER_API void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	COMMONUSER_API void HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult);
	COMMONUSER_API void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	COMMONUSER_API void OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	COMMONUSER_API void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	COMMONUSER_API void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	COMMONUSER_API void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	COMMONUSER_API void OnDestroySessionRequested(int32 LocalUserNum, FName SessionName);
	COMMONUSER_API void OnFindSessionsComplete(bool bWasSuccessful);
	COMMONUSER_API void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	COMMONUSER_API void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	COMMONUSER_API void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

#else
	COMMONUSER_API void BindOnlineDelegatesOSSv2();
	COMMONUSER_API void CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	COMMONUSER_API void FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer);
	COMMONUSER_API void JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	COMMONUSER_API TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv2(UCommonSession_HostSessionRequest* HostRequest, UCommonSession_SearchSessionRequest* SearchRequest);
	COMMONUSER_API void CleanUpSessionsOSSv2();

	/** 处理来自在线服务的加入请求 */
	COMMONUSER_API void OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams);

	/** 处理来自在线服务的 SESSION 加入请求 */
	COMMONUSER_API void OnSessionJoinRequested(const UE::Online::FUISessionJoinRequested& EventParams);

	/** 获取给定控制器的本地用户 ID */
	COMMONUSER_API UE::Online::FAccountId GetAccountId(APlayerController* PlayerController) const;
	/** 获取给定会话名的大厅 ID */
	COMMONUSER_API UE::Online::FLobbyId GetLobbyId(const FName SessionName) const;
	/** UI 大厅加入请求的事件句柄 */
	UE::Online::FOnlineEventDelegateHandle LobbyJoinRequestedHandle;

	/** UI 大厅会话请求的事件句柄 */
	UE::Online::FOnlineEventDelegateHandle SessionJoinRequestedHandle;

#endif // COMMONUSER_OSSV1

	COMMONUSER_API void CreateHostReservationBeacon();
	COMMONUSER_API void ConnectToHostReservationBeacon();
	COMMONUSER_API void DestroyHostReservationBeacon();

protected:
	/** 会话操作完成后将使用的前往 URL */
	FString PendingTravelURL;

	/** 最近一次会话创建尝试的结果信息，存储于此以便稍后存储错误码 */
	FOnlineResultInformation CreateSessionResult;

	/** 若想在会话创建后取消该会话则为 true */
	bool bWantToDestroyPendingSession = false;

	/** 若这是专用服务器则为 true，专用服务器创建会话不需要 LocalPlayer */
	bool bIsDedicatedServer = false;

	/** 当前搜索的设置 */
	TSharedPtr<FCommonOnlineSearchSettings> SearchSettings;

	/** 用于注册信标的通用信标监听器 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AOnlineBeaconHost> BeaconHostListener;
	/** 信标主机的状态 */
	UPROPERTY(Transient)
	TObjectPtr<UPartyBeaconState> ReservationBeaconHostState;
	/** 控制此游戏访问权限的信标。 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconHost> ReservationBeaconHost;
	/** 用于信标通信的通用类对象 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconClient> ReservationBeaconClient;

	/** 信标预约的队伍数量 */
	UPROPERTY(Config)
	int32 BeaconTeamCount = 2;
	/** 信标预约的队伍规模 */
	UPROPERTY(Config)
	int32 BeaconTeamSize = 8;
	/** 信标预约的最大数量 */
	UPROPERTY(Config)
	int32 BeaconMaxReservations = 16;
};
