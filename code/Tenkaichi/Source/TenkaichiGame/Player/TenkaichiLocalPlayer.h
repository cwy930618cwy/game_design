// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonLocalPlayer.h"
#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiLocalPlayer.generated.h"

#define UE_API TENKAICHIGAME_API

struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class UTenkaichiSettingsLocal;
class UTenkaichiSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

/**
 * UTenkaichiLocalPlayer
 */
UCLASS(MinimalAPI)
class UTenkaichiLocalPlayer : public UCommonLocalPlayer, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiLocalPlayer();

	//~UObject 接口开始
	UE_API virtual void PostInitProperties() override;
	//~UObject 接口结束

	//~UPlayer 接口开始
	UE_API virtual void SwitchController(class APlayerController* PC) override;
	//~UPlayer 接口结束

	//~ULocalPlayer 接口开始
	UE_API virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	UE_API virtual void InitOnlineSession() override;
	//~ULocalPlayer 接口结束

	//~ITenkaichiTeamAgentInterface 接口开始
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~ITenkaichiTeamAgentInterface 接口结束

	// 获取本地设置（进程启动时从配置文件读，永远有效）。
	UFUNCTION()
	UE_API UTenkaichiSettingsLocal* GetLocalSettings() const;

	// 获取共享设置（用存档系统读，可能要在用户登录后才正确）。
	UFUNCTION()
	UE_API UTenkaichiSettingsShared* GetSharedSettings() const;

	// 启动一个异步请求来加载共享设置，加载或新建完成后会调用 OnSharedSettingsLoaded。
	UE_API void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
	// 共享设置加载完成后的回调。
	UE_API void OnSharedSettingsLoaded(UTenkaichiSettingsShared* LoadedOrCreatedSettings);

	// 音频输出设备变更时的回调。
	UE_API void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	UE_API void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	// 关联的 PlayerController 变化时的回调（重新绑定队伍监听）。
	UE_API void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	// 共享设置（临时持有，登录后加载真正存档替换）。
	UPROPERTY(Transient)
	mutable TObjectPtr<UTenkaichiSettingsShared> SharedSettings;

	// 记录已加载共享设置时的 NetId（避免重复加载）。
	FUniqueNetIdRepl NetIdForSharedSettings;

	// 输入映射上下文（本地玩家专属输入）。
	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	// 队伍变更委托实例。
	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 最后一次绑定的 PlayerController（用于取消旧监听）。
	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};

#undef UE_API