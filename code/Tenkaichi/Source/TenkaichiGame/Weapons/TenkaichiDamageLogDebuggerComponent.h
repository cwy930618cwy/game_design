// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "TenkaichiDamageLogDebuggerComponent.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UObject;
struct FGameplayTag;
struct FTenkaichiVerbMessage;

// 单帧受击统计：把同一帧的多次受击合并成一条记录
struct FFrameDamageEntry
{
	int32 NumImpacts = 0;      // 这一帧被打了几次
	double SumDamage = 0.0;    // 这一帧累计的伤害值
	double TimeOfFirstHit = 0.0; // 这一帧第一次被打的时间点
};

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UTenkaichiDamageLogDebuggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// 构造函数：在 .cpp 里开启 Tick（PrimaryComponentTick.bCanEverTick = true）
	UTenkaichiDamageLogDebuggerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 组件激活时调用：向消息总线注册"受伤"消息监听
	virtual void BeginPlay() override;

	// 组件结束/销毁时调用：反注册消息监听，防止内存泄漏
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 每帧调用：检查距上次受击是否超过阈值，超时则汇总打印这一波伤害
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 可调参数：距离上次受伤超过该秒数，就结算一次伤害日志
	UPROPERTY(EditAnywhere)
	double SecondsBetweenDamageBeforeLogging = 1.0;

private:
	// 消息监听句柄：EndPlay 里用它反注册
	FGameplayMessageListenerHandle ListenerHandle;

	// 上一次受伤的时间点：Tick 里用它判断是否超时结算
	double LastDamageEntryTime = 0.0;

	// 帧号 → 该帧受击统计（按帧合并同一波伤害）
	TMap<int64, FFrameDamageEntry> DamageLog;

private:
	// 收到"受伤"消息时的回调：把伤害按帧记进 DamageLog
	void OnDamageMessage(FGameplayTag Channel, const FTenkaichiVerbMessage& Payload);
};