// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "GameplayMessageProcessor.generated.h"

#define UE_API TENKAICHIGAME_API

namespace EEndPlayReason { enum Type : int; }

class UObject;

/**
 * UGameplayMessageProcessor
 *
 * 任何"观察其他玩法消息、并可能重新发出更新消息"的处理器基类（比如检测到连击/连杀就重发一条）。
 *
 * 注意：这些处理器在服务器上只生成一次（不是每个玩家一个），
 * 如果只想对部分玩家生效，处理器要自己做内部过滤。
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UGameplayMessageProcessor : public UActorComponent
{
	GENERATED_BODY()

public:
	//~UActorComponent 接口开始
	// 组件开始运行时调用：启动监听
	UE_API virtual void BeginPlay() override;
	// 组件结束运行时调用：停止监听并清理所有监听句柄
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~UActorComponent 接口结束

	// 开始监听消息（虚函数，子类覆盖：在这里注册要听的消息）
	UE_API virtual void StartListening();
	// 停止监听消息（虚函数，子类覆盖：在这里停止听）
	UE_API virtual void StopListening();

protected:
	// 把一个新的监听句柄存进数组（方便 EndPlay 时统一注销）
	UE_API void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);
	// 获取服务器时间（用 GameState 的服务器时钟）
	UE_API double GetServerTime() const;

private:
	// 所有监听句柄（EndPlay 时统一注销）
	TArray<FGameplayMessageListenerHandle> ListenerHandles;
};

#undef UE_API