// Copyright Epic Games, Inc. All Rights Reserved.

#include "Teams/AsyncAction_ObserveTeam.h"

#include "Teams/TenkaichiTeamAgentInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ObserveTeam)

// 构造函数：把对象初始化器原样转交给父类，不做任何额外初始化
UAsyncAction_ObserveTeam::UAsyncAction_ObserveTeam(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 监听指定对象上的队伍变化（蓝图异步节点入口），内部转调真正的创建函数
UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::ObserveTeam(UObject* TeamAgent)
{
	return InternalObserveTeamChanges(TeamAgent);
}

// 内部创建入口：判定传入对象非空后，造出本异步节点对象、记住监听目标并注册到 GameInstance 防 GC
UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::InternalObserveTeamChanges(TScriptInterface<ITenkaichiTeamAgentInterface> TeamActor)
{
	UAsyncAction_ObserveTeam* Action = nullptr;

	if (TeamActor != nullptr)
	{
		Action = NewObject<UAsyncAction_ObserveTeam>();
		Action->TeamInterfacePtr = TeamActor;
		Action->RegisterWithGameInstance(TeamActor.GetObject());
	}

	return Action;
}

// 节点准备销毁时的收尾：先让父类解除 GameInstance 注册，再从队伍变化委托上解绑本对象的所有绑定
void UAsyncAction_ObserveTeam::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	// 如果我们被取消了，需要把所有可能监听过的东西都解绑
	if (ITenkaichiTeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		TeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
	}
}

// 节点激活：绑定队伍变化委托、立刻广播一次当前队伍，绑定失败则自我销毁
void UAsyncAction_ObserveTeam::Activate()
{
	bool bCouldSucceed = false;
	int32 CurrentTeamIndex = INDEX_NONE;

	if (ITenkaichiTeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		CurrentTeamIndex = GenericTeamIdToInteger(TeamInterface->GetGenericTeamId());

		TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnWatchedAgentChangedTeam);

		bCouldSucceed = true;
	}

	// 广播一次，让使用者拿到当前状态
	OnTeamChanged.Broadcast(CurrentTeamIndex != INDEX_NONE, CurrentTeamIndex);

	// 我们没能绑定到任何委托，所以之后永远不会再有更新了
	if (!bCouldSucceed)
	{
		SetReadyToDestroy();
	}
}

// 被监听对象换队时的回调：把新队伍号转成蓝图委托再广播一次（旧队号与本对象指针在此不使用）
void UAsyncAction_ObserveTeam::OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	OnTeamChanged.Broadcast(NewTeam != INDEX_NONE, NewTeam);
}