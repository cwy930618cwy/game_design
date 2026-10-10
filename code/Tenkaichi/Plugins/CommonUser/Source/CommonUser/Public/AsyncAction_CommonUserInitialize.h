// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserSubsystem.h"
#include "Engine/CancellableAsyncAction.h"

#include "AsyncAction_CommonUserInitialize.generated.h"

#define UE_API COMMONUSER_API

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
struct FInputDeviceId;

class FText;
class UObject;
struct FFrame;

/**
 * 用于处理初始化用户的不同功能的异步动作
 */
UCLASS(MinimalAPI)
class UAsyncAction_CommonUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 使用通用用户系统初始化一个本地玩家，其中包括执行平台特定的登录和权限检查。
	 * 当流程成功或失败时，它会广播 OnInitializationComplete 委托。
	 *
	 * @param LocalPlayerIndex	期望的 ULocalPlayer 在 Game Instance 中的索引，0 表示主玩家，1+ 用于本地多人游戏
	 * @param PrimaryInputDevice 用户的主输入设备，若无效则使用系统默认
	 * @param bCanUseGuestLogin	若为 true，该玩家可以是不带真实系统网络 ID 的游客
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_CommonUserInitialize* InitializeForLocalPlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 尝试将现有用户登录到平台特定的在线后端，以启用完整的在线游玩
	 * 当流程成功或失败时，它会广播 OnInitializationComplete 委托。
	 *
	 * @param LocalPlayerIndex	Game Instance 中现有 LocalPlayer 的索引
	 */
	UFUNCTION(BlueprintCallable, Category = CommonUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_CommonUserInitialize* LoginForOnlinePlay(UCommonUserSubsystem* Target, int32 LocalPlayerIndex);

	/** 初始化成功或失败时调用 */
	UPROPERTY(BlueprintAssignable)
	FCommonUserOnInitializeCompleteMulticast OnInitializationComplete;

	/** 失败并在需要时发送回调 */
	UE_API void HandleFailure();

	/** 包装委托，在合适时传递到 OnInitializationComplete */
	UFUNCTION()
	UE_API virtual void HandleInitializationComplete(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

protected:
	/** 实际开始初始化 */
	UE_API virtual void Activate() override;

	TWeakObjectPtr<UCommonUserSubsystem> Subsystem;
	FCommonUserInitializeParams Params;
};

#undef UE_API
