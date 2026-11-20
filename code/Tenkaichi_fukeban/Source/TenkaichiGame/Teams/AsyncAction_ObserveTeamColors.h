// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/WeakInterfacePtr.h"

#include "AsyncAction_ObserveTeamColors.generated.h"

class ITenkaichiTeamAgentInterface;
class UTenkaichiTeamDisplayAsset;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTeamColorObservedAsyncDelegate, bool, bTeamSet, int32, TeamId, const UTenkaichiTeamDisplayAsset*, DisplayAsset);

/**
 * 监听指定对象上的队伍颜色（外观资产）变化
 */
UCLASS()
class UAsyncAction_ObserveTeamColors : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// 监听指定队伍代理（TeamAgent）上的队伍变化
	//  - 它会立刻触发一次，给出当前的队伍归属
	//  - 对于任何可能属于某个队伍的对象（实现了 ITenkaichiTeamAgentInterface 的），
	//    它之后还会持续监听队伍归属的变化
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
	static UAsyncAction_ObserveTeamColors* ObserveTeamColors(UObject* TeamAgent);

	//~UBlueprintAsyncActionBase 接口开始
	// 节点被激活时调用：取当前队伍与外观资产、订阅变更委托，并立刻广播一次
	virtual void Activate() override;
	// 节点即将销毁或被取消时调用：解除"换队"委托订阅，避免对象销毁后仍被回调
	virtual void SetReadyToDestroy() override;
	//~UBlueprintAsyncActionBase 接口结束

public:
	// 当队伍被设置或改变时调用
	UPROPERTY(BlueprintAssignable)
	FTeamColorObservedAsyncDelegate OnTeamChanged;

private:
	// 统一广播出口：先解绑旧队伍的外观监听，再广播当前队伍与外观，最后绑定新队伍的外观监听
	void BroadcastChange(int32 NewTeam, const UTenkaichiTeamDisplayAsset* DisplayAsset);

	// 被监听对象换队时的回调：查出新队伍的外观资产后统一广播
	UFUNCTION()
	void OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	// 当前队伍的外观资产被修改时的回调：用上次广播的队伍号再播一次
	UFUNCTION()
	void OnDisplayAssetChanged(const UTenkaichiTeamDisplayAsset* DisplayAsset);

	// 被监听对象的接口视图，用于调用接口方法和绑定换队委托
	TWeakInterfacePtr<ITenkaichiTeamAgentInterface> TeamInterfacePtr;
	// 同一个对象的裸 UObject 弱引用，用于需要 WorldContextObject 的场合
	TWeakObjectPtr<UObject> TeamInterfaceObj;

	// 上次广播的队伍号，用于判断是否需要把外观监听从旧队搬到新队
	int32 LastBroadcastTeamId = INDEX_NONE;
};