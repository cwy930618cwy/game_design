// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiGameMode.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TenkaichiLogChannels.h"
#include "Misc/CommandLine.h"
//#include "System/TenkaichiAssetManager.h"   // ⚠️ 注释挂起：依赖 P14 AssetManager
//#include "TenkaichiGameState.h"             // ⚠️ 注释挂起：依赖 P13.8 GameState
//#include "System/TenkaichiGameSession.h"   // ⚠️ 注释挂起：依赖 P14 GameSession
#include "Player/TenkaichiPlayerController.h"
#include "Player/TenkaichiPlayerBotController.h"
#include "Player/TenkaichiPlayerState.h"
#include "Character/TenkaichiCharacter.h"
//#include "UI/TenkaichiHUD.h"                // ⚠️ 注释挂起：依赖 P16 HUD
#include "Character/TenkaichiPawnExtensionComponent.h"
#include "Character/TenkaichiPawnData.h"
#include "GameModes/TenkaichiWorldSettings.h"
#include "GameModes/TenkaichiExperienceDefinition.h"
#include "GameModes/TenkaichiExperienceManagerComponent.h"
//#include "GameModes/TenkaichiUserFacingExperienceDefinition.h"   // ⚠️ 注释挂起：依赖 P13.9 UserFacingExperience
#include "Kismet/GameplayStatics.h"
//#include "Development/TenkaichiDeveloperSettings.h"   // ⚠️ 注释挂起：依赖 P18 DeveloperSettings
#include "Player/TenkaichiPlayerSpawningManagerComponent.h"
#include "CommonUserSubsystem.h"
#include "CommonSessionSubsystem.h"
#include "TimerManager.h"
#include "GameMapsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameMode)

// 构造函数：指定 GameState/PlayerController/Pawn/HUD 等默认类。
ATenkaichiGameMode::ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//GameStateClass = ATenkaichiGameState::StaticClass();   // ⚠️ 注释挂起：P13.8 GameState
	//GameSessionClass = ATenkaichiGameSession::StaticClass();   // ⚠️ 注释挂起：P14 GameSession
	PlayerControllerClass = ATenkaichiPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = ATenkaichiReplayPlayerController::StaticClass();
	PlayerStateClass = ATenkaichiPlayerState::StaticClass();
	DefaultPawnClass = ATenkaichiCharacter::StaticClass();
	//HUDClass = ATenkaichiHUD::StaticClass();   // ⚠️ 注释挂起：P16 HUD
}

// 查指定控制器该用哪个 PawnData（先从 PlayerState 查，再回退到经验默认值）。
const UTenkaichiPawnData* ATenkaichiGameMode::GetPawnDataForController(const AController* InController) const
{
	// 看看 PawnData 是不是已经设置在 PlayerState 上了
	if (InController != nullptr)
	{
		if (const ATenkaichiPlayerState* TenkaichiPS = InController->GetPlayerState<ATenkaichiPlayerState>())
		{
			if (const UTenkaichiPawnData* PawnData = TenkaichiPS->GetPawnData<UTenkaichiPawnData>())
			{
				return PawnData;
			}
		}
	}

	// 如果没有，就回退到当前经验的默认值
	check(GameState);
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		const UTenkaichiExperienceDefinition* Experience = ExperienceComponent->GetCurrentExperienceChecked();
		if (Experience->DefaultPawnData != nullptr)
		{
			return Experience->DefaultPawnData;
		}

		// 经验加载完了但还是没有 PawnData，暂时回退到全局默认值
		//return UTenkaichiAssetManager::Get().GetDefaultPawnData();   // ⚠️ 注释挂起：P14 AssetManager
	}

	// 经验还没加载完，所以还没有 PawnData 可用
	return nullptr;
}

// 游戏初始化：下一帧再处理"经验匹配"（给启动设置留时间）。
void ATenkaichiGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 等下一帧，给时间初始化启动设置
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

#if 0
// ⚠️ 注释挂起：本函数硬依赖 P14 UTenkaichiAssetManager + P18 UTenkaichiDeveloperSettings（HandleMatchAssignmentIfNotExpectingOne）
// 如果还没拿到经验，就按优先级去选一个（URL 选项 → 开发者设置 → 命令行 → WorldSettings → 专用服务器 → 默认）。
void ATenkaichiGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// 优先级顺序（最高的赢）
	//  - 匹配分派（如果有）
	//  - URL 选项覆盖
	//  - 开发者设置（仅 PIE）
	//  - 命令行覆盖
	//  - World 设置
	//  - 专用服务器
	//  - 默认经验

	UWorld* World = GetWorld();

	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UTenkaichiExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UTenkaichiDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	// 看看命令行想不想设置经验
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UTenkaichiExperienceDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	// 看看 World 设置里有没有默认经验
	if (!ExperienceId.IsValid())
	{
		if (ATenkaichiWorldSettings* TypedWorldSettings = Cast<ATenkaichiWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	UTenkaichiAssetManager& AssetManager = UTenkaichiAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		UE_LOG(LogTenkaichiExperience, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// 最终回退到默认经验
	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// 这会开始以专用服务器身份主持
			return;
		}

		//@TODO: 从某个配置设置里拉这个值
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("TenkaichiExperienceDefinition"), FName("B_TenkaichiDefaultExperience"));
		ExperienceIdSource = TEXT("Default");
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}
#endif

// 尝试专用服务器登录（专用服务器场景）。
bool ATenkaichiGameMode::TryDedicatedServerLogin()
{
	// 一些注册为活动专用服务器的基本代码，游戏里会大幅修改这段
	FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && World && World->GetNetMode() == NM_DedicatedServer && World->URL.Map == DefaultMap)
	{
		// 只有专用服务器上的默认地图才注册
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

		// 专用服务器可能需要做一次在线登录
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ATenkaichiGameMode::OnUserInitializedForDedicatedServer);

		// 专用服务器上没有本地用户，但索引 0 表示默认平台用户，由在线登录代码处理
		if (!UserSubsystem->TryToLoginForOnlinePlay(0))
		{
			OnUserInitializedForDedicatedServer(nullptr, false, FText(), ECommonUserPrivilege::CanPlayOnline, ECommonUserOnlineContext::Default);
		}

		return true;
	}

	return false;
}

#if 0
// ⚠️ 注释挂起：本函数硬依赖 P13.9 UTenkaichiUserFacingExperienceDefinition（HostDedicatedServerMatch）
// 主持一场专用服务器比赛。
void ATenkaichiGameMode::HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode)
{
	FPrimaryAssetType UserExperienceType = UTenkaichiUserFacingExperienceDefinition::StaticClass()->GetFName();

	// 弄清楚要加载哪个 UserFacingExperience
	FPrimaryAssetId UserExperienceId;
	FString UserExperienceFromCommandLine;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserExperience="), UserExperienceFromCommandLine) ||
		FParse::Value(FCommandLine::Get(), TEXT("Playlist="), UserExperienceFromCommandLine))
	{
		UserExperienceId = FPrimaryAssetId::ParseTypeAndName(UserExperienceFromCommandLine);
		if (!UserExperienceId.PrimaryAssetType.IsValid())
		{
			UserExperienceId = FPrimaryAssetId(FPrimaryAssetType(UserExperienceType), FName(*UserExperienceFromCommandLine));
		}
	}

	// 搜索匹配的经验，强制加载没关系，因为我们在专用服务器启动阶段
	UTenkaichiAssetManager& AssetManager = UTenkaichiAssetManager::Get();
	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssetsWithType(UserExperienceType);
	if (ensure(Handle.IsValid()))
	{
		Handle->WaitUntilComplete();
	}

	TArray<UObject*> UserExperiences;
	AssetManager.GetPrimaryAssetObjectList(UserExperienceType, UserExperiences);
	UTenkaichiUserFacingExperienceDefinition* FoundExperience = nullptr;
	UTenkaichiUserFacingExperienceDefinition* DefaultExperience = nullptr;

	for (UObject* Object : UserExperiences)
	{
		UTenkaichiUserFacingExperienceDefinition* UserExperience = Cast<UTenkaichiUserFacingExperienceDefinition>(Object);
		if (ensure(UserExperience))
		{
			if (UserExperience->GetPrimaryAssetId() == UserExperienceId)
			{
				FoundExperience = UserExperience;
				break;
			}

			if (UserExperience->bIsDefaultExperience && DefaultExperience == nullptr)
			{
				DefaultExperience = UserExperience;
			}
		}
	}

	if (FoundExperience == nullptr)
	{
		FoundExperience = DefaultExperience;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(FoundExperience && GameInstance))
	{
		// 真正主持游戏
		UCommonSession_HostSessionRequest* HostRequest = FoundExperience->CreateHostingRequest(this);
		if (ensure(HostRequest))
		{
			HostRequest->OnlineMode = OnlineMode;

			// TODO 覆盖其他参数？

			UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
			SessionSubsystem->HostSession(nullptr, HostRequest);

			// 这会处理地图传送
		}
	}
}
#endif

// 专用服务器用户初始化完成的回调。
void ATenkaichiGameMode::OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		// 解绑
		UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
		UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ATenkaichiGameMode::OnUserInitializedForDedicatedServer);

		// 专用服务器不需要用户登录，但某些在线子系统可能期望它
		if (bSuccess && ensure(UserInfo))
		{
			UE_LOG(LogTenkaichiExperience, Log, TEXT("Dedicated server user login succeeded for id %s, starting online server"), *UserInfo->GetNetId().ToString());
		}
		else
		{
			UE_LOG(LogTenkaichiExperience, Log, TEXT("Dedicated server user login unsuccessful, starting online server as login is not required"));
		}

		HostDedicatedServerMatch(ECommonSessionOnlineMode::Online);
	}
}

// 拿到"匹配分派的经验"后：交给 ExperienceManagerComponent 去加载。
void ATenkaichiGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogTenkaichiExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

		UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogTenkaichiExperience, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
	}
}

// 经验加载完成：生成所有已经登录但还没 Pawn 的玩家。
void ATenkaichiGameMode::OnExperienceLoaded(const UTenkaichiExperienceDefinition* CurrentExperience)
{
	// 生成所有已经连接的玩家
	//@TODO: 这里只处理 *玩家* 控制器，但 GetDefaultPawnClassForController_Implementation 里我们跳过了所有控制器
	// GetDefaultPawnClassForController_Implementation 可能本来就只对玩家调用
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

// 返回"经验是否已加载完"。
bool ATenkaichiGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsExperienceLoaded();
}

// 返回控制器的默认 Pawn 类（从 PawnData 取）。
UClass* ATenkaichiGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UTenkaichiPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

// 在指定位置生成默认 Pawn（并设置 PawnData）。
APawn* ATenkaichiGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// 永远不把默认玩家 Pawn 存进地图
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (UTenkaichiPawnExtensionComponent* PawnExtComp = UTenkaichiPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				if (const UTenkaichiPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogTenkaichi, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogTenkaichi, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogTenkaichi, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

// 是否在出生点生成（本项目永远不用出生点，交给生成管理组件）。
bool ATenkaichiGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	// 我们永远不用出生点，始终用生成管理组件。
	return false;
}

// 处理新玩家开始（经验没加载完就延迟，等加载完再开始）。
void ATenkaichiGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// 延迟启动新玩家，直到经验加载完
	// （在那之前登录的玩家会由 OnExperienceLoaded 启动）
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

// 选择玩家出生点（交给 PlayerSpawningManagerComponent）。
AActor* ATenkaichiGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (UTenkaichiPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UTenkaichiPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ChoosePlayerStart(Player);
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

// 完成玩家重生（交给 PlayerSpawningManagerComponent）。
void ATenkaichiGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (UTenkaichiPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UTenkaichiPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

// 玩家能否重生（转发给 ControllerCanRestart）。
bool ATenkaichiGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

// PlayerCanRestart 的"无差别"版本，玩家和 Bot 都能用。
bool ATenkaichiGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		// Bot 版的 Super::PlayerCanRestart_Implementation
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	if (UTenkaichiPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UTenkaichiPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ControllerCanRestart(Controller);
	}

	return true;
}

// 初始化 GameState（顺便监听"经验加载完成"）。
void ATenkaichiGameMode::InitGameState()
{
	Super::InitGameState();

	// 监听经验加载完成
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnTenkaichiExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

// 通用玩家初始化（广播 OnGameModePlayerInitialized）。
void ATenkaichiGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

// 下一帧重生指定的玩家或 Bot。
void ATenkaichiGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
	else if (ATenkaichiPlayerBotController* BotController = Cast<ATenkaichiPlayerBotController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(BotController, &ATenkaichiPlayerBotController::ServerRestartController);
	}
}

// 更新玩家出生点（本项目返回 true 占位，真正的生成在 PostLogin 时做）。
bool ATenkaichiGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	// 什么都不做，等 PostLogin 时真正生成玩家。
	// 现在做任何事都不好，像队伍分配这样的系统还没发生呢。
	return true;
}

// 玩家重生失败（有 Pawn 类就下一帧重试）。
void ATenkaichiGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// 如果我们尝试生成 Pawn 失败了，就再试一次
	// *注意* 在永远重试之前先检查是不是真的有 Pawn 类
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			// 如果是玩家，不要永远循环，也许有什么变了、他们不能再重生了，那就停止尝试
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);
			}
			else
			{
				UE_LOG(LogTenkaichi, Verbose, TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again."), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		UE_LOG(LogTenkaichi, Verbose, TEXT("FailedToRestartPlayer(%s) but there's no pawn class so giving up."), *GetPathNameSafe(NewPlayer));
	}
}