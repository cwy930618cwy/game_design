// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "TenkaichiGameState.generated.h"

#define UE_API TENKAICHIGAME_API

struct FTenkaichiVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * ATenkaichiGameState
 *
 *	本项目使用的基础游戏状态类。
 */
UCLASS(MinimalAPI, Config = Game)
class ATenkaichiGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	// 构造函数：创建 AbilitySystemComponent + ExperienceManagerComponent 子对象。
	UE_API ATenkaichiGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor 接口开始
	// 组件预初始化（先调父类）。
	UE_API virtual void PreInitializeComponents() override;
	// 组件后初始化（初始化全局 ASC 的 actor 信息）。
	UE_API virtual void PostInitializeComponents() override;
	// Actor 生命周期结束（先调父类）。
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// 每帧更新（服务器权威时更新 ServerFPS）。
	UE_API virtual void Tick(float DeltaSeconds) override;
	//~AActor 接口结束

	//~AGameStateBase 接口开始
	// 添加玩家状态（先调父类）。
	UE_API virtual void AddPlayerState(APlayerState* PlayerState) override;
	// 移除玩家状态（先调父类）。
	UE_API virtual void RemovePlayerState(APlayerState* PlayerState) override;
	// 无缝传送过渡检查点（移除 Bot 和非活跃玩家）。
	UE_API virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	//~AGameStateBase 接口结束

	//~IAbilitySystemInterface 接口开始
	// 返回这个 GameState 的全局能力系统组件。
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~IAbilitySystemInterface 接口结束

	// 获取用于"游戏全局事物"的能力系统组件。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|GameState")
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const { return AbilitySystemComponent; }

	// 发送一条所有客户端（大概率）都会收到的消息
	// （只用于像淘汰、服务器加入消息等可以接受丢失的客户端通知）
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Tenkaichi|GameState")
	UE_API void MulticastMessageToClients(const FTenkaichiVerbMessage Message);

	// 发送一条保证所有客户端都会收到的消息
	// （只用于不能接受丢失的客户端通知）
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Tenkaichi|GameState")
	UE_API void MulticastReliableMessageToClients(const FTenkaichiVerbMessage Message);

	// 获取服务器的 FPS（复制给客户端）。
	UE_API float GetServerFPS() const;

	// 标记本地玩家状态正在录制回放。
	UE_API void SetRecorderPlayerState(APlayerState* NewPlayerState);

	// 获取录制回放的玩家状态（如果有效）。
	UE_API APlayerState* GetRecorderPlayerState() const;

	// 回放玩家状态变化时调用的委托。
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRecorderPlayerStateChanged, APlayerState*);
	FOnRecorderPlayerStateChanged OnRecorderPlayerStateChangedEvent;

private:
	// 处理当前玩法经验的加载和管理。
	UPROPERTY()
	TObjectPtr<UTenkaichiExperienceManagerComponent> ExperienceManagerComponent;

	// 用于"游戏全局事物"（主要是 gameplay cue）的能力系统组件子对象。
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|GameState")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

protected:
	// 服务器的 FPS（复制给客户端）。
	UPROPERTY(Replicated)
	float ServerFPS;

	// 录制回放的玩家状态，用来选择正确的 Pawn 跟随。
	// 只在回放流里设置，正常情况下不复制。
	UPROPERTY(Transient, ReplicatedUsing = OnRep_RecorderPlayerState)
	TObjectPtr<APlayerState> RecorderPlayerState;

	// 回放玩家状态复制回调。
	UFUNCTION()
	UE_API void OnRep_RecorderPlayerState();

};

#undef UE_API