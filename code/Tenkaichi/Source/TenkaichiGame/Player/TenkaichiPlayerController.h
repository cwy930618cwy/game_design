// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Camera/TenkaichiCameraAssistInterface.h"
#include "CommonPlayerController.h"
#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiPlayerController.generated.h"

#define UE_API TENKAICHIGAME_API

struct FGenericTeamId;

class ATenkaichiHUD;
class ATenkaichiPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * ATenkaichiPlayerController
 *
 *	本工程使用的玩家控制器基类。
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "本工程使用的玩家控制器基类。"))
class ATenkaichiPlayerController : public ACommonPlayerController, public ITenkaichiCameraAssistInterface, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API ATenkaichiPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取玩家状态（ATenkaichiPlayerState），用于读取队伍/小队/PawnData 等档案信息。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerController")
	UE_API ATenkaichiPlayerState* GetTenkaichiPlayerState() const;

	// 获取玩家身上的技能系统组件（ASC），用于施放技能/应用效果等。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerController")
	UE_API UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;

	// 获取玩家对应的 HUD（游戏内界面）。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerController")
	UE_API ATenkaichiHUD* GetTenkaichiHUD() const;

	// 从游戏状态逻辑调用：若 ShouldRecordClientReplay 返回 true，则开始录制自动客户端回放。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerController")
	UE_API bool TryToRecordClientReplay();

	// 判断是否应录制回放，子类可修改此逻辑。
	UE_API virtual bool ShouldRecordClientReplay();

	// 在服务器上执行一条作弊指令。
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheat(const FString& Msg);

	// 在服务器上为所有玩家执行一条作弊指令。
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheatAll(const FString& Msg);

	//~AActor 接口开始
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~AActor 接口结束

	//~AController 接口开始
	UE_API virtual void OnPossess(APawn* InPawn) override;
	UE_API virtual void OnUnPossess() override;
	UE_API virtual void InitPlayerState() override;
	UE_API virtual void CleanupPlayerState() override;
	UE_API virtual void OnRep_PlayerState() override;
	//~AController 接口结束

	//~APlayerController 接口开始
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PlayerTick(float DeltaTime) override;
	UE_API virtual void SetPlayer(UPlayer* InPlayer) override;
	UE_API virtual void AddCheats(bool bForce) override;
	UE_API virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	UE_API virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	UE_API virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~APlayerController 接口结束

	//~ITenkaichiCameraAssistInterface 接口开始
	UE_API virtual void OnCameraPenetratingTarget() override;
	//~ITenkaichiCameraAssistInterface 接口结束

	//~ITenkaichiTeamAgentInterface 接口开始
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~ITenkaichiTeamAgentInterface 接口结束

	// 设置/关闭自动奔跑。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Character")
	UE_API void SetIsAutoRunning(const bool bEnabled);

	// 查询当前是否处于自动奔跑状态。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Character")
	UE_API bool GetIsAutoRunning() const;

private:
	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

private:
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// 当玩家状态被设置或清空时调用。
	UE_API virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:

	//~APlayerController 接口开始

	//~APlayerController 接口结束

	UE_API void OnSettingsChanged(UTenkaichiSettingsShared* Settings);

	UE_API void OnStartAutoRun();
	UE_API void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	UE_API void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	UE_API void K2_OnEndAutoRun();

	bool bHideViewTargetPawnNextFrame = false;
};


// 用于回放录制与播放的玩家控制器
UCLASS()
class ATenkaichiReplayPlayerController : public ATenkaichiPlayerController
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
	virtual bool ShouldRecordClientReplay() override;

	// 回放播放期间，游戏状态的 RecorderPlayerState 被复制时触发的回调。
	void RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState);

	// 被跟随的玩家状态更换 Pawn 时触发的回调。
	UFUNCTION()
	void OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn);

	// 当前正在跟随的玩家状态。
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> FollowedPlayerState;
};

#undef UE_API