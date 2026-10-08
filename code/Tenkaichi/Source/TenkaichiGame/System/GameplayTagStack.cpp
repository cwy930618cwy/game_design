// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h
#include "GameplayTagStack.h"

// 用到 FFrame（KismetExecutionMessage 需要），提供脚本执行消息打印
#include "UObject/Stack.h"

// UHT 生成的实现（对应 GameplayTagStack.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagStack)

//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

// 调试打印：把"标签x数量"拼成字符串
FString FGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
}

//////////////////////////////////////////////////////////////////////
// FGameplayTagStackContainer

// 给指定标签加上 N 层
void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	// 标签无效就不处理，并给蓝图层打警告
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	// 只有层数大于 0 才真加
	if (StackCount > 0)
	{
		// 先在现有条目里找同名标签
		for (FGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				// 找到：层数直接累加
				const int32 NewCount = Stack.StackCount + StackCount;
				Stack.StackCount = NewCount;
				TagToCountMap[Tag] = NewCount;   // 同步更新查询表
				MarkItemDirty(Stack);             // 标记该条目"变脏"，触发增量复制
				return;
			}
		}

		// 没找到：在数组末尾新加一个条目
		FGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		MarkItemDirty(NewStack);                  // 标记新条目，触发复制
		TagToCountMap.Add(Tag, StackCount);       // 登记到查询表
	}
}

// 给指定标签减掉 N 层
void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	// 标签无效就不处理，并给蓝图层打警告
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	//@TODO: 如果尝试移除一个不存在或层数不够的栈，要不要报错？先不报
	if (StackCount > 0)
	{
		// 遍历找同名标签
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				// 减完小于等于 0：整条移除
				if (Stack.StackCount <= StackCount)
				{
					It.RemoveCurrent();            // 从数组删掉这个条目
					TagToCountMap.Remove(Tag);     // 从查询表删掉
					MarkArrayDirty();              // 标记整个数组"变脏"，触发复制
				}
				// 否则：只减层数
				else
				{
					const int32 NewCount = Stack.StackCount - StackCount;
					Stack.StackCount = NewCount;
					TagToCountMap[Tag] = NewCount;
					MarkItemDirty(Stack);           // 标记该条目"变脏"
				}
				return;
			}
		}
	}
}

// 客户端收到"有条目被移除"时调用：从查询表里删掉对应标签
void FGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FGameplayTag Tag = Stacks[Index].Tag;
		TagToCountMap.Remove(Tag);
	}
}

// 客户端收到"有新条目加入"时调用：把新条目登记进查询表
void FGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap.Add(Stack.Tag, Stack.StackCount);
	}
}

// 客户端收到"有条目被修改"时调用：更新查询表里对应标签的层数
void FGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap[Stack.Tag] = Stack.StackCount;
	}
}