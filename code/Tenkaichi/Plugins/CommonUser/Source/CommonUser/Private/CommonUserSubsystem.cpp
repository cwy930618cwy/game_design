// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUserSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "InputKeyEventArgs.h"
#include "NativeGameplayTags.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUserSubsystem)

#if COMMONUSER_OSSV1
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#else
#include "Online/Auth.h"
#include "Online/ExternalUI.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineServices.h"
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Privileges.h"

using namespace UE::Online;
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogCommonUser, Log, All);
DEFINE_LOG_CATEGORY(LogCommonUser);

UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::SystemMessage_Error, "SystemMessage.Error");
UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::SystemMessage_Warning, "SystemMessage.Warning");
UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::SystemMessage_Display, "SystemMessage.Display");
UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::SystemMessage_Error_InitializeLocalPlayerFailed, "SystemMessage.Error.InitializeLocalPlayerFailed");

UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::Platform_Trait_RequiresStrictControllerMapping, "Platform.Trait.RequiresStrictControllerMapping");
UE_DEFINE_GAMEPLAY_TAG(FCommonUserTags::Platform_Trait_SingleOnlineUser, "Platform.Trait.SingleOnlineUser");


//////////////////////////////////////////////////////////////////////
// UCommonUserInfo

UCommonUserInfo::FCachedData* UCommonUserInfo::GetCachedData(ECommonUserOnlineContext Context)
{
	// 直接查找，game 有一个与 default 分开的缓存
	FCachedData* FoundData = CachedDataMap.Find(Context);
	if (FoundData)
	{
		return FoundData;
	}

	// 现在尝试系统解析
	UCommonUserSubsystem* Subsystem = GetSubsystem();

	ECommonUserOnlineContext ResolvedContext = Subsystem->ResolveOnlineContext(Context);
	return CachedDataMap.Find(ResolvedContext);
}

const UCommonUserInfo::FCachedData* UCommonUserInfo::GetCachedData(ECommonUserOnlineContext Context) const
{
	return const_cast<UCommonUserInfo*>(this)->GetCachedData(Context);
}

void UCommonUserInfo::UpdateCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context)
{
	// 只应该用已解析且有效的类型来调用这个
	FCachedData* GameCache = GetCachedData(ECommonUserOnlineContext::Game);
	FCachedData* ContextCache = GetCachedData(Context);

	if (!ensure(GameCache && ContextCache))
	{
		// 应该始终有效
		return;
	}

	// 先更新直接缓存
	ContextCache->CachedPrivileges.Add(Privilege, Result);

	if (GameCache != ContextCache)
	{
		// 查找另一个上下文合并进 game
		ECommonUserPrivilegeResult GameContextResult = Result;
		ECommonUserPrivilegeResult OtherContextResult = ECommonUserPrivilegeResult::Available;
		for (TPair<ECommonUserOnlineContext, FCachedData>& Pair : CachedDataMap)
		{
			if (&Pair.Value != ContextCache && &Pair.Value != GameCache)
			{
				ECommonUserPrivilegeResult* FoundResult = Pair.Value.CachedPrivileges.Find(Privilege);
				if (FoundResult)
				{
					OtherContextResult = *FoundResult;
				}
				else
				{
					OtherContextResult = ECommonUserPrivilegeResult::Unknown;
				}
				break;
			}
		}

		if (GameContextResult == ECommonUserPrivilegeResult::Available && OtherContextResult != ECommonUserPrivilegeResult::Available)
		{
			// 另一个上下文更差，使用那个
			GameContextResult = OtherContextResult;
		}

		GameCache->CachedPrivileges.Add(Privilege, GameContextResult);
	}
}

void UCommonUserInfo::UpdateCachedNetId(const FUniqueNetIdRepl& NewId, ECommonUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNetId = NewId;

		// 更新昵称
		const UCommonUserSubsystem* Subsystem = GetSubsystem();
		if (ensure(Subsystem))
		{
			if (bIsGuest)
			{
				if (ContextCache->CachedNickname.IsEmpty())
				{
					// 如果为空就设置一个默认游客名，可以用 SetNickname 修改
					ContextCache->CachedNickname = NSLOCTEXT("CommonUser", "GuestNickname", "Guest").ToString();
				}
			}
			else
			{
				// 用系统昵称刷新，会覆盖 SetNickname
				ContextCache->CachedNickname = Subsystem->GetLocalUserNickname(GetPlatformUserId(), Context);
			}
		}
	}

	// 我们不合并 ID，因为游客的工作方式特殊
}

class UCommonUserSubsystem* UCommonUserInfo::GetSubsystem() const
{
	return Cast<UCommonUserSubsystem>(GetOuter());
}

bool UCommonUserInfo::IsLoggedIn() const
{
	return (InitializationState == ECommonUserInitializationState::LoggedInLocalOnly || InitializationState == ECommonUserInitializationState::LoggedInOnline);
}

bool UCommonUserInfo::IsDoingLogin() const
{
	return (InitializationState == ECommonUserInitializationState::DoingInitialLogin || InitializationState == ECommonUserInitializationState::DoingNetworkLogin);
}

ECommonUserPrivilegeResult UCommonUserInfo::GetCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
	{
		const ECommonUserPrivilegeResult* FoundResult = FoundCached->CachedPrivileges.Find(Privilege);
		if (FoundResult)
		{
			return *FoundResult;
		}
	}
	return ECommonUserPrivilegeResult::Unknown;
}

ECommonUserAvailability UCommonUserInfo::GetPrivilegeAvailability(ECommonUserPrivilege Privilege) const
{
	// 功能或用户无效
	if ((int32)Privilege < 0 || (int32)Privilege >= (int32)ECommonUserPrivilege::Invalid_Count || InitializationState == ECommonUserInitializationState::Invalid)
	{
		return ECommonUserAvailability::Invalid;
	}

	ECommonUserPrivilegeResult CachedResult = GetCachedPrivilegeResult(Privilege, ECommonUserOnlineContext::Game);

	// 首先处理显式失败
	switch (CachedResult)
	{
	case ECommonUserPrivilegeResult::LicenseInvalid:
	case ECommonUserPrivilegeResult::VersionOutdated:
	case ECommonUserPrivilegeResult::AgeRestricted:
		return ECommonUserAvailability::AlwaysUnavailable;

	case ECommonUserPrivilegeResult::NetworkConnectionUnavailable:
	case ECommonUserPrivilegeResult::AccountTypeRestricted:
	case ECommonUserPrivilegeResult::AccountUseRestricted:
	case ECommonUserPrivilegeResult::PlatformFailure:
		return ECommonUserAvailability::CurrentlyUnavailable;

	default:
		break;
	}

	if (bIsGuest)
	{
		// 游客只能游玩，不能使用在线功能
		if (Privilege == ECommonUserPrivilege::CanPlay)
		{
			return ECommonUserAvailability::NowAvailable;
		}
		else
		{
			return ECommonUserAvailability::AlwaysUnavailable;
		}
	}

	// 检查网络状态
	if (Privilege == ECommonUserPrivilege::CanPlayOnline ||
		Privilege == ECommonUserPrivilege::CanUseCrossPlay ||
		Privilege == ECommonUserPrivilege::CanCommunicateViaTextOnline ||
		Privilege == ECommonUserPrivilege::CanCommunicateViaVoiceOnline)
	{
		UCommonUserSubsystem* Subsystem = GetSubsystem();
		if (ensure(Subsystem) && !Subsystem->HasOnlineConnection(ECommonUserOnlineContext::Game))
		{
			return ECommonUserAvailability::CurrentlyUnavailable;
		}
	}

	if (InitializationState == ECommonUserInitializationState::FailedtoLogin)
	{
		// 之前的登录尝试失败了
		return ECommonUserAvailability::CurrentlyUnavailable;
	}
	else if (InitializationState == ECommonUserInitializationState::Unknown || InitializationState == ECommonUserInitializationState::DoingInitialLogin)
	{
		// 尚未登录
		return ECommonUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == ECommonUserInitializationState::LoggedInLocalOnly || InitializationState == ECommonUserInitializationState::DoingNetworkLogin)
	{
		// 本地登录成功，所以游玩检查有效
		if (Privilege == ECommonUserPrivilege::CanPlay && CachedResult == ECommonUserPrivilegeResult::Available)
		{
			return ECommonUserAvailability::NowAvailable;
		}

		// 尚未在线登录
		return ECommonUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == ECommonUserInitializationState::LoggedInOnline)
	{
		// 已完全登录
		if (CachedResult == ECommonUserPrivilegeResult::Available)
		{
			return ECommonUserAvailability::NowAvailable;
		}

		// 因其他原因失败
		return ECommonUserAvailability::CurrentlyUnavailable;
	}

	return ECommonUserAvailability::Unknown;
}

FUniqueNetIdRepl UCommonUserInfo::GetNetId(ECommonUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
	{
		return FoundCached->CachedNetId;
	}

	return FUniqueNetIdRepl();
}

FString UCommonUserInfo::GetNickname(ECommonUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
	{
		return FoundCached->CachedNickname;
	}

	// TODO 也许这里应该返回 unknown user？
	return FString();
}

void UCommonUserInfo::SetNickname(const FString& NewNickname, ECommonUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNickname = NewNickname;
	}
}

FString UCommonUserInfo::GetDebugString() const
{
	FUniqueNetIdRepl NetId = GetNetId();
	return NetId.ToDebugString();
}

FPlatformUserId UCommonUserInfo::GetPlatformUserId() const
{
	return PlatformUser;
}

int32 UCommonUserInfo::GetPlatformUserIndex() const
{
	// 把我们的平台 ID 转换为索引
	const UCommonUserSubsystem* Subsystem = GetSubsystem();

	if (ensure(Subsystem))
	{
		return Subsystem->GetPlatformUserIndexForId(PlatformUser);
	}

	return INDEX_NONE;
}


//////////////////////////////////////////////////////////////////////
// UCommonUserSubsystem

void UCommonUserSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建我们的 OSS 包装器
	CreateOnlineContexts();

	BindOnlineDelegates();
	
	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);

	// 匹配引擎默认值
	SetMaxLocalPlayers(4);

	ResetUserState();

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UCommonUserSubsystem::CreateOnlineContexts()
{
	// 先初始化 default
	DefaultContextInternal = new FOnlineContextCache();
#if COMMONUSER_OSSV1
	DefaultContextInternal->OnlineSubsystem = Online::GetSubsystem(GetWorld());
	check(DefaultContextInternal->OnlineSubsystem);
	DefaultContextInternal->IdentityInterface = DefaultContextInternal->OnlineSubsystem->GetIdentityInterface();
	check(DefaultContextInternal->IdentityInterface.IsValid());

	IOnlineSubsystem* PlatformSub = IOnlineSubsystem::GetByPlatform();

	if (PlatformSub && DefaultContextInternal->OnlineSubsystem != PlatformSub)
	{
		// 如果存在，就设置可选的平台服务
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineSubsystem = PlatformSub;
		PlatformContextInternal->IdentityInterface = PlatformSub->GetIdentityInterface();
		check(PlatformContextInternal->IdentityInterface.IsValid());
	}
#else
	DefaultContextInternal->OnlineServices = GetServices(GetWorld(), EOnlineServices::Default);
	check(DefaultContextInternal->OnlineServices);
	DefaultContextInternal->AuthService = DefaultContextInternal->OnlineServices->GetAuthInterface();
	check(DefaultContextInternal->AuthService);

	UE::Online::IOnlineServicesPtr PlatformServices = GetServices(GetWorld(), EOnlineServices::Platform);
	if (PlatformServices && DefaultContextInternal->OnlineServices != PlatformServices)
	{
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineServices = PlatformServices;
		PlatformContextInternal->AuthService = PlatformContextInternal->OnlineServices->GetAuthInterface();
		check(PlatformContextInternal->AuthService);
	}
#endif

	// 显式的外部服务之后可以在需要时再设置
}

void UCommonUserSubsystem::Deinitialize()
{
	DestroyOnlineContexts();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);

	LocalUserInfos.Reset();
	ActiveLoginRequests.Reset();

	Super::Deinitialize();
}

void UCommonUserSubsystem::DestroyOnlineContexts()
{
	// 所有缓存的共享指针必须在这里清除
	if (ServiceContextInternal && ServiceContextInternal != DefaultContextInternal)
	{
		delete ServiceContextInternal;
	}
	if (PlatformContextInternal && PlatformContextInternal != DefaultContextInternal)
	{
		delete PlatformContextInternal;
	}
	if (DefaultContextInternal)
	{
		delete DefaultContextInternal;
	}

	ServiceContextInternal = PlatformContextInternal = DefaultContextInternal = nullptr;
}

UCommonUserInfo* UCommonUserSubsystem::CreateLocalUserInfo(int32 LocalPlayerIndex)
{
	UCommonUserInfo* NewUser = nullptr;
	if (ensure(!LocalUserInfos.Contains(LocalPlayerIndex)))
	{
		NewUser = NewObject<UCommonUserInfo>(this);
		NewUser->LocalPlayerIndex = LocalPlayerIndex;
		NewUser->InitializationState = ECommonUserInitializationState::Unknown;

		// 始终创建 game 和 default 缓存
		NewUser->CachedDataMap.Add(ECommonUserOnlineContext::Game, UCommonUserInfo::FCachedData());
		NewUser->CachedDataMap.Add(ECommonUserOnlineContext::Default, UCommonUserInfo::FCachedData());

		// 如果需要就添加 platform
		if (HasSeparatePlatformContext())
		{
			NewUser->CachedDataMap.Add(ECommonUserOnlineContext::Platform, UCommonUserInfo::FCachedData());
		}

		LocalUserInfos.Add(LocalPlayerIndex, NewUser);
	}
	return NewUser;
}

bool UCommonUserSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 仅当不存在游戏特定子类时才创建实例
	return ChildClasses.Num() == 0;
}

void UCommonUserSubsystem::BindOnlineDelegates()
{
#if COMMONUSER_OSSV1
	return BindOnlineDelegatesOSSv1();
#else
	return BindOnlineDelegatesOSSv2();
#endif
}

void UCommonUserSubsystem::LogOutLocalUser(FPlatformUserId PlatformUser)
{
	UCommonUserInfo* UserInfo = ModifyInfo(GetUserInfoForPlatformUser(PlatformUser));

	// 如果用户从未完全登录或正在登录过程中，就不需要做任何事
	if (UserInfo && (UserInfo->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly || UserInfo->InitializationState == ECommonUserInitializationState::LoggedInOnline))
	{
		ECommonUserAvailability OldAvailablity = UserInfo->GetPrivilegeAvailability(ECommonUserPrivilege::CanPlay);

		UserInfo->InitializationState = ECommonUserInitializationState::FailedtoLogin;

		// 这会广播 game 委托
		HandleChangedAvailability(UserInfo, ECommonUserPrivilege::CanPlay, OldAvailablity);
	}
}

bool UCommonUserSubsystem::TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if COMMONUSER_OSSV1
	// V1 路径不支持
	return false;
#else
	return TransferPlatformAuthOSSv2(System, Request, PlatformUser);
#endif
}

bool UCommonUserSubsystem::AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogCommonUser, Log, TEXT("Player AutoLogin requested - UserIdx:%d, Privilege:%d, Context:%d"),
		PlatformUser.GetInternalId(),
		(int32)Request->DesiredPrivilege,
		(int32)Request->DesiredContext);

#if COMMONUSER_OSSV1
	return AutoLoginOSSv1(System, Request, PlatformUser);
#else
	return AutoLoginOSSv2(System, Request, PlatformUser);
#endif
}

bool UCommonUserSubsystem::ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogCommonUser, Log, TEXT("Player LoginUI requested - UserIdx:%d, Privilege:%d, Context:%d"),
		PlatformUser.GetInternalId(),
		(int32)Request->DesiredPrivilege,
		(int32)Request->DesiredContext);

#if COMMONUSER_OSSV1
	return ShowLoginUIOSSv1(System, Request, PlatformUser);
#else
	return ShowLoginUIOSSv2(System, Request, PlatformUser);
#endif
}

bool UCommonUserSubsystem::QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if COMMONUSER_OSSV1
	return QueryUserPrivilegeOSSv1(System, Request, PlatformUser);
#else
	return QueryUserPrivilegeOSSv2(System, Request, PlatformUser);
#endif
}


#if COMMONUSER_OSSV1
IOnlineSubsystem* UCommonUserSubsystem::GetOnlineSubsystem(ECommonUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);

	if (System)
	{
		return System->OnlineSubsystem;
	}

	return nullptr;
}

IOnlineIdentity* UCommonUserSubsystem::GetOnlineIdentity(ECommonUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return System->IdentityInterface.Get();
	}

	return nullptr;
}

FName UCommonUserSubsystem::GetOnlineSubsystemName(ECommonUserOnlineContext Context) const
{
	IOnlineSubsystem* SubSystem = GetOnlineSubsystem(Context);
	if (SubSystem)
	{
		return SubSystem->GetSubsystemName();
	}

	return NAME_None;
}

EOnlineServerConnectionStatus::Type UCommonUserSubsystem::GetConnectionStatus(ECommonUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return System->CurrentConnectionStatus;
	}

	return EOnlineServerConnectionStatus::ServiceUnavailable;
}

void UCommonUserSubsystem::BindOnlineDelegatesOSSv1()
{
	ECommonUserOnlineContext ServiceType = ResolveOnlineContext(ECommonUserOnlineContext::ServiceOrDefault);
	ECommonUserOnlineContext PlatformType = ResolveOnlineContext(ECommonUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineSubsystem && PlatformContext && PlatformContext->OnlineSubsystem);
	// 连接委托需要同时监听两个系统

	ServiceContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType));
	ServiceContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

	for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
	{
		ServiceContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, ServiceType));
		ServiceContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, ServiceType));
	}

	if (ServiceType != PlatformType)
	{
		PlatformContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType));
		PlatformContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

		for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
		{
			PlatformContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, PlatformType));
			PlatformContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, PlatformType));
		}
	}

	// 硬件变化委托只监听 platform
	PlatformContext->IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &ThisClass::HandleControllerPairingChanged));
}

bool UCommonUserSubsystem::AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	return System->IdentityInterface->AutoLogin(GetPlatformUserIndexForId(PlatformUser));
}

bool UCommonUserSubsystem::ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IOnlineExternalUIPtr ExternalUI = System->OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		// TODO 不清楚应该设置哪些标志
		return ExternalUI->ShowLoginUI(GetPlatformUserIndexForId(PlatformUser), false, false, FOnLoginUIClosedDelegate::CreateUObject(this, &ThisClass::HandleOnLoginUIClosed, Request->CurrentContext));
	}
	return false;
}

bool UCommonUserSubsystem::QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	// 在 unknown 或 failure 时开始查询
	EUserPrivileges::Type OSSPrivilege = ConvertOSSPrivilege(Request->DesiredPrivilege);

	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	check(CurrentId.IsValid());
	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate Delegate = IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UCommonUserSubsystem::HandleCheckPrivilegesComplete, Request->DesiredPrivilege, Request->UserInfo, Request->CurrentContext);
	System->IdentityInterface->GetUserPrivilege(*CurrentId, OSSPrivilege, Delegate);

	// 这可能会立即成功并重新进入此函数，所以我们必须返回
	return true;
}

#else

UE::Online::EOnlineServices UCommonUserSubsystem::GetOnlineServicesProvider(ECommonUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
	{
		return System->OnlineServices->GetServicesProvider();
	}
	return UE::Online::EOnlineServices::None;
}

UE::Online::IAuthPtr UCommonUserSubsystem::GetOnlineAuth(ECommonUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
	{
		return System->AuthService;
	}
	return nullptr;
}

UE::Online::EOnlineServicesConnectionStatus UCommonUserSubsystem::GetConnectionStatus(ECommonUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
	{
		return System->CurrentConnectionStatus;
	}
	return UE::Online::EOnlineServicesConnectionStatus::NotConnected;
}

void UCommonUserSubsystem::BindOnlineDelegatesOSSv2()
{
	ECommonUserOnlineContext ServiceType = ResolveOnlineContext(ECommonUserOnlineContext::ServiceOrDefault);
	ECommonUserOnlineContext PlatformType = ResolveOnlineContext(ECommonUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineServices && PlatformContext && PlatformContext->OnlineServices);

	ServiceContext->LoginStatusChangedHandle = ServiceContext->AuthService->OnLoginStatusChanged().Add(this, &ThisClass::HandleAuthLoginStatusChanged, ServiceType);
	if (IConnectivityPtr ConnectivityInterface = ServiceContext->OnlineServices->GetConnectivityInterface())
	{
		ServiceContext->ConnectionStatusChangedHandle = ConnectivityInterface->OnConnectionStatusChanged().Add(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType);
	}
	CacheConnectionStatus(ServiceType);

	if (ServiceType != PlatformType)
	{
		PlatformContext->LoginStatusChangedHandle = PlatformContext->AuthService->OnLoginStatusChanged().Add(this, &ThisClass::HandleAuthLoginStatusChanged, PlatformType);
		if (IConnectivityPtr ConnectivityInterface = PlatformContext->OnlineServices->GetConnectivityInterface())
		{
			PlatformContext->ConnectionStatusChangedHandle = ConnectivityInterface->OnConnectionStatusChanged().Add(this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType);
		}
		CacheConnectionStatus(PlatformType);
	}
	// TODO:  控制器配对变化 - 移出 OSS，改为直接监听 CoreDelegate？
}

void UCommonUserSubsystem::CacheConnectionStatus(ECommonUserOnlineContext Context)
{
	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	EOnlineServicesConnectionStatus ConnectionStatus = EOnlineServicesConnectionStatus::NotConnected;
	if (IConnectivityPtr ConnectivityInterface = ContextCache->OnlineServices->GetConnectivityInterface())
	{
		const TOnlineResult<FGetConnectionStatus> Result = ConnectivityInterface->GetConnectionStatus(FGetConnectionStatus::Params());
		if (Result.IsOk())
		{
			ConnectionStatus = Result.GetOkValue().Status;
		}
	}
	else
	{
		ConnectionStatus = EOnlineServicesConnectionStatus::Connected;
	}

	UE::Online::FConnectionStatusChanged EventParams;
	EventParams.PreviousStatus = ContextCache->CurrentConnectionStatus;
	EventParams.CurrentStatus = ConnectionStatus;
	HandleNetworkConnectionStatusChanged(EventParams, Context);
}

bool UCommonUserSubsystem::TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IAuthPtr PlatformAuthInterface = GetOnlineAuth(ECommonUserOnlineContext::Platform);
	if (Request->CurrentContext != ECommonUserOnlineContext::Platform
		&& PlatformAuthInterface)
	{
		FAuthQueryExternalAuthToken::Params Params;
		Params.LocalAccountId = GetLocalUserNetId(PlatformUser, ECommonUserOnlineContext::Platform).GetV2();

		PlatformAuthInterface->QueryExternalAuthToken(MoveTemp(Params))
		.OnComplete(this, [this, Request](const TOnlineResult<FAuthQueryExternalAuthToken>& Result)
		{
			UCommonUserInfo* UserInfo = Request->UserInfo.Get();
			if (!UserInfo)
			{
				// 用户已不存在，直接删除这个请求
				ActiveLoginRequests.Remove(Request);
				return;
			}

			if (Result.IsOk())
			{
				const FAuthQueryExternalAuthToken::Result& GenerateAuthTokenResult = Result.GetOkValue();
				FAuthLogin::Params Params;
				Params.PlatformUserId = UserInfo->GetPlatformUserId();
				Params.CredentialsType = LoginCredentialsType::ExternalAuth;
				Params.CredentialsToken.Emplace<FExternalAuthToken>(GenerateAuthTokenResult.ExternalAuthToken);

				IAuthPtr PrimaryAuthInterface = GetOnlineAuth(Request->CurrentContext);
				PrimaryAuthInterface->Login(MoveTemp(Params))
				.OnComplete(this, [this, Request](const TOnlineResult<FAuthLogin>& Result)
				{
					UCommonUserInfo* UserInfo = Request->UserInfo.Get();
					if (!UserInfo)
					{
						// 用户已不存在，直接删除这个请求
						ActiveLoginRequests.Remove(Request);
						return;
					}

					if (Result.IsOk())
					{
						Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::Done;
						Request->Error.Reset();
					}
					else
					{
						Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::Failed;
						Request->Error = Result.GetErrorValue();
					}
					ProcessLoginRequest(Request);
				});
			}
			else
			{
				Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::Failed;
				Request->Error = Result.GetErrorValue();
				ProcessLoginRequest(Request);
			}
		});
		return true;
	}
	return false;
}

bool UCommonUserSubsystem::AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	FAuthLogin::Params LoginParameters;
	LoginParameters.PlatformUserId = PlatformUser;
	LoginParameters.CredentialsType = LoginCredentialsType::Auto;
	// 其他 LoginParameters 保持默认，让在线服务决定如何尝试自动登录用户
	TOnlineAsyncOpHandle<FAuthLogin> LoginHandle = System->AuthService->Login(MoveTemp(LoginParameters));
	LoginHandle.OnComplete(this, &ThisClass::HandleUserLoginCompletedV2, PlatformUser, Request->CurrentContext);
	return true;
}

bool UCommonUserSubsystem::ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IExternalUIPtr ExternalUI = System->OnlineServices->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		FExternalUIShowLoginUI::Params ShowLoginUIParameters;
		ShowLoginUIParameters.PlatformUserId = PlatformUser;
		TOnlineAsyncOpHandle<FExternalUIShowLoginUI> LoginHandle = ExternalUI->ShowLoginUI(MoveTemp(ShowLoginUIParameters));
		LoginHandle.OnComplete(this, &ThisClass::HandleOnLoginUIClosedV2, PlatformUser, Request->CurrentContext);
		return true;
	}
	return false;
}

bool UCommonUserSubsystem::QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UCommonUserInfo* UserInfo = Request->UserInfo.Get();

	if (IPrivilegesPtr PrivilegesInterface = System->OnlineServices->GetPrivilegesInterface())
	{
		const EUserPrivileges DesiredPrivilege = ConvertOnlineServicesPrivilege(Request->DesiredPrivilege);

		FQueryUserPrivilege::Params Params;
		Params.LocalAccountId = GetLocalUserNetId(PlatformUser, Request->CurrentContext).GetV2();
		Params.Privilege = DesiredPrivilege;
		TOnlineAsyncOpHandle<FQueryUserPrivilege> QueryHandle = PrivilegesInterface->QueryUserPrivilege(MoveTemp(Params));
		QueryHandle.OnComplete(this, &ThisClass::HandleCheckPrivilegesComplete, Request->UserInfo, DesiredPrivilege, Request->CurrentContext);
		return true;
	}
	else
	{
		UpdateUserPrivilegeResult(UserInfo, Request->DesiredPrivilege, ECommonUserPrivilegeResult::Available, Request->CurrentContext);
	}
	return false;
}

TSharedPtr<FAccountInfo> UCommonUserSubsystem::GetOnlineServiceAccountInfo(IAuthPtr AuthService, FPlatformUserId InUserId) const
{
	TSharedPtr<FAccountInfo> AccountInfo;
	FAuthGetLocalOnlineUserByPlatformUserId::Params GetAccountParams = { InUserId };
	TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> GetAccountResult = AuthService->GetLocalOnlineUserByPlatformUserId(MoveTemp(GetAccountParams));
	if (GetAccountResult.IsOk())
	{
		AccountInfo = GetAccountResult.GetOkValue().AccountInfo;
	}
	return AccountInfo;
}

#endif

bool UCommonUserSubsystem::HasOnlineConnection(ECommonUserOnlineContext Context) const
{
#if COMMONUSER_OSSV1
	EOnlineServerConnectionStatus::Type ConnectionType = GetConnectionStatus(Context);

	if (ConnectionType == EOnlineServerConnectionStatus::Normal || ConnectionType == EOnlineServerConnectionStatus::Connected)
	{
		return true;
	}

	return false;
#else
	return GetConnectionStatus(Context) == UE::Online::EOnlineServicesConnectionStatus::Connected;
#endif
}

ELoginStatusType UCommonUserSubsystem::GetLocalUserLoginStatus(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return ELoginStatusType::NotLoggedIn;
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
#if COMMONUSER_OSSV1
		return System->IdentityInterface->GetLoginStatus(GetPlatformUserIndexForId(PlatformUser));
#else
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(System->AuthService, PlatformUser))
		{
			return AccountInfo->LoginStatus;
		}
#endif
	}
	return ELoginStatusType::NotLoggedIn;
}

FUniqueNetIdRepl UCommonUserSubsystem::GetLocalUserNetId(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return FUniqueNetIdRepl();
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
#if COMMONUSER_OSSV1
		return FUniqueNetIdRepl(System->IdentityInterface->GetUniquePlayerId(GetPlatformUserIndexForId(PlatformUser)));
#else
		// TODO:  OSSv2 对 FAccountId 的 FUniqueNetIdRepl 包装正在进行中
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(System->AuthService, PlatformUser))
		{
			return FUniqueNetIdRepl(AccountInfo->AccountId);
		}
#endif
	}

	return FUniqueNetIdRepl();
}

FString UCommonUserSubsystem::GetLocalUserNickname(FPlatformUserId PlatformUser, ECommonUserOnlineContext Context) const
{
#if COMMONUSER_OSSV1
	IOnlineIdentity* Identity = GetOnlineIdentity(Context);
	if (ensure(Identity))
	{
		return Identity->GetPlayerNickname(GetPlatformUserIndexForId(PlatformUser));
	}
#else
	if (IAuthPtr AuthService = GetOnlineAuth(Context))
	{
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(AuthService, PlatformUser))
		{
			if (const FSchemaVariant* DisplayName = AccountInfo->Attributes.Find(AccountAttributeData::DisplayName))
			{
				return DisplayName->GetString();
			}
		}
	}
#endif // COMMONUSER_OSSV1

	return FString();
}

void UCommonUserSubsystem::SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText)
{
	OnHandleSystemMessage.Broadcast(MessageType, TitleText, BodyText);
}

void UCommonUserSubsystem::SetMaxLocalPlayers(int32 InMaxLocalPlayers)
{
	if (ensure(InMaxLocalPlayers >= 1))
	{
		// 我们可以拥有比 MAX_LOCAL_PLAYERS 更多的本地玩家，其余的被当作游客
		MaxNumberOfLocalPlayers = InMaxLocalPlayers;

		UGameInstance* GameInstance = GetGameInstance();
		UGameViewportClient* ViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;

		if (ViewportClient)
		{
			ViewportClient->MaxSplitscreenPlayers = MaxNumberOfLocalPlayers;
		}
	}
}

int32 UCommonUserSubsystem::GetMaxLocalPlayers() const
{
	return MaxNumberOfLocalPlayers;
}

int32 UCommonUserSubsystem::GetNumLocalPlayers() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(GameInstance))
	{
		return GameInstance->GetNumLocalPlayers();
	}
	return 1;
}

ECommonUserInitializationState UCommonUserSubsystem::GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const
{
	const UCommonUserInfo* UserInfo = GetUserInfoForLocalPlayerIndex(LocalPlayerIndex);
	if (UserInfo)
	{
		return UserInfo->InitializationState;
	}

	if (LocalPlayerIndex < 0 || LocalPlayerIndex >= GetMaxLocalPlayers())
	{
		return ECommonUserInitializationState::Invalid;
	}

	return ECommonUserInitializationState::Unknown;
}

bool UCommonUserSubsystem::TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	if (!PrimaryInputDevice.IsValid())
	{
		// 设置为默认设备
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	}

	FCommonUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.PrimaryInputDevice = PrimaryInputDevice;
	Params.bCanUseGuestLogin = bCanUseGuestLogin;
	Params.bCanCreateNewLocalPlayer = true;
	Params.RequestedPrivilege = ECommonUserPrivilege::CanPlay;

	return TryToInitializeUser(Params);
}

bool UCommonUserSubsystem::TryToLoginForOnlinePlay(int32 LocalPlayerIndex)
{
	FCommonUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.bCanCreateNewLocalPlayer = false;
	Params.RequestedPrivilege = ECommonUserPrivilege::CanPlayOnline;

	return TryToInitializeUser(Params);
}

bool UCommonUserSubsystem::TryToInitializeUser(FCommonUserInitializeParams Params)
{
	if (Params.LocalPlayerIndex < 0 || (!Params.bCanCreateNewLocalPlayer && Params.LocalPlayerIndex >= GetNumLocalPlayers()))
	{
		if (!bIsDedicatedServer)
		{
			UE_LOG(LogCommonUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, invalid index"), 
				Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
			return false;
		}
	}

	if (Params.LocalPlayerIndex > GetNumLocalPlayers() || Params.LocalPlayerIndex >= GetMaxLocalPlayers())
	{
		UE_LOG(LogCommonUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, can only create in order up to max players"), 
			Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
		return false;
	}

	// 如果需要就填充平台用户和输入设备
	if (Params.ControllerId != INDEX_NONE && (!Params.PrimaryInputDevice.IsValid() || !Params.PlatformUser.IsValid()))
	{
		IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Params.ControllerId, Params.PlatformUser, Params.PrimaryInputDevice);
	}

	if (Params.PrimaryInputDevice.IsValid() && !Params.PlatformUser.IsValid())
	{
		Params.PlatformUser = GetPlatformUserIdForInputDevice(Params.PrimaryInputDevice);
	}
	else if (Params.PlatformUser.IsValid() && !Params.PrimaryInputDevice.IsValid())
	{
		Params.PrimaryInputDevice = GetPrimaryInputDeviceForPlatformUser(Params.PlatformUser);
	}

	UCommonUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));
	UCommonUserInfo* LocalUserInfoForController = ModifyInfo(GetUserInfoForInputDevice(Params.PrimaryInputDevice));

	if (LocalUserInfoForController && LocalUserInfo && LocalUserInfoForController != LocalUserInfo)
	{
		UE_LOG(LogCommonUser, Error, TEXT("TryToInitializeUser %d failed because controller %d is already assigned to player %d"),
			Params.LocalPlayerIndex, Params.PrimaryInputDevice.GetId(), LocalUserInfoForController->LocalPlayerIndex);
		return false;
	}

	if (Params.LocalPlayerIndex == 0 && Params.bCanUseGuestLogin)
	{
		UE_LOG(LogCommonUser, Error, TEXT("TryToInitializeUser failed because player 0 cannot be a guest"));
		return false;
	}

	if (!LocalUserInfo)
	{
		LocalUserInfo = CreateLocalUserInfo(Params.LocalPlayerIndex);
	}
	else
	{
		// 从现有用户信息复制
		if (!Params.PrimaryInputDevice.IsValid())
		{
			Params.PrimaryInputDevice = LocalUserInfo->PrimaryInputDevice;
		}

		if (!Params.PlatformUser.IsValid())
		{
			Params.PlatformUser = LocalUserInfo->PlatformUser;
		}
	}
	
	if (LocalUserInfo->InitializationState != ECommonUserInitializationState::Unknown && LocalUserInfo->InitializationState != ECommonUserInitializationState::FailedtoLogin)
	{
		// 登录期间不允许修改参数
		if (LocalUserInfo->PrimaryInputDevice != Params.PrimaryInputDevice || LocalUserInfo->PlatformUser != Params.PlatformUser || LocalUserInfo->bCanBeGuest != Params.bCanUseGuestLogin)
		{
			UE_LOG(LogCommonUser, Error, TEXT("TryToInitializeUser failed because player %d has already started the login process with diffrent settings!"), Params.LocalPlayerIndex);
			return false;
		}
	}

	// 现在就设置期望的索引，这样如果它创建了玩家，就知道该用哪个控制器
	LocalUserInfo->PrimaryInputDevice = Params.PrimaryInputDevice;
	LocalUserInfo->PlatformUser = Params.PlatformUser;
	LocalUserInfo->bCanBeGuest = Params.bCanUseGuestLogin;
	RefreshLocalUserInfo(LocalUserInfo);

	// 要么在进行初始登录，要么在进行网络登录
	if (LocalUserInfo->GetPrivilegeAvailability(ECommonUserPrivilege::CanPlay) == ECommonUserAvailability::NowAvailable && Params.RequestedPrivilege == ECommonUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::DoingNetworkLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::DoingInitialLogin;
	}

	LoginLocalUser(LocalUserInfo, Params.RequestedPrivilege, Params.OnlineContext, FOnLocalUserLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginForUserInitialize, Params));

	return true;
}

void UCommonUserSubsystem::ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FCommonUserInitializeParams Params)
{
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		const bool bIsMapped = LoginKeysForAnyUser.Num() > 0 || LoginKeysForNewUser.Num() > 0;
		const bool bShouldBeMapped = AnyUserKeys.Num() > 0 || NewUserKeys.Num() > 0;

		if (bIsMapped && !bShouldBeMapped)
		{
			// 把它设回包装的处理器
			ViewportClient->OnOverrideInputKey() = WrappedInputKeyHandler;
			WrappedInputKeyHandler.Unbind();
		}
		else if (!bIsMapped && bShouldBeMapped)
		{
			// 设置一个包装的处理器
			WrappedInputKeyHandler = ViewportClient->OnOverrideInputKey();
			ViewportClient->OnOverrideInputKey().BindUObject(this, &UCommonUserSubsystem::OverrideInputKeyForLogin);
		}

		LoginKeysForAnyUser = AnyUserKeys;
		LoginKeysForNewUser = NewUserKeys;

		if (bShouldBeMapped)
		{
			ParamsForLoginKey = Params;
		}
		else
		{
			ParamsForLoginKey = FCommonUserInitializeParams();
		}
	}
}

bool UCommonUserSubsystem::CancelUserInitialization(int32 LocalPlayerIndex)
{
	UCommonUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		return false;
	}

	if (!LocalUserInfo->IsDoingLogin())
	{
		return false;
	}

	// 从登录队列中移除
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.IsValid() && Request->UserInfo->LocalPlayerIndex == LocalPlayerIndex)
		{
			ActiveLoginRequests.Remove(Request);
		}
	}

	// 用最佳猜测设置状态
	if (LocalUserInfo->InitializationState == ECommonUserInitializationState::DoingNetworkLogin)
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::LoggedInLocalOnly;
	}
	else
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::FailedtoLogin;
	}

	return true;
}

bool UCommonUserSubsystem::TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer)
{
	UGameInstance* GameInstance = GetGameInstance();
	
	if (!ensure(GameInstance))
	{
		return false;
	}

	if (LocalPlayerIndex == 0 && bDestroyPlayer)
	{
		UE_LOG(LogCommonUser, Error, TEXT("TryToLogOutUser cannot destroy player 0"));
		return false;
	}

	CancelUserInitialization(LocalPlayerIndex);
	
	UCommonUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		UE_LOG(LogCommonUser, Warning, TEXT("TryToLogOutUser failed to log out user %i because they are not logged in"), LocalPlayerIndex);
		return false;
	}

	FPlatformUserId UserId = LocalUserInfo->PlatformUser;
	if (IsRealPlatformUser(UserId))
	{
		// 目前这不会做平台登出，以防他们想立刻重新登录
		UE_LOG(LogCommonUser, Log, TEXT("TryToLogOutUser succeeded for real platform user %d"), UserId.GetInternalId());

		LogOutLocalUser(UserId);
	}
	else if (ensure(LocalUserInfo->bIsGuest))
	{
		// 对于游客用户直接删除
		UE_LOG(LogCommonUser, Log, TEXT("TryToLogOutUser succeeded for guest player index %d"), LocalPlayerIndex);

		LocalUserInfos.Remove(LocalPlayerIndex);
	}

	if (bDestroyPlayer)
	{
		ULocalPlayer* ExistingPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(UserId);

		if (ExistingPlayer)
		{
			GameInstance->RemoveLocalPlayer(ExistingPlayer);
		}
	}

	return true;
}

void UCommonUserSubsystem::ResetUserState()
{
	// 手动清除现有的信息对象
	for (TPair<int32, UCommonUserInfo*> Pair : LocalUserInfos)
	{
		if (Pair.Value)
		{
			Pair.Value->MarkAsGarbage();
		}
	}

	LocalUserInfos.Reset();

	// 取消进行中的登录
	ActiveLoginRequests.Reset();

	// 为 ID 0 创建玩家信息
	UCommonUserInfo* FirstUser = CreateLocalUserInfo(0);

	FirstUser->PlatformUser = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
	FirstUser->PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(FirstUser->PlatformUser);

	// TODO: 安排下一帧刷新玩家 0？
	RefreshLocalUserInfo(FirstUser);
}

bool UCommonUserSubsystem::OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs)
{
	int32 NextLocalPlayerIndex = INDEX_NONE;

	const UCommonUserInfo* MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);
	if (EventArgs.Event == IE_Pressed)
	{
		if (MappedUser == nullptr || !MappedUser->IsLoggedIn())
		{
			if (MappedUser)
			{
				NextLocalPlayerIndex = MappedUser->LocalPlayerIndex;
			}
			else
			{
				// 查找下一个玩家
				for (int32 i = 0; i < MaxNumberOfLocalPlayers; i++)
				{
					if (GetLocalPlayerInitializationState(i) == ECommonUserInitializationState::Unknown)
					{
						NextLocalPlayerIndex = i;
						break;
					}
				}
			}

			if (NextLocalPlayerIndex != INDEX_NONE)
			{
				if (LoginKeysForAnyUser.Contains(EventArgs.Key))
				{
					// 如果我们正在登录过程中，就返回 true 以忽略平台特定的输入
					if (MappedUser && MappedUser->IsDoingLogin())
					{
						return true;
					}

					// 按下开始界面
					FCommonUserInitializeParams NewParams = ParamsForLoginKey;
					NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
					NewParams.PrimaryInputDevice = EventArgs.InputDevice;

					return TryToInitializeUser(NewParams);
				}

				// 看看这个控制器 ID 是否已映射
				MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);

				if (!MappedUser || MappedUser->LocalPlayerIndex == INDEX_NONE)
				{
					if (LoginKeysForNewUser.Contains(EventArgs.Key))
					{
						// 如果我们正在登录过程中，就返回 true 以忽略平台特定的输入
						if (MappedUser && MappedUser->IsDoingLogin())
						{
							return true;
						}

						// 本地多人游戏
						FCommonUserInitializeParams NewParams = ParamsForLoginKey;
						NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
						NewParams.PrimaryInputDevice = EventArgs.InputDevice;

						return TryToInitializeUser(NewParams);
					}
				}
			}
		}
	}

	if (WrappedInputKeyHandler.IsBound())
	{
		return WrappedInputKeyHandler.Execute(EventArgs);
	}

	return false;
}

static inline FText GetErrorText(const FOnlineErrorType& InOnlineError)
{
#if COMMONUSER_OSSV1
	return InOnlineError.GetErrorMessage();
#else
	return InOnlineError.GetText();
#endif
}

void UCommonUserSubsystem::HandleLoginForUserInitialize(const UCommonUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& InError, ECommonUserOnlineContext Context, FCommonUserInitializeParams Params)
{
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	FTimerManager& TimerManager = GameInstance->GetTimerManager();
	TOptional<FOnlineErrorType> Error = InError; // Copy so we can reset on handled errors

	UCommonUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	UCommonUserInfo* FirstUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(0));

	if (!ensure(LocalUserInfo && FirstUserInfo))
	{
		return;
	}

	// 检查硬平台/服务 ID
	RefreshLocalUserInfo(LocalUserInfo);

	FUniqueNetIdRepl FirstPlayerId = FirstUserInfo->GetNetId(ECommonUserOnlineContext::PlatformOrDefault);

	// 检查登录失败后是否应该创建游客。有些平台返回成功但复用了第一个玩家的 ID，把这当作失败
	if (LocalUserInfo != FirstUserInfo && LocalUserInfo->bCanBeGuest && (NewStatus == ELoginStatusType::NotLoggedIn || NetId == FirstPlayerId))
	{
#if COMMONUSER_OSSV1
		NetId = (FUniqueNetIdRef)FUniqueNetIdString::Create(FString::Printf(TEXT("GuestPlayer%d"), LocalUserInfo->LocalPlayerIndex), NULL_SUBSYSTEM);
#else
		// TODO:  OSSv2 FUniqueNetIdRepl 对 FAccountId 的包装正在进行中
		// TODO:  OSSv2 - 如何处理游客账户？
#endif
		LocalUserInfo->bIsGuest = true;
		NewStatus = ELoginStatusType::UsingLocalProfile;
		Error.Reset();
		UE_LOG(LogCommonUser, Log, TEXT("HandleLoginForUserInitialize created guest id %s for local player %d"), *NetId.ToString(), LocalUserInfo->LocalPlayerIndex);
	}
	else
	{
		LocalUserInfo->bIsGuest = false;
	}

	ensure(LocalUserInfo->IsDoingLogin());

	if (Error.IsSet())
	{
		FText ErrorText = GetErrorText(Error.GetValue());
		TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UCommonUserSubsystem::HandleUserInitializeFailed, Params, ErrorText));
		return;
	}

	if (Context == ECommonUserOnlineContext::Game)
	{
		LocalUserInfo->UpdateCachedNetId(NetId, ECommonUserOnlineContext::Game);
	}
		
	ULocalPlayer* CurrentPlayer = GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex);
	if (!CurrentPlayer && Params.bCanCreateNewLocalPlayer)
	{
		FString ErrorString;
		CurrentPlayer = GameInstance->CreateLocalPlayer(LocalUserInfo->PlatformUser, ErrorString, true);

		if (!CurrentPlayer)
		{
			TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UCommonUserSubsystem::HandleUserInitializeFailed, Params, FText::AsCultureInvariant(ErrorString)));
			return;
		}
		ensure(GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex) == CurrentPlayer);
	}

	// 如果需要就更新控制器和网络 ID
	SetLocalPlayerUserInfo(CurrentPlayer, LocalUserInfo);

	// 设置一个延迟回调
	TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UCommonUserSubsystem::HandleUserInitializeSucceeded, Params));
}

void UCommonUserSubsystem::HandleUserInitializeFailed(FCommonUserInitializeParams Params, FText Error)
{
	UCommonUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// 自这个被调度以来，用户信息已被重置
		return;
	}

	UE_LOG(LogCommonUser, Warning, TEXT("TryToInitializeUser %d failed with error %s"), LocalUserInfo->LocalPlayerIndex, *Error.ToString());

	// 如果状态不对，就中止，因为我们可能被取消了
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// 如果初始登录失败，或我们最终完全登出，就设为完全失败
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(Params.PlatformUser, Params.OnlineContext);
	if (NewStatus == ELoginStatusType::NotLoggedIn || LocalUserInfo->InitializationState == ECommonUserInitializationState::DoingInitialLogin)
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::FailedtoLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::LoggedInLocalOnly;
	}

	FText TitleText = NSLOCTEXT("CommonUser", "LoginFailedTitle", "Login Failure");

	if (!Params.bSuppressLoginErrors)
	{
		SendSystemMessage(FCommonUserTags::SystemMessage_Error_InitializeLocalPlayerFailed, TitleText, Error);
	}
	
	// 调用回调
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
}

void UCommonUserSubsystem::HandleUserInitializeSucceeded(FCommonUserInitializeParams Params)
{
	UCommonUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// 自这个被调度以来，用户信息已被重置
		return;
	}

	// 如果状态不对，就中止，因为我们可能被取消了
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// 修正状态
	if (Params.RequestedPrivilege == ECommonUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::LoggedInOnline;
	}
	else
	{
		LocalUserInfo->InitializationState = ECommonUserInitializationState::LoggedInLocalOnly;
	}

	ensure(LocalUserInfo->GetPrivilegeAvailability(Params.RequestedPrivilege) == ECommonUserAvailability::NowAvailable);

	// 调用回调
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
}

bool UCommonUserSubsystem::LoginLocalUser(const UCommonUserInfo* UserInfo, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete)
{
	UCommonUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	if (!ensure(UserInfo))
	{
		return false;
	}

	TSharedRef<FUserLoginRequest> NewRequest = MakeShared<FUserLoginRequest>(LocalUserInfo, RequestedPrivilege, Context, MoveTemp(OnComplete));
	ActiveLoginRequests.Add(NewRequest);

	// 这会执行回调或启动登录流程
	ProcessLoginRequest(NewRequest);

	return true;
}

void UCommonUserSubsystem::ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request)
{
	// 首先，看看我们是否已完全登录
	UCommonUserInfo* UserInfo = Request->UserInfo.Get();

	if (!UserInfo)
	{
		// 用户已不存在，直接删除这个请求
		ActiveLoginRequests.Remove(Request);

		return;
	}

	const FPlatformUserId PlatformUser = UserInfo->GetPlatformUserId();

	// 如果平台用户 ID 无效（因为这是游客），就直接跳到失败
	if (!IsRealPlatformUser(PlatformUser))
	{
#if COMMONUSER_OSSV1
		Request->Error = FOnlineError(NSLOCTEXT("CommonUser", "InvalidPlatformUser", "Invalid Platform User"));
#else
		Request->Error = UE::Online::Errors::InvalidUser();
#endif
		// 从活动数组中移除
		ActiveLoginRequests.Remove(Request);

		// 如果已绑定，就执行委托
		Request->Delegate.ExecuteIfBound(UserInfo, ELoginStatusType::NotLoggedIn, FUniqueNetIdRepl(), Request->Error, Request->DesiredContext);

		return;
	}

	// 先判断要处理哪个上下文
	if (Request->CurrentContext == ECommonUserOnlineContext::Invalid)
	{
		// 如果是游戏登录，先从平台上下文开始
		if (Request->DesiredContext == ECommonUserOnlineContext::Game)
		{
			Request->CurrentContext = ResolveOnlineContext(ECommonUserOnlineContext::PlatformOrDefault);
		}
		else
		{
			Request->CurrentContext = ResolveOnlineContext(Request->DesiredContext);
		}
	}

	ELoginStatusType CurrentStatus = GetLocalUserLoginStatus(PlatformUser, Request->CurrentContext);
	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	FOnlineContextCache* System = GetContextCache(Request->CurrentContext);

	if (!ensure(System))
	{
		return;
	}

	// 启动一个新的请求
	if (Request->OverallLoginState == ECommonUserAsyncTaskState::NotStarted)
	{
		Request->OverallLoginState = ECommonUserAsyncTaskState::InProgress;
	}

	bool bHasRequiredStatus = (CurrentStatus == ELoginStatusType::LoggedIn);
	if (Request->DesiredPrivilege == ECommonUserPrivilege::CanPlay)
	{
		// 如果这不是需要在线功能的登录，就允许本地配置也算作完全登录
		bHasRequiredStatus |= (CurrentStatus == ELoginStatusType::UsingLocalProfile);
	}

	// 检查总体是否成功
	if (bHasRequiredStatus && CurrentId.IsValid())
	{
		// 如果我们在等登录 UI 关闭，就暂停
		if (Request->LoginUIState == ECommonUserAsyncTaskState::InProgress)
		{
			return;
		}

		Request->OverallLoginState = ECommonUserAsyncTaskState::Done;
	}
	else
	{
		// 尝试使用平台 auth 登录
		if (Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::NotStarted)
		{
			Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::InProgress;

			if (TransferPlatformAuth(System, Request, PlatformUser))
			{
				return;
			}
			// 我们没有启动登录尝试，所以设为失败
			Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::Failed;
		}

		// 接下来检查 AutoLogin
		if (Request->AutoLoginState == ECommonUserAsyncTaskState::NotStarted)
		{
			if (Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::Done || Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::Failed)
			{
				Request->AutoLoginState = ECommonUserAsyncTaskState::InProgress;

				// 尝试用默认凭据自动登录，这在很多平台上都有效
				if (AutoLogin(System, Request, PlatformUser))
				{
					return;
				}
				// 我们没有启动自动登录尝试，所以设为失败
				Request->AutoLoginState = ECommonUserAsyncTaskState::Failed;
			}
		}

		// 接下来检查登录 UI
		if (Request->LoginUIState == ECommonUserAsyncTaskState::NotStarted)
		{
			if ((Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::Done || Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::Failed)
				&& (Request->AutoLoginState == ECommonUserAsyncTaskState::Done || Request->AutoLoginState == ECommonUserAsyncTaskState::Failed))
			{
				Request->LoginUIState = ECommonUserAsyncTaskState::InProgress;

				if (ShowLoginUI(System, Request, PlatformUser))
				{
					return;
				}
				// 我们没有显示 UI，所以设为失败
				Request->LoginUIState = ECommonUserAsyncTaskState::Failed;
			}
		}
	}

	// 检查总体是否失败
	if (Request->LoginUIState == ECommonUserAsyncTaskState::Failed &&
		Request->AutoLoginState == ECommonUserAsyncTaskState::Failed &&
		Request->TransferPlatformAuthState == ECommonUserAsyncTaskState::Failed)
	{
		Request->OverallLoginState = ECommonUserAsyncTaskState::Failed;
	}
	else if (Request->OverallLoginState == ECommonUserAsyncTaskState::InProgress &&
		Request->LoginUIState != ECommonUserAsyncTaskState::InProgress &&
		Request->AutoLoginState != ECommonUserAsyncTaskState::InProgress &&
		Request->TransferPlatformAuthState != ECommonUserAsyncTaskState::InProgress)
	{
		// 如果没有任何子状态仍在进行中，但我们还没有成功登录，就把它标记为失败，以避免永远卡住
		Request->OverallLoginState = ECommonUserAsyncTaskState::Failed;
	}

	if (Request->OverallLoginState == ECommonUserAsyncTaskState::Done)
	{
		// 如果需要就做权限检查
		if (Request->PrivilegeCheckState == ECommonUserAsyncTaskState::NotStarted)
		{
			Request->PrivilegeCheckState = ECommonUserAsyncTaskState::InProgress;

			ECommonUserPrivilegeResult CachedResult = UserInfo->GetCachedPrivilegeResult(Request->DesiredPrivilege, Request->CurrentContext);
			if (CachedResult == ECommonUserPrivilegeResult::Available)
			{
				// 使用缓存的成功值
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Done;
			}
			else
			{
				if (QueryUserPrivilege(System, Request, PlatformUser))
				{
					return;
				}
				else
				{
#if !COMMONUSER_OSSV1
					// 临时处理，等 OSSv2 实现权限后再改
					CachedResult = ECommonUserPrivilegeResult::Available;
					Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Done;
#endif
				}
			}
		}

		if (Request->PrivilegeCheckState == ECommonUserAsyncTaskState::Failed)
		{
			// 把权限失败也算作登录失败
			Request->OverallLoginState = ECommonUserAsyncTaskState::Failed;
		}
		else if (Request->PrivilegeCheckState == ECommonUserAsyncTaskState::Done)
		{
			// 如果平台上下文已完成但仍需处理服务上下文，就接着做服务上下文
			ECommonUserOnlineContext ResolvedDesiredContext = ResolveOnlineContext(Request->DesiredContext);

			if (Request->OverallLoginState == ECommonUserAsyncTaskState::Done && Request->CurrentContext != ResolvedDesiredContext)
			{
				Request->CurrentContext = ResolvedDesiredContext;
				Request->OverallLoginState = ECommonUserAsyncTaskState::NotStarted;
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::NotStarted;
				Request->TransferPlatformAuthState = ECommonUserAsyncTaskState::NotStarted;
				Request->AutoLoginState = ECommonUserAsyncTaskState::NotStarted;
				Request->LoginUIState = ECommonUserAsyncTaskState::NotStarted;

				// 重新处理并立即返回
				ProcessLoginRequest(Request);
				return;
			}
		}
	}

	if (Request->PrivilegeCheckState == ECommonUserAsyncTaskState::InProgress)
	{
		// 暂停以等待它完成
		return;
	}

	// 如果完成，就移除并调用回调
	if (Request->OverallLoginState == ECommonUserAsyncTaskState::Done || Request->OverallLoginState == ECommonUserAsyncTaskState::Failed)
	{
		// 如果这已经在嵌套函数中发生过了，就跳过
		if (ActiveLoginRequests.Contains(Request))
		{
			// 如果没有设置错误，就添加一个通用错误
			if (Request->OverallLoginState == ECommonUserAsyncTaskState::Failed && !Request->Error.IsSet())
			{
	#if COMMONUSER_OSSV1
				Request->Error = FOnlineError(NSLOCTEXT("CommonUser", "FailedToRequest", "Failed to Request Login"));
	#else
				Request->Error = UE::Online::Errors::RequestFailure();
	#endif
			}

			// 从活动数组中移除
			ActiveLoginRequests.Remove(Request);

			// 如果已绑定，就执行委托
			Request->Delegate.ExecuteIfBound(UserInfo, CurrentStatus, CurrentId, Request->Error, Request->DesiredContext);
		}
	}
}

#if COMMONUSER_OSSV1
void UCommonUserSubsystem::HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& ErrorString, ECommonUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	FUniqueNetIdRepl NewId = FUniqueNetIdRepl(NetId);
	UE_LOG(LogCommonUser, Log, TEXT("Player login Completed - System:%s, UserIdx:%d, Successful:%d, NewStatus:%s, NewId:%s, ErrorIfAny:%s"),
		*GetOnlineSubsystemName(Context).ToString(),
		PlatformUserIndex,
		(int32)bWasSuccessful,
		ELoginStatus::ToString(NewStatus),
		*NewId.ToString(),
		*ErrorString);

	// 更新任何等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UCommonUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已不存在，直接删除这个请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// 在某些平台上，这会从登录 UI 带着一个失败被调用
			if (Request->AutoLoginState == ECommonUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? ECommonUserAsyncTaskState::Done : ECommonUserAsyncTaskState::Failed;
			}

			if (!bWasSuccessful)
			{
				Request->Error = FOnlineError(FText::FromString(ErrorString));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UCommonUserSubsystem::HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, ECommonUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);

	// 更新任何等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UCommonUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已不存在，直接删除这个请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// 查找第一个尝试在此上下文登录的用户
		if (Request->CurrentContext == Context && Request->LoginUIState == ECommonUserAsyncTaskState::InProgress)
		{
			if (LoggedInNetId.IsValid() && LoggedInNetId->IsValid() && Error.WasSuccessful())
			{
				// 实际登录的平台用户 ID 可能不是请求 UI 的那个，
				// 所以如果返回的 ID 实际上是有效的，就交换它们
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = ECommonUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = ECommonUserAsyncTaskState::Failed;
				Request->Error = Error;
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UCommonUserSubsystem::HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, ECommonUserPrivilege UserPrivilege, TWeakObjectPtr<UCommonUserInfo> CommonUserInfo, ECommonUserOnlineContext Context)
{
	// 只在用户仍然存在时处理
	UCommonUserInfo* UserInfo = CommonUserInfo.Get();

	if (!UserInfo)
	{
		return;
	}

	ECommonUserPrivilegeResult UserResult = ConvertOSSPrivilegeResult(Privilege, PrivilegeResults);

	// 更新用户的缓存值
	UpdateUserPrivilegeResult(UserInfo, UserPrivilege, UserResult, Context);

	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	// 如果这里返回断开连接，就更新连接状态
	if (UserResult == ECommonUserPrivilegeResult::NetworkConnectionUnavailable)
	{
		ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::NoNetworkConnection;
	}
	else if (UserResult == ECommonUserPrivilegeResult::Available && UserPrivilege == ECommonUserPrivilege::CanPlayOnline)
	{
		if (ContextCache->CurrentConnectionStatus == EOnlineServerConnectionStatus::NoNetworkConnection)
		{
			ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
		}
	}
		
	// 看看是否有登录请求在等这个
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == UserPrivilege && Request->PrivilegeCheckState == ECommonUserAsyncTaskState::InProgress)
		{
			if (UserResult == ECommonUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Failed;

				// 用英文组句，比如「（该用户不允许）去（玩这个游戏）」
				Request->Error = FOnlineError(FText::Format(NSLOCTEXT("CommonUser", "PrivilegeFailureFormat", "{0} to {1}"), GetPrivilegeResultDescription(UserResult), GetPrivilegeDescription(UserPrivilege)));
			}

			ProcessLoginRequest(Request);
		}
	}
}
#else

void UCommonUserSubsystem::HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser, ECommonUserOnlineContext Context)
{
	const bool bWasSuccessful = Result.IsOk();
	FAccountId NewId;
	if (bWasSuccessful)
	{
		NewId = Result.GetOkValue().AccountInfo->AccountId;
	}
	
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	UE_LOG(LogCommonUser, Log, TEXT("Player login Completed - System:%d, UserIdx:%d, Successful:%d, NewId:%s, ErrorIfAny:%s"),
		(int32)Context,
		PlatformUser.GetInternalId(),
		(int32)Result.IsOk(),
		*ToLogString(NewId),
		Result.IsError() ? *Result.GetErrorValue().GetLogString() : TEXT(""));

	// 更新任何等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UCommonUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已不存在，直接删除这个请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// 在某些平台上，这会从登录 UI 带着一个失败被调用
			if (Request->AutoLoginState == ECommonUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? ECommonUserAsyncTaskState::Done : ECommonUserAsyncTaskState::Failed;
			}

			if (bWasSuccessful)
			{
				Request->Error.Reset();
			}
			else
			{
				Request->Error = Result.GetErrorValue();
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UCommonUserSubsystem::HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser, ECommonUserOnlineContext Context)
{
	// 更新任何等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UCommonUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已不存在，直接删除这个请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// 查找第一个尝试在此上下文登录的用户
		if (Request->CurrentContext == Context && Request->LoginUIState == ECommonUserAsyncTaskState::InProgress)
		{
			if (Result.IsOk())
			{
				// 实际登录的平台用户 ID 可能不是请求 UI 的那个，
				// 所以如果返回的 ID 实际上是有效的，就交换它们
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = ECommonUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = ECommonUserAsyncTaskState::Failed;
				Request->Error = Result.GetErrorValue();
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UCommonUserSubsystem::HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UCommonUserInfo> CommonUserInfo, EUserPrivileges DesiredPrivilege, ECommonUserOnlineContext Context)
{
	// 只在用户仍然存在时处理
	UCommonUserInfo* UserInfo = CommonUserInfo.Get();
	if (!UserInfo)
	{
		return;
	}

	ECommonUserPrivilege UserPrivilege = ConvertOnlineServicesPrivilege(DesiredPrivilege);
	ECommonUserPrivilegeResult UserResult = ECommonUserPrivilegeResult::PlatformFailure;
	if (const FQueryUserPrivilege::Result* OkResult = Result.TryGetOkValue())
	{
		UserResult = ConvertOnlineServicesPrivilegeResult(DesiredPrivilege, OkResult->PrivilegeResult);
	}
	else
	{
		UE_LOG(LogCommonUser, Warning, TEXT("QueryUserPrivilege failed: %s"), *Result.GetErrorValue().GetLogString());
	}

	// 更新用户的缓存值
	UserInfo->UpdateCachedPrivilegeResult(UserPrivilege, UserResult, Context);

	// 看看是否有登录请求在等这个
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == UserPrivilege && Request->PrivilegeCheckState == ECommonUserAsyncTaskState::InProgress)
		{
			if (UserResult == ECommonUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = ECommonUserAsyncTaskState::Failed;
				Request->Error = Result.IsError() ? Result.GetErrorValue() : UE::Online::Errors::Unknown();
			}

			ProcessLoginRequest(Request);
		}
	}
}
#endif // COMMONUSER_OSSV1

void UCommonUserSubsystem::RefreshLocalUserInfo(UCommonUserInfo* UserInfo)
{
	if (ensure(UserInfo))
	{
		// 始终更新 default
		UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, ECommonUserOnlineContext::Default), ECommonUserOnlineContext::Default);

		if (HasSeparatePlatformContext())
		{
			// 也更新 platform
			UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, ECommonUserOnlineContext::Platform), ECommonUserOnlineContext::Platform);
		}
	}
}

void UCommonUserSubsystem::HandleChangedAvailability(UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability)
{
	ECommonUserAvailability NewAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	if (OldAvailability != NewAvailability)
	{
		OnUserPrivilegeChanged.Broadcast(UserInfo, Privilege, OldAvailability, NewAvailability);
	}
}

void UCommonUserSubsystem::UpdateUserPrivilegeResult(UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context)
{
	check(UserInfo);
	
	ECommonUserAvailability OldAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	UserInfo->UpdateCachedPrivilegeResult(Privilege, Result, Context);

	HandleChangedAvailability(UserInfo, Privilege, OldAvailability);
}

#if COMMONUSER_OSSV1
ECommonUserPrivilege UCommonUserSubsystem::ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return ECommonUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return ECommonUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateOnline:
		return ECommonUserPrivilege::CanCommunicateViaTextOnline; // No good thing to do here, just mapping to text.
	case EUserPrivileges::CanUseUserGeneratedContent:
		return ECommonUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanUserCrossPlay:
		return ECommonUserPrivilege::CanUseCrossPlay;
	default:
		return ECommonUserPrivilege::Invalid_Count;
	}
}

EUserPrivileges::Type UCommonUserSubsystem::ConvertOSSPrivilege(ECommonUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case ECommonUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case ECommonUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case ECommonUserPrivilege::CanCommunicateViaTextOnline:
	case ECommonUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateOnline;
	case ECommonUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case ECommonUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanUserCrossPlay;
	default:
		// 没有失败类型，返回 CanPlay
		return EUserPrivileges::CanPlay;
	}
}

ECommonUserPrivilegeResult UCommonUserSubsystem::ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const
{
	// V1 的结果枚举是一个位字段，每个平台的行为都略有不同
	if (Results == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		return ECommonUserPrivilegeResult::Available;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotLoggedIn))
	{
		return ECommonUserPrivilegeResult::UserNotLoggedIn;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate))
	{
		return ECommonUserPrivilegeResult::VersionOutdated;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure)
	{
		return ECommonUserPrivilegeResult::AgeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure)
	{
		return ECommonUserPrivilegeResult::AccountTypeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::NetworkConnectionUnavailable)
	{
		return ECommonUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// 把其他账户失败归为一类
	uint32 AccountUseFailures = (uint32)IOnlineIdentity::EPrivilegeResults::OnlinePlayRestricted 
		| (uint32)IOnlineIdentity::EPrivilegeResults::UGCRestriction 
		| (uint32)IOnlineIdentity::EPrivilegeResults::ChatRestriction;

	if (Results & AccountUseFailures)
	{
		return ECommonUserPrivilegeResult::AccountUseRestricted;
	}

	// 如果完全无法游玩，就是许可失败
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return ECommonUserPrivilegeResult::LicenseInvalid;
	}

	// 未知原因
	return ECommonUserPrivilegeResult::PlatformFailure;
}
#else
ECommonUserPrivilege UCommonUserSubsystem::ConvertOnlineServicesPrivilege(EUserPrivileges Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return ECommonUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return ECommonUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateViaTextOnline:
		return ECommonUserPrivilege::CanCommunicateViaTextOnline;
	case EUserPrivileges::CanCommunicateViaVoiceOnline:
		return ECommonUserPrivilege::CanCommunicateViaVoiceOnline;
	case EUserPrivileges::CanUseUserGeneratedContent:
		return ECommonUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanCrossPlay:
		return ECommonUserPrivilege::CanUseCrossPlay;
	default:
		return ECommonUserPrivilege::Invalid_Count;
	}
}

EUserPrivileges UCommonUserSubsystem::ConvertOnlineServicesPrivilege(ECommonUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case ECommonUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case ECommonUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case ECommonUserPrivilege::CanCommunicateViaTextOnline:
		return EUserPrivileges::CanCommunicateViaTextOnline;
	case ECommonUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateViaVoiceOnline;
	case ECommonUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case ECommonUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanCrossPlay;
	default:
		// 没有失败类型，返回 CanPlay
		return EUserPrivileges::CanPlay;
	}
}

ECommonUserPrivilegeResult UCommonUserSubsystem::ConvertOnlineServicesPrivilegeResult(EUserPrivileges Privilege, EPrivilegeResults Results) const
{
	// V1 的结果枚举是一个位字段，每个平台的行为都略有不同
	if (Results == EPrivilegeResults::NoFailures)
	{
		return ECommonUserPrivilegeResult::Available;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::UserNotFound | EPrivilegeResults::UserNotLoggedIn))
	{
		return ECommonUserPrivilegeResult::UserNotLoggedIn;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::RequiredPatchAvailable | EPrivilegeResults::RequiredSystemUpdate))
	{
		return ECommonUserPrivilegeResult::VersionOutdated;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::AgeRestrictionFailure))
	{
		return ECommonUserPrivilegeResult::AgeRestricted;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::AccountTypeFailure))
	{
		return ECommonUserPrivilegeResult::AccountTypeRestricted;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::NetworkConnectionUnavailable))
	{
		return ECommonUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// 把其他账户失败归为一类
	const EPrivilegeResults AccountUseFailures = EPrivilegeResults::OnlinePlayRestricted
		| EPrivilegeResults::UGCRestriction
		| EPrivilegeResults::ChatRestriction;

	if (EnumHasAnyFlags(Results, AccountUseFailures))
	{
		return ECommonUserPrivilegeResult::AccountUseRestricted;
	}

	// 如果完全无法游玩，就是许可失败
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return ECommonUserPrivilegeResult::LicenseInvalid;
	}

	// 未知原因
	return ECommonUserPrivilegeResult::PlatformFailure;
}
#endif // COMMONUSER_OSSV1

FString UCommonUserSubsystem::PlatformUserIdToString(FPlatformUserId UserId)
{
	if (UserId == PLATFORMUSERID_NONE)
	{
		return TEXT("None");
	}
	else
	{
		return FString::Printf(TEXT("%d"), UserId.GetInternalId());
	}
}

FString UCommonUserSubsystem::ECommonUserOnlineContextToString(ECommonUserOnlineContext Context)
{
	switch (Context)
	{
	case ECommonUserOnlineContext::Game:
		return TEXT("Game");
	case ECommonUserOnlineContext::Default:
		return TEXT("Default");
	case ECommonUserOnlineContext::Service:
		return TEXT("Service");
	case ECommonUserOnlineContext::ServiceOrDefault:
		return TEXT("Service/Default");
	case ECommonUserOnlineContext::Platform:
		return TEXT("Platform");
	case ECommonUserOnlineContext::PlatformOrDefault:
		return TEXT("Platform/Default");
	default:
		return TEXT("Invalid");
	}
}

FText UCommonUserSubsystem::GetPrivilegeDescription(ECommonUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case ECommonUserPrivilege::CanPlay:
		return NSLOCTEXT("CommonUser", "PrivilegeCanPlay", "play the game");
	case ECommonUserPrivilege::CanPlayOnline:
		return NSLOCTEXT("CommonUser", "PrivilegeCanPlayOnline", "play online");
	case ECommonUserPrivilege::CanCommunicateViaTextOnline:
		return NSLOCTEXT("CommonUser", "PrivilegeCanCommunicateViaTextOnline", "communicate with text");
	case ECommonUserPrivilege::CanCommunicateViaVoiceOnline:
		return NSLOCTEXT("CommonUser", "PrivilegeCanCommunicateViaVoiceOnline", "communicate with voice");
	case ECommonUserPrivilege::CanUseUserGeneratedContent:
		return NSLOCTEXT("CommonUser", "PrivilegeCanUseUserGeneratedContent", "access user content");
	case ECommonUserPrivilege::CanUseCrossPlay:
		return NSLOCTEXT("CommonUser", "PrivilegeCanUseCrossPlay", "play with other platforms");
	default:
		return NSLOCTEXT("CommonUser", "PrivilegeInvalid", "Invalid");
	}
}

FText UCommonUserSubsystem::GetPrivilegeResultDescription(ECommonUserPrivilegeResult Result) const
{
	// TODO 这些字符串可能有认证要求，我们需要按主机平台分别覆盖
	switch (Result)
	{
	case ECommonUserPrivilegeResult::Unknown:
		return NSLOCTEXT("CommonUser", "ResultUnknown", "Unknown if the user is allowed");
	case ECommonUserPrivilegeResult::Available:
		return NSLOCTEXT("CommonUser", "ResultAvailable", "The user is allowed");
	case ECommonUserPrivilegeResult::UserNotLoggedIn:
		return NSLOCTEXT("CommonUser", "ResultUserNotLoggedIn", "The user must login");
	case ECommonUserPrivilegeResult::LicenseInvalid:
		return NSLOCTEXT("CommonUser", "ResultLicenseInvalid", "A valid game license is required");
	case ECommonUserPrivilegeResult::VersionOutdated:
		return NSLOCTEXT("CommonUser", "VersionOutdated", "The game or hardware needs to be updated");
	case ECommonUserPrivilegeResult::NetworkConnectionUnavailable:
		return NSLOCTEXT("CommonUser", "ResultNetworkConnectionUnavailable", "A network connection is required");
	case ECommonUserPrivilegeResult::AgeRestricted:
		return NSLOCTEXT("CommonUser", "ResultAgeRestricted", "This age restricted account is not allowed");
	case ECommonUserPrivilegeResult::AccountTypeRestricted:
		return NSLOCTEXT("CommonUser", "ResultAccountTypeRestricted", "This account type does not have access");
	case ECommonUserPrivilegeResult::AccountUseRestricted:
		return NSLOCTEXT("CommonUser", "ResultAccountUseRestricted", "This account is not allowed");
	case ECommonUserPrivilegeResult::PlatformFailure:
		return NSLOCTEXT("CommonUser", "ResultPlatformFailure", "Not allowed");
	default:
		return NSLOCTEXT("CommonUser", "ResultInvalid", "Invalid");

	}
}

const UCommonUserSubsystem::FOnlineContextCache* UCommonUserSubsystem::GetContextCache(ECommonUserOnlineContext Context) const
{
	return const_cast<UCommonUserSubsystem*>(this)->GetContextCache(Context);
}

UCommonUserSubsystem::FOnlineContextCache* UCommonUserSubsystem::GetContextCache(ECommonUserOnlineContext Context)
{
	switch (Context)
	{
	case ECommonUserOnlineContext::Game:
	case ECommonUserOnlineContext::Default:
		return DefaultContextInternal;

	case ECommonUserOnlineContext::Service:
		return ServiceContextInternal;
	case ECommonUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? ServiceContextInternal : DefaultContextInternal;

	case ECommonUserOnlineContext::Platform:
		return PlatformContextInternal;
	case ECommonUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? PlatformContextInternal : DefaultContextInternal;
	}

	return nullptr;
}

ECommonUserOnlineContext UCommonUserSubsystem::ResolveOnlineContext(ECommonUserOnlineContext Context) const
{
	switch (Context)
	{
	case ECommonUserOnlineContext::Game:
	case ECommonUserOnlineContext::Default:
		return ECommonUserOnlineContext::Default;

	case ECommonUserOnlineContext::Service:
		return ServiceContextInternal ? ECommonUserOnlineContext::Service : ECommonUserOnlineContext::Invalid;
	case ECommonUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? ECommonUserOnlineContext::Service : ECommonUserOnlineContext::Default;

	case ECommonUserOnlineContext::Platform:
		return PlatformContextInternal ? ECommonUserOnlineContext::Platform : ECommonUserOnlineContext::Invalid;
	case ECommonUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? ECommonUserOnlineContext::Platform : ECommonUserOnlineContext::Default;
	}

	return  ECommonUserOnlineContext::Invalid;
}

bool UCommonUserSubsystem::HasSeparatePlatformContext() const
{
	ECommonUserOnlineContext ServiceType = ResolveOnlineContext(ECommonUserOnlineContext::ServiceOrDefault);
	ECommonUserOnlineContext PlatformType = ResolveOnlineContext(ECommonUserOnlineContext::PlatformOrDefault);

	if (ServiceType != PlatformType)
	{
		return true;
	}
	return false;
}

void UCommonUserSubsystem::SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UCommonUserInfo* UserInfo)
{
	if (!bIsDedicatedServer && ensure(LocalPlayer && UserInfo))
	{
		LocalPlayer->SetPlatformUserId(UserInfo->GetPlatformUserId());

		FUniqueNetIdRepl NetId = UserInfo->GetNetId(ECommonUserOnlineContext::Game);
		LocalPlayer->SetCachedUniqueNetId(NetId);

		// 如果可能，也更新玩家状态
		APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
		if (PlayerController && PlayerController->PlayerState)
		{
			PlayerController->PlayerState->SetUniqueId(NetId);
		}
	}
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const
{
	TObjectPtr<UCommonUserInfo> const* Found = LocalUserInfos.Find(LocalPlayerIndex);
	if (Found)
	{
		return *Found;
	}
	return nullptr;
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	return GetUserInfoForPlatformUser(PlatformUser);
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return nullptr;
	}

	for (TPair<int32, UCommonUserInfo*> Pair : LocalUserInfos)
	{
		// 这个检查里不要包含游客用户
		if (ensure(Pair.Value) && Pair.Value->PlatformUser == PlatformUser && !Pair.Value->bIsGuest)
		{
			return Pair.Value;
		}
	}

	return nullptr;
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const
{
	if (!NetId.IsValid())
	{
		// TODO 在移动平台上，netID 无效时，我们是否需要处理登录前的情况？
		return nullptr;
	}

	for (TPair<int32, UCommonUserInfo*> UserPair : LocalUserInfos)
	{
		if (ensure(UserPair.Value))
		{
			for (const TPair<ECommonUserOnlineContext, UCommonUserInfo::FCachedData>& CachedPair : UserPair.Value->CachedDataMap)
			{
				if (NetId == CachedPair.Value.CachedNetId)
				{
					return UserPair.Value;
				}
			}
		}
	}

	return nullptr;
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForControllerId(int32 ControllerId) const
{
	FPlatformUserId PlatformUser;
	FInputDeviceId IgnoreDevice;

	IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUser, IgnoreDevice);

	return GetUserInfoForPlatformUser(PlatformUser);
}

const UCommonUserInfo* UCommonUserSubsystem::GetUserInfoForInputDevice(FInputDeviceId InputDevice) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForInputDevice(InputDevice);
	return GetUserInfoForPlatformUser(PlatformUser);
}

bool UCommonUserSubsystem::IsRealPlatformUserIndex(int32 PlatformUserIndex) const
{
	if (PlatformUserIndex < 0)
	{
		return false;
	}

#if COMMONUSER_OSSV1
	if (PlatformUserIndex >= MAX_LOCAL_PLAYERS)
	{
		// 与 OSS 的数量进行核对
		return false;
	}
#else
	// TODO:  OSSv2 定义 MAX_LOCAL_PLAYERS？
#endif

	if (PlatformUserIndex > 0 && GetTraitTags().HasTag(FCommonUserTags::Platform_Trait_SingleOnlineUser))
	{
		return false;
	}

	return true;
}

bool UCommonUserSubsystem::IsRealPlatformUser(FPlatformUserId PlatformUser) const
{
	// 校验在转换/分配时完成，所以信任类型
	if (!PlatformUser.IsValid())
	{
		return false;
	}

	// TODO: 用某种方式对照 OSS 或输入映射器进行校验

	if (GetTraitTags().HasTag(FCommonUserTags::Platform_Trait_SingleOnlineUser))
	{
		// 只有默认用户支持在线功能 
		if (PlatformUser != IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser())
		{
			return false;
		}
	}

	return true;
}

FPlatformUserId UCommonUserSubsystem::GetPlatformUserIdForIndex(int32 PlatformUserIndex) const
{
	return IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(PlatformUserIndex);
}

int32 UCommonUserSubsystem::GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetUserIndexForPlatformUser(PlatformUser);
}

FPlatformUserId UCommonUserSubsystem::GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const
{
	return IPlatformInputDeviceMapper::Get().GetUserForInputDevice(InputDevice);
}

FInputDeviceId UCommonUserSubsystem::GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PlatformUser);
}

void UCommonUserSubsystem::SetTraitTags(const FGameplayTagContainer& InTags)
{
	CachedTraitTags = InTags;
}

bool UCommonUserSubsystem::ShouldWaitForStartInput() const
{
	// 默认情况下，如果这是单用户平台，就不等待输入
	return !HasTraitTag(FCommonUserTags::Platform_Trait_SingleOnlineUser.GetTag());
}

#if COMMONUSER_OSSV1
void UCommonUserSubsystem::HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, ECommonUserOnlineContext Context)
{
	UE_LOG(LogCommonUser, Log, TEXT("Player login status changed - System:%s, UserIdx:%d, OldStatus:%s, NewStatus:%s, NewId:%s"),
		*GetOnlineSubsystemName(Context).ToString(),
		PlatformUserIndex,
		ELoginStatus::ToString(OldStatus),
		ELoginStatus::ToString(NewStatus),
		*NewId.ToString());

	if (NewStatus == ELoginStatus::NotLoggedIn && OldStatus != ELoginStatus::NotLoggedIn)
	{
		FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
		LogOutLocalUser(PlatformUser);
	}
}

void UCommonUserSubsystem::HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser)
{
	UE_LOG(LogCommonUser, Log, TEXT("Player controller pairing changed - UserIdx:%d, PreviousUser:%s, NewUser:%s"),
		PlatformUserIndex,
		*ToDebugString(PreviousUser),
		*ToDebugString(NewUser));

	UGameInstance* GameInstance = GetGameInstance();
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ULocalPlayer* ControlledLocalPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(PlatformUser);
	ULocalPlayer* NewLocalPlayer = GameInstance->FindLocalPlayerFromUniqueNetId(NewUser.User);
	const UCommonUserInfo* NewUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(NewUser.User));
	const UCommonUserInfo* PreviousUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(PreviousUser.User));

	// 看看我们是否认为这已经绑定到了现有玩家	
	if (PreviousUser.ControllersRemaining == 0 && PreviousUserInfo && PreviousUserInfo != NewUserInfo)
	{
		// 这意味着用户是故意通过平台接口登出的
		if (IsRealPlatformUser(PlatformUser))
		{
			LogOutLocalUser(PlatformUser);
		}
	}

	if (ControlledLocalPlayer && ControlledLocalPlayer != NewLocalPlayer)
	{
		// TODO 目前调用这个委托的平台其实并没有真正处理控制器 ID 的交换
		// SetLocalPlayerUserIndex(ControlledLocalPlayer, -1);
	}
}

void UCommonUserSubsystem::HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, ECommonUserOnlineContext Context)
{
	UE_LOG(LogCommonUser, Log, TEXT("HandleNetworkConnectionStatusChanged(ServiceName: %s, LastStatus: %s, ConnectionStatus: %s)"),
		*ServiceName,
		EOnlineServerConnectionStatus::ToString(LastConnectionStatus),
		EOnlineServerConnectionStatus::ToString(ConnectionStatus));

	// 缓存当前用户的旧可用性
	TMap<UCommonUserInfo*, ECommonUserAvailability> AvailabilityMap;

	for (TPair<int32, UCommonUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(ECommonUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// 服务名通常与 OSS 名相同，但在所有平台上并不一定
		System->CurrentConnectionStatus = ConnectionStatus;
	}

	for (TPair<UCommonUserInfo*, ECommonUserAvailability> Pair : AvailabilityMap)
	{
		// Notify other systems when someone goes online/offline
		HandleChangedAvailability(Pair.Key, ECommonUserPrivilege::CanPlayOnline, Pair.Value);
	}

}
#else
void UCommonUserSubsystem::HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, ECommonUserOnlineContext Context)
{
	UE_LOG(LogCommonUser, Log, TEXT("Player login status changed - System:%d, UserId:%s, NewStatus:%s"),
		(int)Context,
		*ToLogString(EventParameters.AccountInfo->AccountId),
		LexToString(EventParameters.LoginStatus));
}

void UCommonUserSubsystem::HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, ECommonUserOnlineContext Context)
{
	UE_LOG(LogCommonUser, Log, TEXT("HandleNetworkConnectionStatusChanged(Context:%d, ServiceName:%s, OldStatus:%s, NewStatus:%s)"),
		(int)Context,
		*EventParameters.ServiceName,
		LexToString(EventParameters.PreviousStatus),
		LexToString(EventParameters.CurrentStatus));

	// 缓存当前用户的旧可用性
	TMap<UCommonUserInfo*, ECommonUserAvailability> AvailabilityMap;

	for (TPair<int32, UCommonUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(ECommonUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// 服务名通常与 OSS 名相同，但在所有平台上并不一定
		System->CurrentConnectionStatus = EventParameters.CurrentStatus;
	}

	for (TPair<UCommonUserInfo*, ECommonUserAvailability> Pair : AvailabilityMap)
	{
		// Notify other systems when someone goes online/offline
		HandleChangedAvailability(Pair.Key, ECommonUserPrivilege::CanPlayOnline, Pair.Value);
	}
}
#endif // COMMONUSER_OSSV1

void UCommonUserSubsystem::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	FString InputDeviceIDString = FString::Printf(TEXT("%d"), InputDeviceId.GetId());
	const bool bIsConnected = NewConnectionState == EInputDeviceConnectionState::Connected;
	UE_LOG(LogCommonUser, Log, TEXT("Controller connection changed - UserIdx:%s, UserID:%s, Connected:%d"), *InputDeviceIDString, *PlatformUserIdToString(PlatformUserId), bIsConnected ? 1 : 0);

	// TODO 为支持此功能的平台实现
}


}



}

��
}


}



}

