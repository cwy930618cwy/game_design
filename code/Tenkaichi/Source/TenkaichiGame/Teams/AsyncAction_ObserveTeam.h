// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/ScriptInterface.h"
#include "UObject/WeakInterfacePtr.h"

#include "AsyncAction_ObserveTeam.generated.h"

class ITenkaichiTeamAgentInterface;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamObservedAsyncDelegate, bool, bTeamSet, int32, TeamId);

/**
 * 监听指定对象上的队伍变化
 */
UCLASS()
class UAsyncAction_ObserveTeam : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// 监听指定队伍代理（TeamAgent）上的队伍变化
	//  - 它会立刻触发一次，给出当前的队伍归属
	//  - 对于任何可能属于某个队伍的对象（实现了 ITenkaichiTeamAgentInterface 的），
	//    它之后还会持续监听队伍归属的变化
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
	static UAsyncAction_ObserveTeam* ObserveTeam(UObject* TeamAgent);

	//~UBlueprintAsyncActionBase 接口开始
	// 节点被激活时调用：订阅队伍变化委托，并立刻广播一次当前的队伍归属
	virtual void Activate() override;
	// 节点即将销毁或被取消时调用：解除委托订阅，避免对象销毁后仍被回调
	virtual void SetReadyToDestroy() override;
	//~UBlueprintAsyncActionBase 接口结束

public:
	// 当队伍被设置或改变时调用
	UPROPERTY(BlueprintAssignable)
	FTeamObservedAsyncDelegate OnTeamChanged;

private:
	// 监听指定队伍 Actor 上的队伍变化
	static UAsyncAction_ObserveTeam* InternalObserveTeamChanges(TScriptInterface<ITenkaichiTeamAgentInterface> TeamActor);

private:
	// 被监听对象换队时的回调：把新队伍号转成蓝图委托再广播一次
	UFUNCTION()
	void OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	// 记住"我在监听谁"，用弱引用保存，避免延长被监听对象的生命周期
	TWeakInterfacePtr<ITenkaichiTeamAgentInterface> TeamInterfacePtr;
};