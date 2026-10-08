// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 标签容器头文件：提供 FGameplayTag 类型
#include "GameplayTagContainer.h"
// FastArray 序列化头文件：提供增量复制能力
#include "Net/Serialization/FastArraySerializer.h"

// 生成头文件（UHT 用，放最后）
#include "GameplayTagStack.generated.h"

struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

/**
 * 表示"一个标签的叠层"（tag + 数量）
 */
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	// 默认构造
	FGameplayTagStack()
	{}

	// 带参构造：传入标签和初始层数
	FGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount)
	{
	}

	// 调试用：把这一层打印成 "标签x数量" 的字符串
	FString GetDebugString() const;

private:
	// 只有容器（FGameplayTagStackContainer）能访问这两个私有字段
	friend FGameplayTagStackContainer;

	// 是哪个标签
	UPROPERTY()
	FGameplayTag Tag;

	// 叠了几层
	UPROPERTY()
	int32 StackCount = 0;
};

/** 装"一堆标签叠层"的容器 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameplayTagStackContainer()
	//	: Owner(nullptr)
	{
	}

public:
	// 给指定标签加上 N 层（N 小于 1 时不做事）
	void AddStack(FGameplayTag Tag, int32 StackCount);

	// 给指定标签减掉 N 层（N 小于 1 时不做事）
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// 查询指定标签的层数（标签不在则返回 0）
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	// 判断指定标签是否至少叠了 1 层
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}

	//~FFastArraySerializer 需要实现的一组"复制回调"（增量同步时被引擎调用）
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~FFastArraySerializer 回调结束

	// 增量复制入口：调用 FastArray 框架做增量序列化
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagStack, FGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}

private:
	// 可复制的"标签叠层"数组（真正同步到网络的数据）
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;

	// 加速查询用的 Map：标签 → 层数（不复制，只在本机用）
	TMap<FGameplayTag, int32> TagToCountMap;
};

// 告诉引擎：这个容器结构开启了"增量复制"能力
template<>
struct TStructOpsTypeTraits<FGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};