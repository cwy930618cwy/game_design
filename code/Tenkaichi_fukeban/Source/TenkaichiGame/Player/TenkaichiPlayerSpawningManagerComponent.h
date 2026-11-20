// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/GameStateComponent.h"

#include "TenkaichiPlayerSpawningManagerComponent.generated.h"

#define UE_API TENKAICHIGAME_API

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ATenkaichiPlayerStart;
class AActor;

/**
 * UTenkaichiPlayerSpawningManagerComponent
 *
 * 出生点管理组件：挂在 GameState 上，把出生点选择逻辑从 GameMode 里拆出来，
 * 让每个玩法模式（Experience）都能自定义自己的出生规则。
 */
UCLASS(MinimalAPI)
class UTenkaichiPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UE_API UTenkaichiPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	UE_API virtual void InitializeComponent() override;
	UE_API virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */

protected:
	// 工具函数：从出生点列表里随机挑一个"空"的，没空的就挑"半空"的。
	UE_API APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ATenkaichiPlayerStart*>& FoundStartPoints) const;

	// 虚函数：给指定玩家选出生点（子类可重载自定义；默认返回空指针 = 走随机空位逻辑）。
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ATenkaichiPlayerStart*>& PlayerStarts) { return nullptr; }
	// 虚函数：重生完成后回调（子类可重载）。
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	// 蓝图事件：重生完成后回调（蓝图里可绑定，DisplayName 同名）。
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	UE_API void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

	/** 这些方法被 ATenkaichiGameMode 代理调用，转发到这个组件，让每个 Experience 更容易自定义复活系统。 */
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API bool ControllerCanRestart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class ATenkaichiGameMode;
	/** ~ATenkaichiGameMode */

	// 缓存收集到的所有出生点（弱引用，防止引用失效）。
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ATenkaichiPlayerStart>> CachedPlayerStarts;

private:
	// 新关卡加载时，把该关卡里的出生点加入缓存。
	UE_API void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	// 场景里新出生一个 Actor 时，若是出生点就加入缓存。
	UE_API void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	// 编辑器专用：找"从这开始玩"的出生点（PIE 调试用）。
	UE_API APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};

#undef UE_API