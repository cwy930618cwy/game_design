// Copyright Epic Games, Inc. All Rights Reserved.

#include "Teams/AsyncAction_ObserveTeamColors.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Teams/TenkaichiTeamAgentInterface.h"
#include "Teams/TenkaichiTeamStatics.h"
#include "Teams/TenkaichiTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ObserveTeamColors)

// 构造函数：把对象初始化器原样转交给父类，不做任何额外初始化
UAsyncAction_ObserveTeamColors::UAsyncAction_ObserveTeamColors(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 监听指定对象的队伍颜色变化（蓝图异步节点入口）：造节点、记住两个弱引用并注册到 GameInstance 防 GC
UAsyncAction_ObserveTeamColors* UAsyncAction_ObserveTeamColors::ObserveTeamColors(UObject* TeamAgent)
{
	UAsyncAction_ObserveTeamColors* Action = nullptr;

	if (TeamAgent != nullptr)
	{
		Action = NewObject<UAsyncAction_ObserveTeamColors>();
		Action->TeamInterfacePtr = TWeakInterfacePtr<ITenkaichiTeamAgentInterface>(TeamAgent);
		Action->TeamInterfaceObj = TeamAgent;
		Action->RegisterWithGameInstance(TeamAgent);
	}

	return Action;
}

// 节点准备销毁时的收尾：先让父类解除 GameInstance 注册，再从"换队"委托上解绑本对象的所有绑定
void UAsyncAction_ObserveTeamColors::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	// 如果我们被取消了，需要把所有可能监听过的东西都解绑
	if (ITenkaichiTeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		TeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
	}
}

// 节点激活：取当前队伍与外观资产、绑定"换队"委托、立刻广播一次，取不到 World 或绑定失败则自我销毁
void UAsyncAction_ObserveTeamColors::Activate()
{
	bool bCouldSucceed = false;
	int32 CurrentTeamIndex = INDEX_NONE;
	UTenkaichiTeamDisplayAsset* CurrentDisplayAsset = nullptr;

	if (ITenkaichiTeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(TeamInterfaceObj.Get(), EGetWorldErrorMode::LogAndReturnNull))
		{
			// 取得当前的队伍信息
			CurrentTeamIndex = GenericTeamIdToInteger(TeamInterface->GetGenericTeamId());
			CurrentDisplayAsset = UTenkaichiTeamStatics::GetTeamDisplayAsset(World, CurrentTeamIndex);

			// 监听未来的队伍变化
			TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnWatchedAgentChangedTeam);

			bCouldSucceed = true;
		}
	}

	// 广播一次，让使用者拿到当前状态
	BroadcastChange(CurrentTeamIndex, CurrentDisplayAsset);

	// 我们没能绑定到任何委托，所以之后永远不会再有更新了
	if (!bCouldSucceed)
	{
		SetReadyToDestroy();
	}
}

// 统一广播出口：先把外观监听从旧队伍解绑，再广播当前队伍与外观，最后绑到新队伍上
void UAsyncAction_ObserveTeamColors::BroadcastChange(int32 NewTeam, const UTenkaichiTeamDisplayAsset* DisplayAsset)
{
	UWorld* World = GEngine->GetWorldFromContextObject(TeamInterfaceObj.Get(), EGetWorldErrorMode::LogAndReturnNull);
	UTenkaichiTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTenkaichiTeamSubsystem>(World);

	const bool bTeamChanged = (LastBroadcastTeamId != NewTeam);

	// 停止监听旧队伍
	if ((TeamSubsystem != nullptr) && bTeamChanged && (LastBroadcastTeamId != INDEX_NONE))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(LastBroadcastTeamId).RemoveAll(this);
	}

	// 广播
	LastBroadcastTeamId = NewTeam;
	OnTeamChanged.Broadcast(NewTeam != INDEX_NONE, NewTeam, DisplayAsset);

	// 开始监听新队伍
	if ((TeamSubsystem != nullptr) && bTeamChanged && (NewTeam != INDEX_NONE))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(NewTeam).AddDynamic(this, &ThisClass::OnDisplayAssetChanged);
	}
}

// 被监听对象换队时的回调：查出新队伍的外观资产后统一广播
void UAsyncAction_ObserveTeamColors::OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	UTenkaichiTeamDisplayAsset* DisplayAsset = UTenkaichiTeamStatics::GetTeamDisplayAsset(TeamAgent, NewTeam);
	BroadcastChange(NewTeam, DisplayAsset);
}

// 当前队伍外观资产被修改时的回调：沿用上次广播的队伍号再播一次
void UAsyncAction_ObserveTeamColors::OnDisplayAssetChanged(const UTenkaichiTeamDisplayAsset* DisplayAsset)
{
	BroadcastChange(LastBroadcastTeamId, DisplayAsset);
}