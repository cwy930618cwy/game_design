// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPlayerController.h"
#include "CommonInputTypeEnum.h"
#include "Components/PrimitiveComponent.h"
#include "TenkaichiLogChannels.h"
#include "TenkaichiCheatManager.h"
#include "TenkaichiPlayerState.h"
#include "Camera/TenkaichiPlayerCameraManager.h"
#include "UI/TenkaichiHUD.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "TenkaichiGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "TenkaichiLocalPlayer.h"
#include "GameModes/TenkaichiGameState.h"
#include "Settings/TenkaichiSettingsLocal.h"
#include "Settings/TenkaichiSettingsShared.h"
#include "Replays/TenkaichiReplaySubsystem.h"
#include "ReplaySubsystem.h"
#include "Development/TenkaichiDeveloperSettings.h"
#include "GameMapsSettings.h"
#if WITH_RPC_REGISTRY
#include "Tests/TenkaichiGameplayRpcRegistrationComponent.h"
#include "HttpServerModule.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerController)

namespace Tenkaichi
{
	namespace Input
	{
		// 是否总是播放力反馈（哪怕最后输入设备不是手柄）。默认 0 = 关。
		static int32 ShouldAlwaysPlayForceFeedback = 0;
		// 控制台变量：可运行时用 "TenkaichiPC.ShouldAlwaysPlayForceFeedback 1" 打开。
		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("TenkaichiPC.ShouldAlwaysPlayForceFeedback"),
			ShouldAlwaysPlayForceFeedback,
			TEXT("是否总是播放力反馈效果，哪怕最后输入设备不是手柄？"));
	}
}

// 构造函数：指定本控制器的相机管理器和作弊管理器类。
ATenkaichiPlayerController::ATenkaichiPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ATenkaichiPlayerCameraManager::StaticClass();

#if USING_CHEAT_MANAGER
	CheatClass = UTenkaichiCheatManager::StaticClass();
#endif // #if USING_CHEAT_MANAGER
}

// 组件预初始化（此处暂无额外逻辑，保留钩子供子类重写）。
void ATenkaichiPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

// 游戏开始：若启用 RPC 注册表则启动 HTTP 监听，并确保控制器不隐藏。
void ATenkaichiPlayerController::BeginPlay()
{
	Super::BeginPlay();
	#if WITH_RPC_REGISTRY
	FHttpServerModule::Get().StartAllListeners();
	int32 RpcPort = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("rpcport="), RpcPort))
	{
		UTenkaichiGameplayRpcRegistrationComponent* ObjectInstance = UTenkaichiGameplayRpcRegistrationComponent::GetInstance();
		if (ObjectInstance && ObjectInstance->IsValidLowLevel())
		{
			ObjectInstance->RegisterAlwaysOnHttpCallbacks();
			ObjectInstance->RegisterInMatchHttpCallbacks();
		}
	}
	#endif
	SetActorHiddenInGame(false);
}

// 结束：调用父类清理（暂无额外逻辑）。
void ATenkaichiPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// 声明网络复制属性：禁用父类的 TargetViewRotation 复制，改由 PlayerState 的 ReplicatedViewRotation 承担。
void ATenkaichiPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 关闭复制 PC 的 TargetViewRotation，因为它在回放或客户端旁观时不好用。
	// 引擎的 TargetViewRotation 只有在服务器提前知道旁观特定 Pawn 时才设置，且只对 COND_OwnerOnly 复制。
	// 在客户端保存的回放里 COND_OwnerOnly 永远不为 true，录制时也不总能确定目标 Pawn。
	// 为了支持客户端保存回放，这段复制移到了 ReplicatedViewRotation，并在 PlayerTick 里更新。
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

// 玩家连接上（暂无额外逻辑）。
void ATenkaichiPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

// 每帧驱动：自动奔跑时给 Pawn 加移动输入；并同步视角旋转到 PlayerState。
void ATenkaichiPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 如果正在自动奔跑，就给 Pawn 加些移动输入
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);
		}
	}

	ATenkaichiPlayerState* TenkaichiPlayerState = GetTenkaichiPlayerState();

	if (PlayerCameraManager && TenkaichiPlayerState)
	{
		APawn* TargetPawn = PlayerCameraManager->GetViewTargetPawn();

		if (TargetPawn)
		{
			// 在服务器上更新视角旋转，让它能复制下去
			if (HasAuthority() || TargetPawn->IsLocallyControlled())
			{
				TenkaichiPlayerState->SetReplicatedViewRotation(TargetPawn->GetViewRotation());
			}

			// 如果目标 Pawn 不是本地控制，就更新目标视角旋转
			if (!TargetPawn->IsLocallyControlled())
			{
				TenkaichiPlayerState = TargetPawn->GetPlayerState<ATenkaichiPlayerState>();
				if (TenkaichiPlayerState)
				{
					// 从被旁观 Pawn 的玩家状态取（可能与本 PC 的玩家状态不同）
					TargetViewRotation = TenkaichiPlayerState->GetReplicatedViewRotation();
				}
			}
		}
	}
}

// 获取本控制器的玩家状态（允许为空）。
ATenkaichiPlayerState* ATenkaichiPlayerController::GetTenkaichiPlayerState() const
{
	return CastChecked<ATenkaichiPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

// 获取本玩家身上的技能系统组件（从玩家状态身上取）。
UTenkaichiAbilitySystemComponent* ATenkaichiPlayerController::GetTenkaichiAbilitySystemComponent() const
{
	const ATenkaichiPlayerState* TenkaichiPS = GetTenkaichiPlayerState();
	return (TenkaichiPS ? TenkaichiPS->GetTenkaichiAbilitySystemComponent() : nullptr);
}

// 获取本控制器的 HUD（允许为空）。
ATenkaichiHUD* ATenkaichiPlayerController::GetTenkaichiHUD() const
{
	return CastChecked<ATenkaichiHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

// 尝试开始录制客户端回放：若应录制且是本地第一个玩家，则设置录制玩家状态并开始。
bool ATenkaichiPlayerController::TryToRecordClientReplay()
{
	// 看看是否应该录制回放
	if (ShouldRecordClientReplay())
	{
		if (UTenkaichiReplaySubsystem* ReplaySubsystem = GetGameInstance()->GetSubsystem<UTenkaichiReplaySubsystem>())
		{
			APlayerController* FirstLocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (FirstLocalPlayerController == this)
			{
				// 如果是第一个玩家，为本地回放更新旁观玩家，然后开始录制
				if (ATenkaichiGameState* GameState = Cast<ATenkaichiGameState>(GetWorld()->GetGameState()))
				{
					GameState->SetRecorderPlayerState(PlayerState);

					ReplaySubsystem->RecordClientReplay(this);
					return true;
				}
			}
		}
	}
	return false;
}

// 判断是否应录制客户端回放：不在主菜单、不在播放/录制中、是本地控制器、且本地设置允许。
bool ATenkaichiPlayerController::ShouldRecordClientReplay()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr &&
		World != nullptr &&
		!World->IsPlayingReplay() &&
		!World->IsRecordingClientReplay() &&
		NM_DedicatedServer != GetNetMode() &&
		IsLocalPlayerController())
	{
		FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
		FString CurrentMap = World->URL.Map;

#if WITH_EDITOR
		CurrentMap = UWorld::StripPIEPrefixFromPackageName(CurrentMap, World->StreamingLevelsPrefix);
#endif
		if (CurrentMap == DefaultMap)
		{
			// 永远不在默认前台地图录制演示（可用更好的"是否在主菜单"判断替代）
			return false;
		}

		if (UReplaySubsystem* ReplaySubsystem = GameInstance->GetSubsystem<UReplaySubsystem>())
		{
			if (ReplaySubsystem->IsRecording() || ReplaySubsystem->IsPlaying())
			{
				// 同一时间只能一个
				return false;
			}
		}

		// 若可行，再检查设置
		if (const UTenkaichiLocalPlayer* TenkaichiLocalPlayer = Cast<UTenkaichiLocalPlayer>(GetLocalPlayer()))
		{
			if (TenkaichiLocalPlayer->GetLocalSettings()->ShouldAutoRecordReplays())
			{
				return true;
			}
		}
	}
	return false;
}

// 队伍索引变化回调：把整数队伍 ID 转成 FGenericTeamId 后广播。
void ATenkaichiPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

// 玩家状态变更钩子（空实现，供派生类重写，不用挂所有其他事件）。
void ATenkaichiPlayerController::OnPlayerStateChanged()
{
	// 空，供派生类实现，无需挂载其他所有事件
}

// 广播玩家状态变更：解绑旧玩家状态的队伍委托、绑定新的，并广播队伍变化。
void ATenkaichiPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// 解绑旧的玩家状态（如果有）
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (ITenkaichiTeamAgentInterface* PlayerStateTeamInterface = Cast<ITenkaichiTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// 绑定新的玩家状态（如果有）
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (ITenkaichiTeamAgentInterface* PlayerStateTeamInterface = Cast<ITenkaichiTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}

	// 广播队伍变化（如果真的有变化）
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

// 初始化玩家状态：调用父类后广播玩家状态变更。
void ATenkaichiPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

// 清理玩家状态：调用父类后广播玩家状态变更。
void ATenkaichiPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

// 玩家状态复制到客户端时：广播变更，并尝试激活出生技能（处理 PC 复制晚于 PS/ASC 的情况）。
void ATenkaichiPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	// 当客户端连到远程服务器时，玩家控制器可能比 PlayerState 和 AbilitySystemComponent 复制得更晚。
	// 但 TryActivateAbilitiesOnSpawn 依赖玩家控制器已复制，才能判断出生技能是否应在本地执行。
	// 因此一旦 PC 存在并解析出 PlayerState，就再尝试一次激活出生技能。
	// 其他网络模式下 PC 不会复制得晚，LyraASC 自己的 TryActivateAbilitiesOnSpawn 调用会成功。
	// 这里的处理只针对"PlayerState 和 ASC 先于 PC 复制、误以为技能不是给本地玩家的"情况。
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (ATenkaichiPlayerState* TenkaichiPS = GetPlayerState<ATenkaichiPlayerState>())
		{
			if (UTenkaichiAbilitySystemComponent* TenkaichiASC = TenkaichiPS->GetTenkaichiAbilitySystemComponent())
			{
				TenkaichiASC->RefreshAbilityActorInfo();
				TenkaichiASC->TryActivateAbilitiesOnSpawn();
			}
		}
	}
}

// 设置本地玩家：绑定共享设置变更事件并立即应用一次。
void ATenkaichiPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	if (const UTenkaichiLocalPlayer* TenkaichiLocalPlayer = Cast<UTenkaichiLocalPlayer>(InPlayer))
	{
		UTenkaichiSettingsShared* UserSettings = TenkaichiLocalPlayer->GetSharedSettings();
		UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

		OnSettingsChanged(UserSettings);
	}
}

// 共享设置变更：根据"是否启用力反馈"更新本控制器的力反馈开关。
void ATenkaichiPlayerController::OnSettingsChanged(UTenkaichiSettingsShared* InSettings)
{
	bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

// 添加作弊：启用作弊管理器时强制为 true 调用父类。
void ATenkaichiPlayerController::AddCheats(bool bForce)
{
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

// 服务器执行单条作弊指令（RPC 实现）。
void ATenkaichiPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogTenkaichi, Warning, TEXT("ServerCheat: %s"), *Msg);
		ClientMessage(ConsoleCommand(Msg));
	}
#endif // #if USING_CHEAT_MANAGER
}

// 作弊指令校验（此处总是允许）。
bool ATenkaichiPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

// 服务器为所有玩家执行作弊指令（RPC 实现）。
void ATenkaichiPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogTenkaichi, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		for (TActorIterator<ATenkaichiPlayerController> It(GetWorld()); It; ++It)
		{
			ATenkaichiPlayerController* TenkaichiPC = (*It);
			if (TenkaichiPC)
			{
				TenkaichiPC->ClientMessage(TenkaichiPC->ConsoleCommand(Msg));
			}
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

// 所有玩家作弊指令校验（此处总是允许）。
bool ATenkaichiPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

// 输入处理前（暂无额外逻辑）。
void ATenkaichiPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

// 输入处理后：把输入转给技能系统处理技能输入。
void ATenkaichiPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

// 相机穿透目标时：标记下一帧隐藏 view target Pawn（防穿模）。
void ATenkaichiPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}

// 占据 Pawn 时：编辑器下按设置执行特定作弊，并关闭自动奔跑。
void ATenkaichiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		for (const FTenkaichiCheatToRun& CheatRow : GetDefault<UTenkaichiDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
			{
				ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
			}
		}
	}
#endif

	SetIsAutoRunning(false);
}

// 设置/关闭自动奔跑：与当前状态不同时才触发开始/结束。
void ATenkaichiPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

// 查询是否自动奔跑：看 ASC 上是否挂着"自动奔跑"标签。
bool ATenkaichiPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		bIsAutoRunning = TenkaichiASC->GetTagCount(TenkaichiGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

// 开始自动奔跑：给 ASC 挂"自动奔跑"标签并触发蓝图事件。
void ATenkaichiPlayerController::OnStartAutoRun()
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}
}

// 结束自动奔跑：移除 ASC 上的"自动奔跑"标签并触发蓝图事件。
void ATenkaichiPlayerController::OnEndAutoRun()
{
	if (UTenkaichiAbilitySystemComponent* TenkaichiASC = GetTenkaichiAbilitySystemComponent())
	{
		TenkaichiASC->SetLooseGameplayTagCount(TenkaichiGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}

// 更新力反馈：仅当启用且输入设备是手柄/触屏时播放，否则清零。
void ATenkaichiPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (Tenkaichi::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}

	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

// 更新隐藏组件：相机穿墙时隐藏 view target Pawn 的所有组件（含子物体）。
void ATenkaichiPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// 内部辅助函数：把所有组件（含附加的子物体）加进隐藏列表
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
			{
				// 添加每个组件以及所有附加的子物体
				for (UPrimitiveComponent* Comp : InComponents)
				{
					if (Comp->IsRegistered())
					{
						OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

						for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
						{
							static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
							UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
							if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
							{
								OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
							}
						}
					}
				}
			};

			//TODO 用接口解决：收集要隐藏的组件之类的。
			//TODO 隐藏不是最好的方案，有时你更想要"随距离淡出"的效果，这需要交给策划设计。

			// 隐藏 Pawn 的组件
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// 隐藏武器（暂时注释）
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}

		// 已消费，重置标志供下一帧
		bHideViewTargetPawnNextFrame = false;
	}
}

// 设置队伍 ID：控制器本身不能设，队伍由关联的玩家状态驱动，直接报错。
void ATenkaichiPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogTenkaichiTeams, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

// 查询队伍 ID：委托给关联的玩家状态。
FGenericTeamId ATenkaichiPlayerController::GetGenericTeamId() const
{
	if (const ITenkaichiTeamAgentInterface* PSWithTeamInterface = Cast<ITenkaichiTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

// 获取队伍变更委托：返回本控制器持有的委托。
FOnTenkaichiTeamIndexChangedDelegate* ATenkaichiPlayerController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

// 释放 Pawn 时：确保被释放的 Pawn 不再是 ASC 的 avatar。
void ATenkaichiPlayerController::OnUnPossess()
{
	// 确保被释放的 Pawn 不再作为我们 ASC 的 avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

//////////////////////////////////////////////////////////////////////
// ATenkaichiReplayPlayerController

// 每帧：若跟随的玩家状态失效，则重新监听录制玩家状态事件并跟随。
void ATenkaichiReplayPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 回放中 scrubbing 时状态可能随时失效
	if (!IsValid(FollowedPlayerState))
	{
		UWorld* World = GetWorld();

		// 同时监听录制和播放
		if (ATenkaichiGameState* GameState = Cast<ATenkaichiGameState>(World->GetGameState()))
		{
			if (!GameState->OnRecorderPlayerStateChangedEvent.IsBoundToObject(this))
			{
				GameState->OnRecorderPlayerStateChangedEvent.AddUObject(this, &ThisClass::RecorderPlayerStateUpdated);
			}
			if (APlayerState* RecorderState = GameState->GetRecorderPlayerState())
			{
				RecorderPlayerStateUpdated(RecorderState);
			}
		}
	}
}

// 平滑跟随目标视角旋转（默认行为已够用，保留钩子）。
void ATenkaichiReplayPlayerController::SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds)
{
	// 默认行为是插值到 TargetViewRotation（由 APlayerController::TickActor 设置），但不够平滑
	Super::SmoothTargetViewRotation(TargetPawn, DeltaSeconds);
}

// 回放控制器不录制回放，永远返回 false。
bool ATenkaichiReplayPlayerController::ShouldRecordClientReplay()
{
	return false;
}

// 录制玩家状态更新：跟随它，并监听其换 Pawn 事件。
void ATenkaichiReplayPlayerController::RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState)
{
	if (NewRecorderPlayerState)
	{
		FollowedPlayerState = NewRecorderPlayerState;

		// 绑定换 Pawn 事件并立即调用一次
		NewRecorderPlayerState->OnPawnSet.AddUniqueDynamic(this, &ATenkaichiReplayPlayerController::OnPlayerStatePawnSet);
		OnPlayerStatePawnSet(NewRecorderPlayerState, NewRecorderPlayerState->GetPawn(), nullptr);
	}
}

// 被跟随者换 Pawn：把视角切到新 Pawn。
void ATenkaichiReplayPlayerController::OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn)
{
	if (ChangedPlayerState == FollowedPlayerState)
	{
		SetViewTarget(NewPlayerPawn);
	}
}