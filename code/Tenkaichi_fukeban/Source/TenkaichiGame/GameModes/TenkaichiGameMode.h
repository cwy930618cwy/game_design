// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "ModularGameMode.h"

#include "TenkaichiGameMode.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class UTenkaichiExperienceDefinition;
class UTenkaichiPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
enum class ECommonSessionOnlineMode : uint8;

// 声明"玩家初始化完成"的多播委托：带两个参数（GameMode 指针、玩家控制器指针）。
// 在玩家或 Bot 加入游戏、以及无缝/非无缝传送之后触发。
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTenkaichiGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * ATenkaichiGameMode
 *
 *	本项目使用的基础游戏模式类。
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ATenkaichiGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	// 构造函数：指定 GameState/PlayerController/Pawn/HUD 等默认类。
	UE_API ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 查指定控制器该用哪个 PawnData（先从 PlayerState 查，再回退到经验默认值）。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Pawn")
	UE_API const UTenkaichiPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase 接口开始
	// 游戏初始化：下一帧再处理"经验匹配"（给启动设置留时间）。
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	// 返回控制器的默认 Pawn 类（从 PawnData 取）。
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	// 在指定位置生成默认 Pawn（并设置 PawnData）。
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	// 是否在出生点生成（本项目永远不用出生点，交给生成管理组件）。
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	// 处理新玩家开始（经验没加载完就延迟，等加载完再开始）。
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	// 选择玩家出生点（交给 PlayerSpawningManagerComponent）。
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	// 完成玩家重生（交给 PlayerSpawningManagerComponent）。
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	// 玩家能否重生（转发给 ControllerCanRestart）。
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	// 初始化 GameState（顺便监听"经验加载完成"）。
	UE_API virtual void InitGameState() override;
	// 更新玩家出生点（本项目返回 true 占位，真正的生成在 PostLogin 时做）。
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	// 通用玩家初始化（广播 OnGameModePlayerInitialized）。
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	// 玩家重生失败（有 Pawn 类就下一帧重试）。
	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~AGameModeBase 接口结束

	// 下一帧重生指定的玩家或 Bot。
	// 如果 bForceReset 为 true，控制器本帧就会被重置（丢弃当前拥有的 Pawn）。
	UFUNCTION(BlueprintCallable)
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// PlayerCanRestart 的"无差别"版本，玩家和 Bot 都能用。
	UE_API virtual bool ControllerCanRestart(AController* Controller);

	// 玩家初始化完成时调用的委托（见上方声明）。
	FOnTenkaichiGameModePlayerInitialized OnGameModePlayerInitialized;

protected:
	// 经验加载完成：生成所有已经登录但还没 Pawn 的玩家。
	UE_API void OnExperienceLoaded(const UTenkaichiExperienceDefinition* CurrentExperience);
	// 返回"经验是否已加载完"。
	UE_API bool IsExperienceLoaded() const;

	// 拿到"匹配分派的经验"后：交给 ExperienceManagerComponent 去加载。
	UE_API void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	// 如果还没拿到经验，就按优先级去选一个。
	UE_API void HandleMatchAssignmentIfNotExpectingOne();

	// 尝试专用服务器登录（专用服务器场景）。
	UE_API bool TryDedicatedServerLogin();
	// 主持一场专用服务器比赛。
	UE_API void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode);

	// 专用服务器用户初始化完成的回调。
	UFUNCTION()
	UE_API void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
};

#undef UE_API