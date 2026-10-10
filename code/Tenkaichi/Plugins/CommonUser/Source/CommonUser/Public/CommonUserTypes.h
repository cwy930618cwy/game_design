// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#if COMMONUSER_OSSV1

// 在线子系统（OSS v1）的 include 与前向声明
#include "OnlineSubsystemTypes.h"
class IOnlineSubsystem;
struct FOnlineError;
using FOnlineErrorType = FOnlineError;
using ELoginStatusType = ELoginStatus::Type;

#else

// 在线服务（OSS v2）的 include 与前向声明
#include "Online/Connectivity.h"
#include "Online/OnlineError.h"
namespace UE::Online
{
	enum class ELoginStatus : uint8;
	enum class EPrivilegeResults : uint32;
	enum class EUserPrivileges : uint8;
	using IAuthPtr = TSharedPtr<class IAuth>;
	using IOnlineServicesPtr = TSharedPtr<class IOnlineServices>;
	template <typename OpType>
	class TOnlineResult;
	struct FAuthLogin;
	struct FConnectionStatusChanged;
	struct FExternalUIShowLoginUI;
	struct FAuthLoginStatusChanged;
	struct FQueryUserPrivilege;
	struct FAccountInfo;
}
using FOnlineErrorType = UE::Online::FOnlineError;
using ELoginStatusType = UE::Online::ELoginStatus;

#endif

#include "CommonUserTypes.generated.h"


/** 用于指定在哪里以及如何执行在线查询的枚举 */
UENUM(BlueprintType)
enum class ECommonUserOnlineContext : uint8
{
	/** 从游戏代码调用，使用默认系统，但带有可能合并多个上下文结果的特殊处理 */
	Game,

	/** 引擎默认的在线系统，它始终存在，且等同于 Service 或 Platform 之一 */
	Default,
	
	/** 显式请求外部服务，该服务可能不存在 */
	Service,

	/** 先查找外部服务，找不到则回退到默认 */
	ServiceOrDefault,
	
	/** 显式请求平台系统，该系统可能不存在 */
	Platform,

	/** 先查找平台系统，找不到则回退到默认 */
	PlatformOrDefault,

	/** 无效系统 */
	Invalid
};

/** 描述某个用户初始化状态的枚举 */
UENUM(BlueprintType)
enum class ECommonUserInitializationState : uint8
{
	/** 用户尚未开始登录流程 */
	Unknown,

	/** 玩家正在通过本地登录获取用户 ID */
	DoingInitialLogin,

	/** 玩家正在执行网络登录，已经完成了本地登录 */
	DoingNetworkLogin,

	/** 玩家完全登录失败 */
	FailedtoLogin,

	
	/** 玩家已登录，并可以使用在线功能 */
	LoggedInOnline,

	/** 玩家已在本地登录（游客或真实用户均可），但无法执行在线操作 */
	LoggedInLocalOnly,


	/** 无效状态或用户 */
	Invalid,
};

/** 指定用户可用权限与能力范围的枚举 */
UENUM(BlueprintType)
enum class ECommonUserPrivilege : uint8
{
	/** 用户是否能够进行游玩，无论在线还是离线 */
	CanPlay,

	/** 用户是否能够游玩在线模式 */
	CanPlayOnline,

	/** 用户是否能够使用文字聊天 */
	CanCommunicateViaTextOnline,

	/** 用户是否能够使用语音聊天 */
	CanCommunicateViaVoiceOnline,

	/** 用户是否能够访问由其他用户生成的内容 */
	CanUseUserGeneratedContent,

	/** 用户是否能够参与跨平台联机 */
	CanUseCrossPlay,

	/** 无效权限（同时表示有效权限的数量） */
	Invalid_Count					UMETA(Hidden)
};

/** 指定某项功能或权限的总体可用性的枚举，综合了来自多个来源的信息 */
UENUM(BlueprintType)
enum class ECommonUserAvailability : uint8
{
	/** 状态完全未知，需要查询 */
	Unknown,

	/** 该功能现在完全可用 */
	NowAvailable,

	/** 该功能在正常登录流程完成后可能可用 */
	PossiblyAvailable,

	/** 该功能因网络连接等原因当前不可用，但未来可能可用 */
	CurrentlyUnavailable,

	/** 由于账户或平台的硬性限制，该功能在本会话剩余时间内将永远不可用 */
	AlwaysUnavailable,

	/** 无效功能 */
	Invalid,
};

/** 给出用户能否使用某项权限的具体原因的枚举 */
UENUM(BlueprintType)
enum class ECommonUserPrivilegeResult : uint8
{
	/** 状态未知，需要查询 */
	Unknown,

	/** 该权限完全可用 */
	Available,

	/** 用户尚未完全登录 */
	UserNotLoggedIn,

	/** 用户不拥有该游戏或内容 */
	LicenseInvalid,

	/** 游戏需要更新或打补丁后该权限才可用 */
	VersionOutdated,

	/** 无网络连接，重新连接后可能解决 */
	NetworkConnectionUnavailable,

	/** 家长控制限制 */
	AgeRestricted,

	/** 账户缺少所需的订阅或账户类型 */
	AccountTypeRestricted,

	/** 其他账户/用户限制，例如被服务封禁 */
	AccountUseRestricted,

	/** 其他平台特定的失败 */
	PlatformFailure,
};

/** 用于跟踪不同异步操作进度的枚举 */
enum class ECommonUserAsyncTaskState : uint8
{
	/** 任务尚未开始 */
	NotStarted,
	/** 任务正在处理中 */
	InProgress,
	/** 任务已成功完成 */
	Done,
	/** 任务未能完成 */
	Failed
};

/** 关于在线错误的详细信息，本质上是 FOnlineError 的封装。 */
USTRUCT(BlueprintType)
struct FOnlineResultInformation
{
	GENERATED_BODY()

	/** 操作是否成功。若成功，本结构体的错误字段将不包含额外信息。 */
	UPROPERTY(BlueprintReadOnly)
	bool bWasSuccessful = true;

	/** 唯一错误 ID，可用于与特定已处理的错误进行比较。 */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorId;

	/** 显示给用户的错误文本。 */
	UPROPERTY(BlueprintReadOnly)
	FText ErrorText;

	/**
	 * 从一个 FOnlineErrorType 初始化本结构
	 * @param InOnlineError 用于初始化的在线错误
	 */
	void COMMONUSER_API FromOnlineError(const FOnlineErrorType& InOnlineError);
};
