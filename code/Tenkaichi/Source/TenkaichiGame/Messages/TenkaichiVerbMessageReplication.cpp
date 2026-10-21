// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/TenkaichiVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FTenkaichiVerbMessageReplicationEntry

// 返回这条消息的调试字符串（调 Message.ToString()）
FString FTenkaichiVerbMessageReplicationEntry::GetDebugString() const
{
	return Message.ToString();
}

//////////////////////////////////////////////////////////////////////
// FTenkaichiVerbMessageReplication

// 从服务器向客户端广播一条消息：把消息加进数组并标记脏，触发 FastArray 增量复制
void FTenkaichiVerbMessageReplication::AddMessage(const FTenkaichiVerbMessage& Message)
{
	FTenkaichiVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);
	MarkItemDirty(NewStack);
}

// 元素被移除时的回调（Lyra 目前不用，函数体留空）
void FTenkaichiVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
// 	for (int32 Index : RemovedIndices)
// 	{
// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
// 		TagToCountMap.Remove(Tag);
// 	}
}

// 元素被新增时的回调：把新收到的消息重新广播给本地订阅者
void FTenkaichiVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FTenkaichiVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

// 元素被变更时的回调：把变更后的消息重新广播给本地订阅者
void FTenkaichiVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FTenkaichiVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

// 把消息重新广播给本地订阅者（经 GameplayMessageRouter 插件的消息系统）
void FTenkaichiVerbMessageReplication::RebroadcastMessage(const FTenkaichiVerbMessage& Message)
{
	check(Owner);
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}