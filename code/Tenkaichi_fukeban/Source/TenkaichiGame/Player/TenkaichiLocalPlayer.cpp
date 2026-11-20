// Copyright Epic Games, Inc. All Rights Reserved.

#if 0 // ⚠️【注释挂起】依赖 Settings 断点类（TenkaichiSettingsLocal/Shared，L3 CommonUser 相关未教），工程里无此文件 → 编译不过，先整体挂起。代码已一比一还原 Lyra，等 Settings 就位后放开注释。（已登记 `教学经验新对话必看\注释备忘录.md`）

#include "Player/TenkaichiLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Settings/TenkaichiSettingsLocal.h"
#include "Settings/TenkaichiSettingsShared.h"
#include "CommonUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiLocalPlayer)

class UObject;

UTenkaichiLocalPlayer::UTenkaichiLocalPlayer()
{
}

// 初始化后，注册"音频输出设备变更"监听。
void UTenkaichiLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (UTenkaichiSettingsLocal* LocalSettings = GetLocalSettings())
	{
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &UTenkaichiLocalPlayer::OnAudioOutputDeviceChanged);
	}
}

// 控制器切换时，重新绑定队伍监听。
void UTenkaichiLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

// 生成 PlayActor 时，重新绑定队伍监听。
bool UTenkaichiLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

// 在线会话初始化时，重新绑定队伍监听。
void UTenkaichiLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

// 关联控制器变化时，解绑旧控制器、绑定新控制器，并广播队伍变更。
void UTenkaichiLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	// 停止监听旧控制器的队伍变更。
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(LastBoundPC.Get()))
	{
		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	// 记录新控制器的队伍 ID，并监听它未来的队伍变更。
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(NewController))
	{
		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
		LastBoundPC = NewController;
	}

	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

// 空实现：LocalPlayer 只观察关联 PlayerController 的队伍，不自己设队。
void UTenkaichiLocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
}

// 返回关联 PlayerController 的队伍 ID。
FGenericTeamId UTenkaichiLocalPlayer::GetGenericTeamId() const
{
	if (ITenkaichiTeamAgentInterface* ControllerAsTeamProvider = Cast<ITenkaichiTeamAgentInterface>(PlayerController))
	{
		return ControllerAsTeamProvider->GetGenericTeamId();
	}
	else
	{
		return FGenericTeamId::NoTeam;
	}
}

// 返回队伍变更委托实例。
FOnTenkaichiTeamIndexChangedDelegate* UTenkaichiLocalPlayer::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

// 获取本地设置（进程启动时读配置，永远有效）。
UTenkaichiSettingsLocal* UTenkaichiLocalPlayer::GetLocalSettings() const
{
	return UTenkaichiSettingsLocal::Get();
}

// 获取共享设置（可能需登录后才加载真实存档，否则返回临时设置）。
UTenkaichiSettingsShared* UTenkaichiLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// 在 PC 上可以用同步加载，因为它只检查磁盘。
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;

		if (bCanLoadBeforeLogin)
		{
			SharedSettings = UTenkaichiSettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// 需要等用户登录才能拿到真实设置，先返回临时设置。
			SharedSettings = UTenkaichiSettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}

// 异步从磁盘加载共享设置（避免重复加载）。
void UTenkaichiLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
	if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings)
	{
		// 已经加载过一次，不重复加载。
		return;
	}

	ensure(UTenkaichiSettingsShared::AsyncLoadOrCreateSettings(this, UTenkaichiSettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &UTenkaichiLocalPlayer::OnSharedSettingsLoaded)));
}

// 共享设置加载完成后，替换当前设置对象。
void UTenkaichiLocalPlayer::OnSharedSettingsLoaded(UTenkaichiSettingsShared* LoadedOrCreatedSettings)
{
	// 设置到达前已被应用。
	if (ensure(LoadedOrCreatedSettings))
	{
		// 用这个替换临时或之前加载的对象，旧对象会正常被 GC。
		SharedSettings = LoadedOrCreatedSettings;

		NetIdForSharedSettings = GetCachedUniqueNetId();
	}
}

// 音频输出设备变更时，真正切换音频设备。
void UTenkaichiLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

// 音频设备切换完成回调。
void UTenkaichiLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}

// 关联控制器换队时，LocalPlayer 跟着广播队伍变更。
void UTenkaichiLocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

#endif // ⚠️【注释挂起结束】Settings 断点类就位后，删掉上面的 #if 0 和这个 #endif 即可放开。