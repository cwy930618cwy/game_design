// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonSessionSubsystem.h"
#include "AssetRegistry/AssetData.h"
#include "CommonUserTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineBeaconHost.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonSessionSubsystem)

#if COMMONUSER_OSSV1
#include "Engine/World.h"
#include "OnlineSubsystemUtils.h"

FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv1"));
#else
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Online/OnlineServicesEngineUtils.h"

FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv2"));
using namespace UE::Online;
#endif // COMMONUSER_OSSV1


DECLARE_LOG_CATEGORY_EXTERN(LogCommonSession, Log, All);
DEFINE_LOG_CATEGORY(LogCommonSession);

#define LOCTEXT_NAMESPACE "CommonUser"

//////////////////////////////////////////////////////////////////////
//UCommonSession_SearchSessionRequest

void UCommonSession_SearchSessionRequest::NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage)
{
	OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
	K2_OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
}


//////////////////////////////////////////////////////////////////////
//UCommonSession_SearchResult

#if COMMONUSER_OSSV1
FString UCommonSession_SearchResult::GetDescription() const
{
	return Result.GetSessionIdStr();
}

void UCommonSession_SearchResult::GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const
{
	bFoundValue = Result.Session.SessionSettings.Get<FString>(Key, /*out*/ Value);
}

void UCommonSession_SearchResult::GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const
{
	bFoundValue = Result.Session.SessionSettings.Get<int32>(Key, /*out*/ Value);
}

int32 UCommonSession_SearchResult::GetNumOpenPrivateConnections() const
{
	return Result.Session.NumOpenPrivateConnections;
}

int32 UCommonSession_SearchResult::GetNumOpenPublicConnections() const
{
	return Result.Session.NumOpenPublicConnections;
}

int32 UCommonSession_SearchResult::GetMaxPublicConnections() const
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

int32 UCommonSession_SearchResult::GetPingInMs() const
{
	return Result.PingInMs;
}
#else
FString UCommonSession_SearchResult::GetDescription() const
{
	return ToLogString(Lobby->LobbyId);
}

void UCommonSession_SearchResult::GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const
{
	if (const FSchemaVariant* VariantValue = Lobby->Attributes.Find(Key))
	{
		bFoundValue = true;
		Value = VariantValue->GetString();
	}
	else
	{
		bFoundValue = false;
	}
}

void UCommonSession_SearchResult::GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const
{
	if (const FSchemaVariant* VariantValue = Lobby->Attributes.Find(Key))
	{
		bFoundValue = true;
		Value = (int32)VariantValue->GetInt64();
	}
	else
	{
		bFoundValue = false;
	}
}

int32 UCommonSession_SearchResult::GetNumOpenPrivateConnections() const
{
	// TODO: 私有连接
	return 0;
}

int32 UCommonSession_SearchResult::GetNumOpenPublicConnections() const
{
	return Lobby->MaxMembers - Lobby->Members.Num();
}

int32 UCommonSession_SearchResult::GetMaxPublicConnections() const
{
	return Lobby->MaxMembers;
}

int32 UCommonSession_SearchResult::GetPingInMs() const
{
	// TODO: 这不是大厅（lobby）的属性。需要配合会话（session）来实现。
	return 0;
}
#endif //COMMONUSER_OSSV1


class FCommonOnlineSearchSettingsBase : public FGCObject
{
public:
	FCommonOnlineSearchSettingsBase(UCommonSession_SearchSessionRequest* InSearchRequest)
	{
		SearchRequest = InSearchRequest;
	}

	virtual ~FCommonOnlineSearchSettingsBase() {}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SearchRequest);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FCommonOnlineSearchSettings");
		return NameString;
	}

public:
	TObjectPtr<UCommonSession_SearchSessionRequest> SearchRequest = nullptr;
};

#if COMMONUSER_OSSV1
//////////////////////////////////////////////////////////////////////
// FCommonSession_OnlineSessionSettings

class FCommonSession_OnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FCommonSession_OnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4)
	{
		NumPublicConnections = MaxNumPlayers;
		if (NumPublicConnections < 0)
		{
			NumPublicConnections = 0;
		}
		NumPrivateConnections = 0;
		bIsLANMatch = bIsLAN;
		bShouldAdvertise = true;
		bAllowJoinInProgress = true;
		bAllowInvites = true;
		bUsesPresence = bIsPresence;
		bAllowJoinViaPresence = true;
		bAllowJoinViaPresenceFriendsOnly = false;
	}

	virtual ~FCommonSession_OnlineSessionSettings() {}
};

//////////////////////////////////////////////////////////////////////
// FCommonOnlineSearchSettingsOSSv1

class FCommonOnlineSearchSettingsOSSv1 : public FOnlineSessionSearch, public FCommonOnlineSearchSettingsBase
{
public:
	FCommonOnlineSearchSettingsOSSv1(UCommonSession_SearchSessionRequest* InSearchRequest)
		: FCommonOnlineSearchSettingsBase(InSearchRequest)
	{
		bIsLanQuery = (InSearchRequest->OnlineMode == ECommonSessionOnlineMode::LAN);
		MaxSearchResults = 10;
		PingBucketSize = 50;

		QuerySettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineComparisonOp::Equals);

		if (InSearchRequest->bUseLobbies)
		{
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FCommonOnlineSearchSettingsOSSv1() {}
};
#else

class FCommonOnlineSearchSettingsOSSv2 : public FCommonOnlineSearchSettingsBase
{
public:
	FCommonOnlineSearchSettingsOSSv2(UCommonSession_SearchSessionRequest* InSearchRequest)
		: FCommonOnlineSearchSettingsBase(InSearchRequest)
	{
		FindLobbyParams.MaxResults = 10;

		FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{ SETTING_ONLINESUBSYSTEM_VERSION, ESchemaAttributeComparisonOp::Equals, true });
	}
public:
	FFindLobbies::Params FindLobbyParams;
};

#endif // COMMONUSER_OSSV1

//////////////////////////////////////////////////////////////////////
// UCommonSession_HostSessionRequest

FString UCommonSession_HostSessionRequest::GetMapName() const
{
	FAssetData MapAssetData;
	if (UAssetManager::Get().GetPrimaryAssetData(MapID, /*out*/ MapAssetData))
	{
		return MapAssetData.PackageName.ToString();
	}
	else
	{
		return FString();
	}
}

FString UCommonSession_HostSessionRequest::ConstructTravelURL() const
{
	FString CombinedExtraArgs;

	if (OnlineMode == ECommonSessionOnlineMode::LAN)
	{
		CombinedExtraArgs += TEXT("?bIsLanMatch");
	}
	
	if (OnlineMode != ECommonSessionOnlineMode::Offline)
	{
		CombinedExtraArgs += TEXT("?listen");
	}

	for (const auto& KVP : ExtraArgs)
	{
		if (!KVP.Key.IsEmpty())
		{
			if (KVP.Value.IsEmpty())
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s"), *KVP.Key);
			}
			else
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s=%s"), *KVP.Key, *KVP.Value);
			}
		}
	}

	//bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));

	return FString::Printf(TEXT("%s%s"),
		*GetMapName(),
		*CombinedExtraArgs);
}

bool UCommonSession_HostSessionRequest::ValidateAndLogErrors(FText& OutError) const
{
#if WITH_SERVER_CODE
	if (GetMapName().IsEmpty())
	{
		OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed."), FText::FromString(MapID.ToString()));
		return false;
	}

	return true;
#else
	// 客户端构建只用于连接专用服务器，默认缺少主持会话所需的代码
	// 你可以在子类中修改此行为，例如处理教程之类的场景
	OutError = NSLOCTEXT("NetworkErrors", "ClientBuildCannotHost", "Client builds cannot host game sessions.");
	return false;
#endif
}

int32 UCommonSession_HostSessionRequest::GetMaxPlayers() const
{
	return MaxPlayerCount;
}

//////////////////////////////////////////////////////////////////////
// UCommonSessionSubsystem

void UCommonSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
	GEngine->OnTravelFailure().AddUObject(this, &UCommonSessionSubsystem::TravelLocalSessionFailure);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UCommonSessionSubsystem::HandlePostLoadMap);

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UCommonSessionSubsystem::BindOnlineDelegates()
{
#if COMMONUSER_OSSV1
	BindOnlineDelegatesOSSv1();
#else
	BindOnlineDelegatesOSSv2();
#endif
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::BindOnlineDelegatesOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));
	SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionComplete));
	SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionComplete));
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
	SessionInterface->AddOnDestroySessionRequestedDelegate_Handle(FOnDestroySessionRequestedDelegate::CreateUObject(this, &ThisClass::OnDestroySessionRequested));

//	SessionInterface->AddOnMatchmakingCompleteDelegate_Handle(FOnMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnMatchmakingComplete));
//	SessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelMatchmakingComplete));

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));
// 	SessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelFindSessionsComplete));
// 	SessionInterface->AddOnPingSearchResultsCompleteDelegate_Handle(FOnPingSearchResultsCompleteDelegate::CreateUObject(this, &ThisClass::OnPingSearchResultsComplete));
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

//	TWO_PARAM(OnSessionParticipantJoined, FName, const FUniqueNetId&);
//	THREE_PARAM(OnSessionParticipantLeft, FName, const FUniqueNetId&, EOnSessionParticipantLeftReason);
//	ONE_PARAM(OnQosDataRequested, FName);
//	TWO_PARAM(OnSessionCustomDataChanged, FName, const FOnlineSessionSettings&);
//	TWO_PARAM(OnSessionSettingsUpdated, FName, const FOnlineSessionSettings&);
//	THREE_PARAM(OnSessionParticipantSettingsUpdated, FName, const FUniqueNetId&, const FOnlineSessionSettings&);
//	FOUR_PARAM(OnSessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FString& /*AppId*/, const FOnlineSessionSearchResult& /*InviteResult*/);
//	THREE_PARAM(OnRegisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);
//	THREE_PARAM(OnUnregisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);

	SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::HandleSessionUserInviteAccepted));
	SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &ThisClass::HandleSessionFailure));
}

#else

void UCommonSessionSubsystem::BindOnlineDelegatesOSSv2()
{
	// TODO: 绑定 OSSv2 委托，等它们可用时再处理
	// 注意，上面的大多数 OSSv1 委托在 OSSv2 中是以完成委托的形式实现的，无需订阅
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
	if (ensure(Lobbies))
	{
		LobbyJoinRequestedHandle = Lobbies->OnUILobbyJoinRequested().Add(this, &UCommonSessionSubsystem::OnLobbyJoinRequested);
	}

	ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
	if (ensure(Sessions))
	{
		SessionJoinRequestedHandle = Sessions->OnUISessionJoinRequested().Add(this, &UCommonSessionSubsystem::OnSessionJoinRequested);
	}
}
#endif

void UCommonSessionSubsystem::Deinitialize()
{
#if COMMONUSER_OSSV1
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());

	if (OnlineSub)
	{
		// 在关闭过程中这可能无效
		const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface)
		{
			SessionInterface->ClearOnSessionFailureDelegates(this);
		}
	}
#endif // COMMONUSER_OSSV1

	if (GEngine)
	{
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

bool UCommonSessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 仅当不存在游戏特定子类时才创建实例
	return ChildClasses.Num() == 0;
}

UCommonSession_HostSessionRequest* UCommonSessionSubsystem::CreateOnlineHostSessionRequest()
{
	/** 游戏特定子系统可重写此函数，也可在创建后修改 */

	UCommonSession_HostSessionRequest* NewRequest = NewObject<UCommonSession_HostSessionRequest>(this);
	NewRequest->OnlineMode = ECommonSessionOnlineMode::Online;
	NewRequest->bUseLobbies = bUseLobbiesDefault;
	NewRequest->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;

	// 我们默认在用于匹配的主会话中启用在线状态（presence）。对于在意在线状态的在线系统，只有主会话才应启用在线状态
	NewRequest->bUsePresence = !IsRunningDedicatedServer();

	return NewRequest;
}

UCommonSession_SearchSessionRequest* UCommonSessionSubsystem::CreateOnlineSearchSessionRequest()
{
	/** 游戏特定子系统可重写此函数，也可在创建后修改 */

	UCommonSession_SearchSessionRequest* NewRequest = NewObject<UCommonSession_SearchSessionRequest>(this);
	NewRequest->OnlineMode = ECommonSessionOnlineMode::Online;

	NewRequest->bUseLobbies = bUseLobbiesDefault;

	return NewRequest;
}

void UCommonSessionSubsystem::HostSession(APlayerController* HostingPlayer, UCommonSession_HostSessionRequest* Request)
{
	if (Request == nullptr)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidRequest", "HostSession passed an invalid request."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	ULocalPlayer* LocalPlayer = (HostingPlayer != nullptr) ? HostingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr && !bIsDedicatedServer)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidHostingPlayer", "HostingPlayer is invalid."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	FText OutError;
	if (!Request->ValidateAndLogErrors(OutError))
	{
		SetCreateSessionError(OutError);
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	if (Request->OnlineMode == ECommonSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			SetCreateSessionError(NSLOCTEXT("NetworkErrors", "CannotHostAsClient", "Cannot host offline game as client."));
			OnCreateSessionComplete(NAME_None, false);
			return;
		}
		else
		{
			// 离线，所以立即前往指定的匹配 URL
			GetWorld()->ServerTravel(Request->ConstructTravelURL());
		}
	}
	else
	{
		CreateOnlineSessionInternal(LocalPlayer, Request);
	}

	NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
}

void UCommonSessionSubsystem::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request)
{
	CreateSessionResult = FOnlineResultInformation();
	PendingTravelURL = Request->ConstructTravelURL();

#if COMMONUSER_OSSV1
	CreateOnlineSessionInternalOSSv1(LocalPlayer, Request);
#else
	CreateOnlineSessionInternalOSSv2(LocalPlayer, Request);
#endif
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request)
{
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	FUniqueNetIdPtr UserId;
	if (LocalPlayer)
	{
		UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	}
	else if (bIsDedicatedServer)
	{
		UserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(DEDICATED_SERVER_USER_INDEX);
	}

	//@TODO: 在某些平台上尝试进行 LAN 会话时可能会走到这里，这需要有效的用户 ID 吗？
	if (ensure(UserId.IsValid()))
	{
		FCommonSession_OnlineSessionSettings HostSettings(Request->OnlineMode == ECommonSessionOnlineMode::LAN, Request->bUsePresence, MaxPlayers);
		HostSettings.bUseLobbiesIfAvailable = Request->bUseLobbies;
		HostSettings.bUseLobbiesVoiceChatIfAvailable = Request->bUseLobbiesVoiceChat;
		HostSettings.Set(SETTING_GAMEMODE, Request->ModeNameForAdvertisement, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_MAPNAME, Request->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		//@TODO: HostSettings.Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
		HostSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")), EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineDataAdvertisementType::ViaOnlineService);

		Sessions->CreateSession(*UserId, SessionName, HostSettings);
		NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
	}
	else
	{
		OnCreateSessionComplete(SessionName, false);
	}
}

#else

void UCommonSessionSubsystem::CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request)
{
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());

	check(OnlineServices);

	FString ModeName = Request->ModeNameForAdvertisement;
	FString MapName = Request->GetMapName();

	if(!Request->bUseLobbies)
	{
		ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
		check(Sessions);
		FCreateSession::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		else if (bIsDedicatedServer)
		{
			// TODO 对 v2 来说这里应该做什么？
		}

		CreateParams.SessionName = SessionName;
		CreateParams.bPresenceEnabled = Request->bUsePresence;
		CreateParams.SessionSettings.SchemaName = FSchemaId(TEXT("GameLobby")); // TODO: make a parameter
		CreateParams.SessionSettings.NumMaxConnections = MaxPlayers;
		CreateParams.SessionSettings.JoinPolicy = ESessionJoinPolicy::Public; // TODO: Check parameters

		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_GAMEMODE, Request->ModeNameForAdvertisement);
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_MAPNAME, Request->GetMapName());
		//@TODO: CreateParams.CustomSettings.Emplace(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"));
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_MATCHING_TIMEOUT, 120.0f);
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")));
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_ONLINESUBSYSTEM_VERSION, true);

		Sessions->CreateSession(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName, ModeName, MapName](const TOnlineResult<FCreateSession>& CreateResult)
			{
				OnCreateSessionComplete(SessionName, CreateResult.IsOk());
				NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, ModeName, MapName);
			});
	}
	else
	{
		ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		check(Lobbies);
		FCreateLobby::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		else if (bIsDedicatedServer)
		{
			// TODO 对 v2 来说这里应该做什么？
		}

		CreateParams.LocalName = SessionName;
		CreateParams.SchemaId = FSchemaId(TEXT("GameLobby")); // TODO: make a parameter
		CreateParams.bPresenceEnabled = Request->bUsePresence;
		CreateParams.MaxMembers = MaxPlayers;
		CreateParams.JoinPolicy = ELobbyJoinPolicy::PublicAdvertised; // TODO: Check parameters

		CreateParams.Attributes.Emplace(SETTING_GAMEMODE, Request->ModeNameForAdvertisement);
		CreateParams.Attributes.Emplace(SETTING_MAPNAME, Request->GetMapName());
		//@TODO: CreateParams.Attributes.Emplace(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"));
		CreateParams.Attributes.Emplace(SETTING_MATCHING_TIMEOUT, 120.0f);
		CreateParams.Attributes.Emplace(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")));
		CreateParams.Attributes.Emplace(SETTING_ONLINESUBSYSTEM_VERSION, true);

		CreateParams.UserAttributes.Emplace(SETTING_GAMEMODE, FString(TEXT("GameSession")));

		// TODO: 添加分屏玩家

		Lobbies->CreateLobby(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName, ModeName, MapName](const TOnlineResult<FCreateLobby>& CreateResult)
			{
				OnCreateSessionComplete(SessionName, CreateResult.IsOk());
				NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, ModeName, MapName);
			});
	}
}

#endif

void UCommonSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnCreateSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

#if COMMONUSER_OSSV1 // OSSv2 joins splitscreen players as part of the create call
	// 添加分屏玩家（如果存在）
#if 0 //@TODO:
	if (bWasSuccessful && LocalPlayers.Num() > 1)
	{
		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnRegisterLocalPlayerComplete_CreateSession));
		}
	}
	else
#endif
#endif
	{
		// 要么失败了，要么只有一个本地用户
		FinishSessionCreation(bWasSuccessful);
	}
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishSessionCreation(Result == EOnJoinSessionCompleteResult::Success);
}

void UCommonSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnStartSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

	if (bWantToDestroyPendingSession)
	{
		CleanUpSessions();
	}
}
#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::FinishSessionCreation(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//@TODO 将此处的时机与 join 回调同步，如果计划有变，两处以及注释都要一起修改
		CreateSessionResult = FOnlineResultInformation();
		CreateSessionResult.bWasSuccessful = true;

		if (bUseBeacons)
		{
			CreateHostReservationBeacon();
		}

		NotifyCreateSessionComplete(CreateSessionResult);

		// 前往指定的匹配 URL
		GetWorld()->ServerTravel(PendingTravelURL);
	}
	else
	{
		if (CreateSessionResult.bWasSuccessful || CreateSessionResult.ErrorText.IsEmpty())
		{
			FString ReturnError = TEXT("GenericFailure"); // TODO: No good way to get session error codes out of OSSV1
			FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");

			CreateSessionResult.bWasSuccessful = false;
			CreateSessionResult.ErrorId = ReturnError;
			CreateSessionResult.ErrorText = ReturnReason;
		}

		UE_LOG(LogCommonSession, Error, TEXT("FinishSessionCreation(%s): %s"), *CreateSessionResult.ErrorId, *CreateSessionResult.ErrorText.ToString());

		NotifyCreateSessionComplete(CreateSessionResult);
		NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);
	}
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnUpdateSessionComplete(SessionName: %s, bWasSuccessful: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
}

void UCommonSessionSubsystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnEndSessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	CleanUpSessions();
}

void UCommonSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnDestroySessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	bWantToDestroyPendingSession = false;
}

void UCommonSessionSubsystem::OnDestroySessionRequested(int32 LocalUserNum, FName SessionName)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserNum);

	NotifyDestroySessionRequested(PlatformUserId, SessionName);
}
#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::FindSessions(APlayerController* SearchingPlayer, UCommonSession_SearchSessionRequest* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogCommonSession, Error, TEXT("FindSessions passed a null request"));
		return;
	}

#if COMMONUSER_OSSV1
	FindSessionsInternal(SearchingPlayer, MakeShared<FCommonOnlineSearchSettingsOSSv1>(Request));
#else
	FindSessionsInternal(SearchingPlayer, MakeShared<FCommonOnlineSearchSettingsOSSv2>(Request));
#endif // COMMONUSER_OSSV1
}

void UCommonSessionSubsystem::FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FCommonOnlineSearchSettings>& InSearchSettings)
{
	if (SearchSettings.IsValid())
	{
		//@TODO: 这对 API 用户来说体验很差，我们应该让额外的搜索搭便车，
		// 直接给它与当前挂起请求相同的结果
		// （或者把请求排队，等前一个完成或失败后再处理）
		UE_LOG(LogCommonSession, Error, TEXT("A previous FindSessions call is still in progress, aborting"));
		SearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionAlreadyInProgress", "Session search already in progress"));
	}

	ULocalPlayer* LocalPlayer = (SearchingPlayer != nullptr) ? SearchingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogCommonSession, Error, TEXT("SearchingPlayer is invalid"));
		InSearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionBadPlayer", "Session search was not provided a local player"));
		return;
	}

	SearchSettings = InSearchSettings;
#if COMMONUSER_OSSV1
	FindSessionsInternalOSSv1(LocalPlayer);
#else
	FindSessionsInternalOSSv2(LocalPlayer);
#endif
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	SearchSettings->QuerySettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineComparisonOp::Equals);

	if (!Sessions->FindSessions(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), StaticCastSharedRef<FCommonOnlineSearchSettingsOSSv1>(SearchSettings.ToSharedRef())))
	{
		// 某些会话搜索失败会在函数内部调用这个委托，其他情况则不会
		OnFindSessionsComplete(false);
	}
}

#else

void UCommonSessionSubsystem::FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer)
{
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
	check(Lobbies);

	FFindLobbies::Params FindLobbyParams = StaticCastSharedPtr<FCommonOnlineSearchSettingsOSSv2>(SearchSettings)->FindLobbyParams;
	FindLobbyParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();

	Lobbies->FindLobbies(MoveTemp(FindLobbyParams)).OnComplete(this, [this, LocalSearchSettings = SearchSettings](const TOnlineResult<FFindLobbies>& FindResult)
	{
		if (LocalSearchSettings != SearchSettings)
		{
			// 这是一个被放弃的搜索，忽略
			return;
		}
		const bool bWasSuccessful = FindResult.IsOk();
		UE_LOG(LogCommonSession, Log, TEXT("FindLobbies(bWasSuccessful: %s)"), *LexToString(bWasSuccessful));
		check(SearchSettings.IsValid());
		if (bWasSuccessful)
		{
			const FFindLobbies::Result& FindResults = FindResult.GetOkValue();
			SearchSettings->SearchRequest->Results.Reset(FindResults.Lobbies.Num());

			for (const TSharedRef<const FLobby>& Lobby : FindResults.Lobbies)
			{
				if (!Lobby->OwnerAccountId.IsValid())
				{
					UE_LOG(LogCommonSession, Verbose, TEXT("\tIgnoring Lobby with no owner (LobbyId: %s)"),
						*ToLogString(Lobby->LobbyId));
				}
				else if (Lobby->Members.Num() == 0)
				{
					UE_LOG(LogCommonSession, Verbose, TEXT("\tIgnoring Lobby with no members (UserId: %s)"),
						*ToLogString(Lobby->OwnerAccountId));
				}
				else
				{
					UCommonSession_SearchResult* Entry = NewObject<UCommonSession_SearchResult>(SearchSettings->SearchRequest);
					Entry->Lobby = Lobby;
					SearchSettings->SearchRequest->Results.Add(Entry);

					UE_LOG(LogCommonSession, Log, TEXT("\tFound lobby (UserId: %s, NumOpenConns: %d)"),
						*ToLogString(Lobby->OwnerAccountId), Lobby->MaxMembers - Lobby->Members.Num());
				}
			}
		}
		else
		{
			SearchSettings->SearchRequest->Results.Empty();
		}

		const FText ResultText = bWasSuccessful ? FText() : FindResult.GetErrorValue().GetText();

		SearchSettings->SearchRequest->NotifySearchFinished(bWasSuccessful, ResultText);
		SearchSettings.Reset();
	});
}
#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UCommonSession_HostSessionRequest* HostRequest)
{
	UE_LOG(LogCommonSession, Log, TEXT("QuickPlay Requested"));

	if (HostRequest == nullptr)
	{
		UE_LOG(LogCommonSession, Error, TEXT("QuickPlaySession passed a null request"));
		return;
	}

	TStrongObjectPtr<UCommonSession_HostSessionRequest> HostRequestPtr = TStrongObjectPtr<UCommonSession_HostSessionRequest>(HostRequest);
	TWeakObjectPtr<APlayerController> JoiningOrHostingPlayerPtr = TWeakObjectPtr<APlayerController>(JoiningOrHostingPlayer);

	UCommonSession_SearchSessionRequest* QuickPlayRequest = CreateOnlineSearchSessionRequest();
	QuickPlayRequest->OnSearchFinished.AddUObject(this, &UCommonSessionSubsystem::HandleQuickPlaySearchFinished, JoiningOrHostingPlayerPtr, HostRequestPtr);

	// 我们默认在用于匹配的主会话上启用在线状态（presence）。对于在意在线状态的在线系统，只有主会话才应启用在线状态

	HostRequestPtr->bUseLobbies = bUseLobbiesDefault;
	HostRequestPtr->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;
	HostRequestPtr->bUsePresence = true;
	QuickPlayRequest->bUseLobbies = bUseLobbiesDefault;

	NotifySessionInformationUpdated(ECommonSessionInformationState::Matchmaking);
	FindSessionsInternal(JoiningOrHostingPlayer, CreateQuickPlaySearchSettings(HostRequest, QuickPlayRequest));
}

TSharedRef<FCommonOnlineSearchSettings> UCommonSessionSubsystem::CreateQuickPlaySearchSettings(UCommonSession_HostSessionRequest* HostRequest, UCommonSession_SearchSessionRequest* SearchRequest)
{
#if COMMONUSER_OSSV1
	return CreateQuickPlaySearchSettingsOSSv1(HostRequest, SearchRequest);
#else
	return CreateQuickPlaySearchSettingsOSSv2(HostRequest, SearchRequest);
#endif // COMMONUSER_OSSV1
}

#if COMMONUSER_OSSV1
TSharedRef<FCommonOnlineSearchSettings> UCommonSessionSubsystem::CreateQuickPlaySearchSettingsOSSv1(UCommonSession_HostSessionRequest* HostRequest, UCommonSession_SearchSessionRequest* SearchRequest)
{
	TSharedRef<FCommonOnlineSearchSettingsOSSv1> QuickPlaySearch = MakeShared<FCommonOnlineSearchSettingsOSSv1>(SearchRequest);

	/** 默认情况下快速游玩（quick play）不希望包含地图或游戏模式，游戏可按需自行填充
	if (!HostRequest->ModeNameForAdvertisement.IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_GAMEMODE, HostRequest->ModeNameForAdvertisement, EOnlineComparisonOp::Equals);
	}

	if (!HostRequest->GetMapName().IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_MAPNAME, HostRequest->GetMapName(), EOnlineComparisonOp::Equals);
	} 
	*/

	// QuickPlaySearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	return QuickPlaySearch;
}

#else

TSharedRef<FCommonOnlineSearchSettings> UCommonSessionSubsystem::CreateQuickPlaySearchSettingsOSSv2(UCommonSession_HostSessionRequest* HostRequest, UCommonSession_SearchSessionRequest* SearchRequest)
{
	TSharedRef<FCommonOnlineSearchSettingsOSSv2> QuickPlaySearch = MakeShared<FCommonOnlineSearchSettingsOSSv2>(SearchRequest);

	/** 默认情况下快速游玩（quick play）不希望包含地图或游戏模式，游戏可按需自行填充
	if (!HostRequest->ModeNameForAdvertisement.IsEmpty())
	{
		QuickPlaySearch->FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{SETTING_GAMEMODE, ESchemaAttributeComparisonOp::Equals, HostRequest->ModeNameForAdvertisement});
	}
	if (!HostRequest->GetMapName().IsEmpty())
	{
		QuickPlaySearch->FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{SETTING_MAPNAME, ESchemaAttributeComparisonOp::Equals, HostRequest->GetMapName()});
	}
	*/

	return QuickPlaySearch;
}

#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UCommonSession_HostSessionRequest> HostRequest)
{
	const int32 ResultCount = SearchSettings->SearchRequest->Results.Num();
	UE_LOG(LogCommonSession, Log, TEXT("QuickPlay Search Finished %s (Results %d) (Error: %s)"), bSucceeded ? TEXT("Success") : TEXT("Failed"), ResultCount, *ErrorMessage.ToString());

	//@TODO: 我们必须检查错误消息是否为空，因为某些 OSS 层仅仅因为没有会话就报告失败。请配合 OSS 2.0 修复。
	if (bSucceeded || ErrorMessage.IsEmpty())
	{
		// 加入最佳的搜索结果。
		if (ResultCount > 0)
		{
			//@TODO: 我们或许应该看看 ping？也许还要考虑其他因素来找最佳的。不确定它们是否已经预先排好序。
			for (UCommonSession_SearchResult* Result : SearchSettings->SearchRequest->Results)
			{
				JoinSession(JoiningOrHostingPlayer.Get(), Result);
				return;
			}
		}
		else
		{
			HostSession(JoiningOrHostingPlayer.Get(), HostRequest.Get());
		}
	}
	else
	{
		//@TODO: 这太糟糕了，需要通知某人。
		NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);
	}
}

void UCommonSessionSubsystem::CleanUpSessions()
{
	bWantToDestroyPendingSession = true;

	if (bUseBeacons)
	{
		DestroyHostReservationBeacon();
	}

	NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);
#if COMMONUSER_OSSV1
	CleanUpSessionsOSSv1();
#else
	CleanUpSessionsOSSv2();
#endif // COMMONUSER_OSSV1
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::CleanUpSessionsOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	EOnlineSessionState::Type SessionState = Sessions->GetSessionState(NAME_GameSession);
	UE_LOG(LogCommonSession, Log, TEXT("Session state is %s"), EOnlineSessionState::ToString(SessionState));

	if (EOnlineSessionState::InProgress == SessionState)
	{
		UE_LOG(LogCommonSession, Log, TEXT("Ending session because of return to front end"));
		Sessions->EndSession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Ending == SessionState)
	{
		UE_LOG(LogCommonSession, Log, TEXT("Waiting for session to end on return to main menu"));
	}
	else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
	{
		UE_LOG(LogCommonSession, Log, TEXT("Destroying session on return to main menu"));
		Sessions->DestroySession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Starting == SessionState || EOnlineSessionState::Creating == SessionState)
	{
		UE_LOG(LogCommonSession, Log, TEXT("Waiting for session to start, and then we will end it to return to main menu"));
	}
}

#else
void UCommonSessionSubsystem::CleanUpSessionsOSSv2()
{
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();	
	ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
	
	FAccountId LocalPlayerId = GetAccountId(GetGameInstance()->GetFirstLocalPlayerController());
	
	if (!LocalPlayerId.IsValid())
	{
		return;
	}

	if (bUseLobbiesDefault)
	{
		FLobbyId LobbyId = GetLobbyId(NAME_GameSession);

		if (!LobbyId.IsValid())
		{
			return;
		}
		// TODO: 让所有本地玩家都离开大厅/会话
		if (ensure(Lobbies))
		{
			Lobbies->LeaveLobby({ LocalPlayerId, LobbyId });
		}
	}
	if (ensure(Sessions))
	{
		Sessions->LeaveSession({ LocalPlayerId, NAME_GameSession, false });
	}
}

#endif // COMMONUSER_OSSV1

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogCommonSession, Log, TEXT("OnFindSessionsComplete(bWasSuccessful: %s)"), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!SearchSettings.IsValid())
	{
		// 会话搜索失败时，或由不同系统发起的搜索，这里可能会被调用两次
		return;
	}

	FCommonOnlineSearchSettingsOSSv1& SearchSettingsV1 = *StaticCastSharedPtr<FCommonOnlineSearchSettingsOSSv1>(SearchSettings);
	if (SearchSettingsV1.SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG(LogCommonSession, Error, TEXT("OnFindSessionsComplete called when search is still in progress!"));
		return;
	}

	if (!ensure(SearchSettingsV1.SearchRequest))
	{
		UE_LOG(LogCommonSession, Error, TEXT("OnFindSessionsComplete called with invalid search request object!"));
		return;
	}

	if (bWasSuccessful)
	{
		SearchSettingsV1.SearchRequest->Results.Reset(SearchSettingsV1.SearchResults.Num());

		for (const FOnlineSessionSearchResult& Result : SearchSettingsV1.SearchResults)
		{
			check(Result.IsValid());

			UCommonSession_SearchResult* Entry = NewObject<UCommonSession_SearchResult>(SearchSettingsV1.SearchRequest);
			Entry->Result = Result;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);

			FString SessionId = TEXT("Unknown");
			if (Result.Session.SessionInfo.IsValid())
			{
				SessionId = Result.Session.SessionInfo->GetSessionId().ToString();
			}

			FString OwningUserId = TEXT("Unknown");
			if (Result.Session.OwningUserId.IsValid())
			{
				OwningUserId = Result.Session.OwningUserId->ToString();
			}

			UE_LOG(LogCommonSession, Log, TEXT("\tFound session (SessionId: %s, UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
				*SessionId,
				*OwningUserId,
				*Result.Session.OwningUserName,
				Result.Session.NumOpenPrivateConnections,
				Result.Session.NumOpenPublicConnections,
				Result.PingInMs
				);
		}
	}
	else
	{
		SearchSettingsV1.SearchRequest->Results.Empty();
	}

	if (0)
	{
		// 模拟会话 OSSV1
		for (int i = 0; i < 10; i++)
		{
			UCommonSession_SearchResult* Entry = NewObject<UCommonSession_SearchResult>(SearchSettings->SearchRequest);
			FOnlineSessionSearchResult FakeResult;
			FakeResult.Session.OwningUserName = TEXT("Fake User");
			FakeResult.Session.SessionSettings.NumPublicConnections = 10;
			FakeResult.Session.SessionSettings.bShouldAdvertise = true;
			FakeResult.Session.SessionSettings.bAllowJoinInProgress = true;
			FakeResult.PingInMs=99;
			Entry->Result = FakeResult;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);
		}
	}
	
	SearchSettingsV1.SearchRequest->NotifySearchFinished(bWasSuccessful, bWasSuccessful ? FText() : LOCTEXT("Error_FindSessionV1Failed", "Find session failed"));
	SearchSettings.Reset();
}
#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::JoinSession(APlayerController* JoiningPlayer, UCommonSession_SearchResult* Request)
{

	if (Request == nullptr)
	{
		UE_LOG(LogCommonSession, Error, TEXT("JoinSession passed a null request"));
		return;
	}

	ULocalPlayer* LocalPlayer = (JoiningPlayer != nullptr) ? JoiningPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogCommonSession, Error, TEXT("JoiningPlayer is invalid"));
		return;
	}

	// 在这里更新在线状态，因为客户端前往之后就没有原始的游戏模式和地图名键了。如果加入/前往失败，会重置回主菜单 
	FString SessionGameMode, SessionMapName;
	bool bEmpty;
#if COMMONUSER_OSSV1
	Request->GetStringSetting(SETTING_GAMEMODE, SessionGameMode, bEmpty);
	Request->GetStringSetting(SETTING_MAPNAME, SessionMapName, bEmpty);
	NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, SessionGameMode, SessionMapName);
#else
	if(Request->Lobby.IsValid())
	{
		Request->GetStringSetting(SETTING_GAMEMODE, SessionGameMode, bEmpty);
		Request->GetStringSetting(SETTING_MAPNAME, SessionMapName, bEmpty);
		NotifySessionInformationUpdated(ECommonSessionInformationState::InGame, SessionGameMode, SessionMapName);
	}
#endif // COMMONUSER_OSSV1

	JoinSessionInternal(LocalPlayer, Request);
}

void UCommonSessionSubsystem::JoinSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request)
{
#if COMMONUSER_OSSV1
	JoinSessionInternalOSSv1(LocalPlayer, Request);
#else
	JoinSessionInternalOSSv2(LocalPlayer, Request);
#endif // COMMONUSER_OSSV1
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);
	
	// We need to manually set that we want this to be our presence session
	Request->Result.Session.SessionSettings.bUsesPresence = true;
	Request->Result.Session.SessionSettings.bUseLobbiesIfAvailable = bUseLobbiesDefault;

	Sessions->JoinSession(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, Request->Result);
}

void UCommonSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// 添加分屏玩家（如果存在）
	//@TODO:
// 	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
// 	{
// 		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
// 		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
// 		{
// 			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
// 				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UShooterGameInstance::OnRegisterJoiningLocalPlayerComplete));
// 		}
// 	}
// 	else
 	{
		FinishJoinSession(Result);
	}
}

void UCommonSessionSubsystem::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void UCommonSessionSubsystem::ConnectToHostReservationBeacon()
{
	UWorld* const World = GetWorld();
	check(World);
	ReservationBeaconClient = World->SpawnActor<APartyBeaconClient>(APartyBeaconClient::StaticClass());
	check(ReservationBeaconClient.IsValid());

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(World);
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);
	FNamedOnlineSession* Session = Sessions->GetNamedSession(NAME_GameSession);
	check(Session);
	FString SessionIdStr = Session->GetSessionIdStr();

	FString ConnectInfo;
	Sessions->GetResolvedConnectString(NAME_GameSession, ConnectInfo, NAME_BeaconPort);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity);
	FUniqueNetIdWrapper DefaultNetId = Identity->GetUniquePlayerId(0);
	check(DefaultNetId.IsValid());

	FPlayerReservation PlayerReservation;
	PlayerReservation.UniqueId = *DefaultNetId;
	PlayerReservation.Platform = OnlineSub->GetLocalPlatformName();

	ReservationBeaconClient->OnHostConnectionFailure().BindWeakLambda(this, [this]()
		{
			// 我们只想在连接活跃时响应失败调用，而不是在它关闭时
			if(ReservationBeaconClient->GetNetDriver())
			{
				FOnlineResultInformation JoinSessionResult;
				JoinSessionResult.bWasSuccessful = false;
				JoinSessionResult.ErrorId = TEXT("UnknownError");

				NotifyJoinSessionComplete(JoinSessionResult);
				NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);

				CleanUpSessions();
			}
		});

	ReservationBeaconClient->OnReservationRequestComplete().BindWeakLambda(this, [this](EPartyReservationResult::Type ReservationResponse)
		{
			if (ReservationResponse == EPartyReservationResult::ReservationAccepted)
			{
				FOnlineResultInformation JoinSessionResult;
				JoinSessionResult.bWasSuccessful = true;
				NotifyJoinSessionComplete(JoinSessionResult);

				InternalTravelToSession(NAME_GameSession);
			}
			else
			{
				FOnlineResultInformation JoinSessionResult;
				JoinSessionResult.bWasSuccessful = false;
				JoinSessionResult.ErrorId = TEXT("UnknownError");

				NotifyJoinSessionComplete(JoinSessionResult);
				NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);

				CleanUpSessions();
			}
		});

	ReservationBeaconClient->RequestReservation(ConnectInfo, SessionIdStr, *DefaultNetId, { PlayerReservation });
}

void UCommonSessionSubsystem::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (bUseBeacons)
		{
			// InternalTravelToSession 和通知会在成功预约后由信标（beacon）调用。信标会在前往过程中被销毁。
			ConnectToHostReservationBeacon();
		}
		else
		{
			//@TODO Synchronize timing of this with create callbacks, modify both places and the comments if plan changes
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = true;
			NotifyJoinSessionComplete(JoinSessionResult);

			InternalTravelToSession(NAME_GameSession);
		}
	}
	else
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionIsFull", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionDoesNotExist", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		//@TODO: 错误处理
		UE_LOG(LogCommonSession, Error, TEXT("FinishJoinSession(Failed with Result: %s)"), *ReturnReason.ToString());

		// 没有可初始化的 FOnlineError
		FOnlineResultInformation JoinSessionResult;
		JoinSessionResult.bWasSuccessful = false;
		JoinSessionResult.ErrorId = LexToString(Result); // This is not robust but there is no extended information available
		JoinSessionResult.ErrorText = ReturnReason;
		NotifyJoinSessionComplete(JoinSessionResult);
		NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);

		// 如果会话加入失败，我们会清理该会话
		CleanUpSessions();
	}
}

#else

void UCommonSessionSubsystem::JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request)
{
	const FName SessionName(NAME_GameSession);
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);

	// 如果请求没有大厅（lobby），就假定它是一个会话
	if (!Request->Lobby.IsValid())
	{
		ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
		check(Sessions);
		FJoinSession::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		CreateParams.SessionName = SessionName;
		CreateParams.SessionId = Request->SessionID;
		UE::Online::FOnlineSessionId SessionID = Request->SessionID;

		Sessions->JoinSession(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName](const TOnlineResult<FJoinSession>& JoinResult)
			{
				if (JoinResult.IsOk())
				{
					InternalTravelToSession(SessionName);
				}
				else
				{
					//@TODO: 错误处理
					UE_LOG(LogCommonSession, Error, TEXT("JoinLobby Failed with Result: %s"), *ToLogString(JoinResult.GetErrorValue()));
				}
			});
	}
	else
	{
		 	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		 	check(Lobbies);
		 
		 	FJoinLobby::Params JoinParams;
			if (LocalPlayer)
			{
				JoinParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
			}
		 	JoinParams.LocalName = SessionName;
		 	JoinParams.LobbyId = Request->Lobby->LobbyId;
		 	JoinParams.bPresenceEnabled = true;
		 
		 	// Add any splitscreen players if they exist //@TODO: See UCommonSessionSubsystem::OnJoinSessionComplete
		 
		 	Lobbies->JoinLobby(MoveTemp(JoinParams)).OnComplete(this, [this, SessionName](const TOnlineResult<FJoinLobby>& JoinResult)
		 	{
		 		if (JoinResult.IsOk())
		 		{
		 			InternalTravelToSession(SessionName);
		 		}
		 		else
		 		{
		 			//@TODO: 错误处理
		 			UE_LOG(LogCommonSession, Error, TEXT("JoinLobby Failed with Result: %s"), *ToLogString(JoinResult.GetErrorValue()));
		 		}
		 	});
	}
}

void UCommonSessionSubsystem::OnSessionJoinRequested(const UE::Online::FUISessionJoinRequested& EventParams)
{
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	IAuthPtr Auth = OnlineServices->GetAuthInterface();
	check(Auth);
	TOnlineResult<FAuthGetLocalOnlineUserByOnlineAccountId> Account = Auth->GetLocalOnlineUserByOnlineAccountId({ EventParams.LocalAccountId });
	if (Account.IsOk())
	{
		FPlatformUserId PlatformUserId = Account.GetOkValue().AccountInfo->PlatformUserId;
		UCommonSession_SearchResult* RequestedSession = nullptr;
		FOnlineResultInformation ResultInfo;
		if (EventParams.Result.IsOk())
		{
			RequestedSession = NewObject<UCommonSession_SearchResult>(this);
			RequestedSession->SessionID = EventParams.Result.GetOkValue();
		}
		else
		{
			ResultInfo.FromOnlineError(EventParams.Result.GetErrorValue());
		}
		NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
	}
	else
	{
		UE_LOG(LogCommonSession, Error, TEXT("OnJoinSessionRequested: Failed to get account by local user id %s"), *UE::Online::ToLogString(EventParams.LocalAccountId));
	}
}

void UCommonSessionSubsystem::OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams)
{
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	IAuthPtr Auth = OnlineServices->GetAuthInterface();
	check(Auth);
	TOnlineResult<FAuthGetLocalOnlineUserByOnlineAccountId> Account = Auth->GetLocalOnlineUserByOnlineAccountId({ EventParams.LocalAccountId });
	if (Account.IsOk())
	{
		FPlatformUserId PlatformUserId = Account.GetOkValue().AccountInfo->PlatformUserId;
		UCommonSession_SearchResult* RequestedSession = nullptr;
		FOnlineResultInformation ResultInfo;
		if (EventParams.Result.IsOk())
		{
			RequestedSession = NewObject<UCommonSession_SearchResult>(this);
			RequestedSession->Lobby = EventParams.Result.GetOkValue();
		}
		else
		{
			ResultInfo.FromOnlineError(EventParams.Result.GetErrorValue());
		}
		NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
	}
	else
	{
		UE_LOG(LogCommonSession, Error, TEXT("OnJoinLobbyRequested: Failed to get account by local user id %s"), *UE::Online::ToLogString(EventParams.LocalAccountId));
	}
}

UE::Online::FAccountId UCommonSessionSubsystem::GetAccountId(APlayerController* PlayerController) const
{
	if (const ULocalPlayer* const LocalPlayer = PlayerController->GetLocalPlayer())
	{
		FUniqueNetIdRepl LocalPlayerIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		if (LocalPlayerIdRepl.IsValid())
		{
			return LocalPlayerIdRepl.GetV2();
		}
	}
	return FAccountId();
}

UE::Online::FLobbyId UCommonSessionSubsystem::GetLobbyId(const FName SessionName) const
{
	FAccountId LocalUserId = GetAccountId(GetGameInstance()->GetFirstLocalPlayerController());
	if (LocalUserId.IsValid())
	{
		IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
		check(OnlineServices);
		ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		check(Lobbies);
		TOnlineResult<FGetJoinedLobbies> JoinedLobbies = Lobbies->GetJoinedLobbies({ LocalUserId });
		if (JoinedLobbies.IsOk())
		{
			for (const TSharedRef<const FLobby>& Lobby : JoinedLobbies.GetOkValue().Lobbies)
			{
				if (Lobby->LocalName == SessionName)
				{
					return Lobby->LobbyId;
				}
			}
		}
	}
	return FLobbyId();
}

#endif // COMMONUSER_OSSV1
void UCommonSessionSubsystem::InternalTravelToSession(const FName SessionName)
{
	//@TODO: 理想情况下我们应该用触发的玩家而不是第一个（反正它们都会同时出发，所以可能不重要）
	APlayerController* const PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		UE_LOG(LogCommonSession, Error, TEXT("InternalTravelToSession(Failed due to %s)"), *ReturnReason.ToString());
		return;
	}

	FString URL;
#if COMMONUSER_OSSV1
	// 前往会话
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	if (!Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		UE_LOG(LogCommonSession, Error, TEXT("InternalTravelToSession(%s)"), *FailReason.ToString());
		return;
	}
#else
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld(), EOnlineServices::Default);
	check(OnlineServices);

	FAccountId LocalUserId = GetAccountId(PlayerController);
	if (LocalUserId.IsValid())
	{
		TOnlineResult<FGetResolvedConnectString> Result;
		if (!bUseLobbiesDefault)
		{
			ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
			check(Sessions)
			TOnlineResult<FGetSessionByName> SessionResult = Sessions->GetSessionByName({ SessionName });

			if (SessionResult.IsOk())
			{
				Result = OnlineServices->GetResolvedConnectString({ LocalUserId, FLobbyId(), SessionResult.GetOkValue().Session->GetSessionId(), NAME_GamePort });
			}
		}
		else
		{
			Result = OnlineServices->GetResolvedConnectString({ LocalUserId, GetLobbyId(SessionName) });
		}

		if (ensure(Result.IsOk()))
		{
			URL = Result.GetOkValue().ResolvedConnectString;
		}
	}
#endif // COMMONUSER_OSSV1

	// 允许在前往之前修改 URL
	OnPreClientTravelEvent.Broadcast(URL);

	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

void UCommonSessionSubsystem::NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
	K2_OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
}

void UCommonSessionSubsystem::NotifyJoinSessionComplete(const FOnlineResultInformation& Result)
{
	OnJoinSessionCompleteEvent.Broadcast(Result);
	K2_OnJoinSessionCompleteEvent.Broadcast(Result);
}

void UCommonSessionSubsystem::NotifyCreateSessionComplete(const FOnlineResultInformation& Result)
{
	OnCreateSessionCompleteEvent.Broadcast(Result);
	K2_OnCreateSessionCompleteEvent.Broadcast(Result);
}

void UCommonSessionSubsystem::NotifySessionInformationUpdated(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	OnSessionInformationChangedEvent.Broadcast(SessionStatus, GameMode, MapName);
	K2_OnSessionInformationChangedEvent.Broadcast(SessionStatus, GameMode, MapName);
}

void UCommonSessionSubsystem::NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
	K2_OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
}

void UCommonSessionSubsystem::SetCreateSessionError(const FText& ErrorText)
{
	CreateSessionResult.bWasSuccessful = false;
	CreateSessionResult.ErrorId = TEXT("InternalFailure");

	// TODO 可能要根据你想给用户多少数据，在发布构建中替换成通用错误文本
	CreateSessionResult.ErrorText = ErrorText;
}

void UCommonSessionSubsystem::CreateHostReservationBeacon()
{
	check(!BeaconHostListener.IsValid());
	check(!ReservationBeaconHost.IsValid());

	UWorld* const World = GetWorld();
	BeaconHostListener = World->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
	check(BeaconHostListener.IsValid());
	verify(BeaconHostListener->InitHost());

	ReservationBeaconHost = World->SpawnActor<APartyBeaconHost>(APartyBeaconHost::StaticClass());
	check(ReservationBeaconHost.IsValid());

	if (ReservationBeaconHostState)
	{
		ReservationBeaconHost->InitFromBeaconState(&*ReservationBeaconHostState);
	}
	else
	{
		// TODO: We are using the default hard-coded values for the parameters for now, but they are configurable
		ReservationBeaconHost->InitHostBeacon(BeaconTeamCount, BeaconTeamSize, BeaconMaxReservations, NAME_GameSession);
		ReservationBeaconHostState = ReservationBeaconHost->GetState();
	}

	BeaconHostListener->RegisterHost(ReservationBeaconHost.Get());
	BeaconHostListener->PauseBeaconRequests(false);
}

void UCommonSessionSubsystem::DestroyHostReservationBeacon()
{
	if (BeaconHostListener.IsValid() && ReservationBeaconHost.IsValid())
	{
		BeaconHostListener->UnregisterHost(ReservationBeaconHost->GetBeaconType());
	}
	if (BeaconHostListener.IsValid())
	{
		BeaconHostListener->Destroy();
		BeaconHostListener = nullptr;
	}
	if (ReservationBeaconHost.IsValid())
	{
		ReservationBeaconHost->Destroy();
		ReservationBeaconHost = nullptr;
	}
}

#if COMMONUSER_OSSV1
void UCommonSessionSubsystem::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogCommonSession, Warning, TEXT("UCommonSessionSubsystem::HandleSessionFailure(NetId: %s, FailureType: %s)"), *NetId.ToDebugString(), LexToString(FailureType));
	
	//@TODO: 可能需要做得更多一点……
}

void UCommonSessionSubsystem::HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserIndex);

	UCommonSession_SearchResult* RequestedSession = nullptr;
	FOnlineResultInformation ResultInfo;
	if (bWasSuccessful)
	{
		RequestedSession = NewObject<UCommonSession_SearchResult>(this);
		RequestedSession->Result = SearchResult;
	}
	else
	{
		// 没有可初始化的 FOnlineError
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("failed"); // This is not robust but there is no extended information available
		ResultInfo.ErrorText = LOCTEXT("Error_SessionUserInviteAcceptedFailed", "Failed to handle the join request");
	}
	NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
}

#endif // COMMONUSER_OSSV1

void UCommonSessionSubsystem::TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	// 这个委托是全局的，但 PIE 可能有多个 game instance，所以要
	// 确保它是为与此 game instance 子系统关联的同一个 world 触发的
	if (World != GetWorld())
	{
		return;
	}

	UE_LOG(LogCommonSession, Warning, TEXT("TravelLocalSessionFailure(World: %s, FailureType: %s, ReasonString: %s)"),
		*GetPathNameSafe(World),
		ETravelFailure::ToString(FailureType),
		*ReasonString);

	// TODO: 等我们也能广播成功时，再广播这个失败。目前我们在开始前往之前就广播成功，所以成功之后又失败会很令人困惑。
	//FOnlineResultInformation JoinSessionResult;
	//JoinSessionResult.bWasSuccessful = false;
	//JoinSessionResult.ErrorId = ReasonString; // TODO:  Is this an adequate ErrorId?
	//JoinSessionResult.ErrorText = FText::FromString(ReasonString);
	//NotifyJoinSessionComplete(JoinSessionResult);
	NotifySessionInformationUpdated(ECommonSessionInformationState::OutOfGame);
}

void UCommonSessionSubsystem::HandlePostLoadMap(UWorld* World)
{
	// 忽略 null world。
	if (!World)
	{
		return;
	}

	// 忽略任何不属于此 game instance 的 world，这在编辑器中是可能出现的。
	if (World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	// 除非 world 类型是 game/pie，否则我们不需要更新会话。
	if (!(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		return;
	}

#if COMMONUSER_OSSV1
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	const FName SessionName(NAME_GameSession);
	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(SessionName);

	// If we're hosting a session, update the advertised map name.
	if (CurrentSession != nullptr && CurrentSession->bHosting)
	{
		// 这需要是完整的包路径，才能匹配主机的 GetMapName 函数，World->GetMapName 目前是短名 - 更新主机设置
		CurrentSession->SessionSettings.Set(SETTING_MAPNAME, UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()), EOnlineDataAdvertisementType::ViaOnlineService);

		SessionInterface->UpdateSession(SessionName, CurrentSession->SessionSettings, true);

		if (bUseBeacons)
		{
			CreateHostReservationBeacon();
		}
	}
#endif // COMMONUSER_OSSV1
}

#undef LOCTEXT_NAMESPACE
#undef LOCTEXT_NAMESPACE
