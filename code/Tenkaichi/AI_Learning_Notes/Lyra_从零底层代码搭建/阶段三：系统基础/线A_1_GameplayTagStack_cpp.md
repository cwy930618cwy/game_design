# 线A-1 GameplayTagStack —— 教 `.cpp`

> **定位**：阶段三线 A-1 的第 3 步——还原 `GameplayTagStack.cpp`（**.cpp 实现部分**）。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\GameplayTagStack.cpp`（109 行）
> 命名：文件无 `Lyra` 前缀，保持 `GameplayTagStack.cpp`。
> 路径：放到你工程的 `Source\TenkaichiGame\System\GameplayTagStack.cpp`。

---

## 一、`.cpp` 要实现哪些东西（对着 `.h` 的声明）

`.h` 里声明了、需要 `.cpp` 实现的函数，成对核对（铁律 11）：

| `.h` 声明 | `.cpp` 实现 |
|-----------|------------|
| `FGameplayTagStack::GetDebugString()` | ✅ 第 12 行 |
| `FGameplayTagStackContainer::AddStack()` | ✅ 第 20 行 |
| `FGameplayTagStackContainer::RemoveStack()` | ✅ 第 48 行 |
| `FGameplayTagStackContainer::PreReplicatedRemove()` | ✅ 第 83 行 |
| `FGameplayTagStackContainer::PostReplicatedAdd()` | ✅ 第 92 行 |
| `FGameplayTagStackContainer::PostReplicatedChange()` | ✅ 第 101 行 |

> 注意：`GetStackCount` / `ContainsTag` / `NetDeltaSerialize` 这三个是**内联在 `.h` 里实现的**，`.cpp` 不用再写。

---

## 二、整份 `.cpp` 一比一还原（照着写）

> **注释中文化**（铁律 21/28）：注释已翻成中文；代码本体（类名/函数/宏/容器操作）与 Lyra 原样一字不差。

```cpp
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
```

---

## 三、逐段讲解（每一段在干嘛）

### 段 1：头文件包含（1~7 行）
```cpp
#include "GameplayTagStack.h"
#include "UObject/Stack.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagStack)
```
- 第 1 行：自己的 `.h`。
- 第 4 行：`UObject/Stack.h`——提供 `FFrame::KismetExecutionMessage`（在蓝图层打警告消息）。
- 第 7 行：UHT 生成的 `.cpp` 实现（配合 `.h` 里的 `.generated.h`），`UE_INLINE_GENERATED_CPP_BY_NAME` 是宏。

### 段 2：`GetDebugString`（11~15 行）
```cpp
FString FGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
}
```
- 用 `FString::Printf` 拼字符串：`Tag.ToString()`（标签名）+ `x` + 层数。
- 例：`Status.Death.Deadx3`。纯调试用。

### 段 3：`AddStack` 加层（20~46 行）★ 核心
逻辑分三步：
1. **标签无效就警告返回**（`Tag.IsValid()` + `KismetExecutionMessage`）。
2. **层数 > 0 才处理**。
3. **分两种情况**：
   - 已存在同标签 → 层数累加 + 更新查询表 + `MarkItemDirty`（标记单个条目变脏）。
   - 不存在 → `Stacks.Emplace_GetRef(Tag, StackCount)` 在末尾新增条目 + `MarkItemDirty` + 登记查询表。

> `MarkItemDirty(条目)`：告诉 FastArray "这个条目变了，请把它单独发到客户端"。这就是**增量复制**的关键——只发这一个条目，不整包发。

### 段 4：`RemoveStack` 减层（48~81 行）★ 核心
逻辑和加层对称：
1. 标签无效就警告返回。
2. 层数 > 0 才处理。
3. 找到同标签后分两种情况：
   - **减完 ≤ 0** → `It.RemoveCurrent()` 整个条目删掉 + 查询表删 + `MarkArrayDirty()`（标记整个数组变脏）。
   - **减完 > 0** → 只减层数 + 更新查询表 + `MarkItemDirty`。

> 注意：`RemoveStack` 用的是 `CreateIterator()`（迭代器遍历，可安全删除当前元素），`AddStack` 用的是范围 for（只读遍历，找匹配项）。两者遍历方式不同是有原因的——一个要删、一个不删。

### 段 5：三个复制回调（83~108 行）★ 网络同步核心
这三个是 `FFastArraySerializer` 的回调，**在客户端收到网络增量数据时被引擎调用**，用来维护本机的 `TagToCountMap` 查询表：
- `PreReplicatedRemove`：客户端收到"某条目被删"→ 从查询表删掉该标签。
- `PostReplicatedAdd`：客户端收到"新条目加入"→ 登记进查询表。
- `PostReplicatedChange`：客户端收到"某条目层数变了"→ 更新查询表。

> 关键点：**`Stacks` 数组是服务器直接同步的**（`MarkItemDirty` 标记过的都会发过来）；而 `TagToCountMap` **不复制**，是靠这三个回调在客户端**重建**的。这样查询快（用 Map）又省流量（只传数组增量）。

---

## 四、写完后自查（铁律 11 两个核对）

1. **缺行核对**：`.cpp` 必须实现 `GetDebugString / AddStack / RemoveStack / PreReplicatedRemove / PostReplicatedAdd / PostReplicatedChange` 这 6 个函数，一个不能少。
2. **声明成对**：`.h` 里声明的这 6 个函数，`.cpp` 里全部实现了；内联在 `.h` 的 3 个（`GetStackCount/ContainsTag/NetDeltaSerialize`）不在 `.cpp` 重复写。

---

## 五、整体回顾：这份文件到底在做什么

把 `.h` + `.cpp` 合起来看，`GameplayTagStack` 就是一套"**可复制、可计数的标签容器**"：
- 服务器端 `AddStack` / `RemoveStack` 增删层数，`MarkItemDirty` 标记变化。
- 引擎把变化增量同步到客户端（`NetDeltaSerialize` + `TStructOpsTypeTraits`）。
- 客户端三个回调重建查询表，上层就能用 `GetStackCount` / `ContainsTag` 快速查询。
- 上层（阶段四/五的角色、效果系统）会用它在 Pawn 上叠"3 层燃烧"这类状态。

---

## 六、确认点

按铁律 29，你写完 `.cpp` 后说"下一步"，**我会先读你工程里的 `GameplayTagStack.cpp` 确认写完、内容对得上**，再进线 A 第 2 项（`TenkaichiGameEngine`，走同样的"总 md → h → cpp"）。