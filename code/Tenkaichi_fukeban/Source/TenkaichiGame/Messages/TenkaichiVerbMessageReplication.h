// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "TenkaichiVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "TenkaichiVerbMessageReplication.generated.h"

class UObject;
struct FTenkaichiVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * 表示一条动词消息
 */
USTRUCT(BlueprintType)
struct FTenkaichiVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FTenkaichiVerbMessageReplicationEntry()
	{}

	FTenkaichiVerbMessageReplicationEntry(const FTenkaichiVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	// 返回这条消息的调试字符串（调 Message.ToString()）
	FString GetDebugString() const;

private:
	friend FTenkaichiVerbMessageReplication;

	UPROPERTY()
	FTenkaichiVerbMessage Message;
};

/** 要复制的动词消息容器 */
USTRUCT(BlueprintType)
struct FTenkaichiVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FTenkaichiVerbMessageReplication()
	{
	}

public:
	// 设置持有者（用于找到世界、进而找到消息系统）
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// 从服务器向客户端广播一条消息
	void AddMessage(const FTenkaichiVerbMessage& Message);

	//~FFastArraySerializer 接口开始
	// 元素被移除时调用（FastArray 回调）
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	// 元素被新增时调用（FastArray 回调）
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// 元素被变更时调用（FastArray 回调）
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~FFastArraySerializer 接口结束

	// 自定义增量序列化（FastArray 的接入点，直接把数组交给引擎的 FastArrayDeltaSerialize）
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FTenkaichiVerbMessageReplicationEntry, FTenkaichiVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	// 在客户端把收到的消息重新广播给本地订阅者
	void RebroadcastMessage(const FTenkaichiVerbMessage& Message);

private:
	// 被复制的消息列表
	UPROPERTY()
	TArray<FTenkaichiVerbMessageReplicationEntry> CurrentMessages;
	
	// 持有者（用于找到世界）
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FTenkaichiVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FTenkaichiVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};