# 线A-1 GameplayTagStack —— 教 `.h`

> **定位**：阶段三线 A-1 的第 2 步——还原 `GameplayTagStack.h`（**.h 声明部分**）。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\GameplayTagStack.h`（99 行）
> 命名：这个文件在 Lyra 里就叫 `GameplayTagStack`（无 `Lyra` 前缀），类名 `FGameplayTagStack` / `FGameplayTagStackContainer`，**不涉及前缀替换**。
> 路径：放到你工程的 `Source\TenkaichiGame\System\GameplayTagStack.h`（对应 Lyra 的 `System\` 目录）。

---

## 一、为什么要先理解两个概念（讲清楚再写）

`.h` 里只有两个结构体，但里面藏了**两个底层机制**，不先讲清楚，照抄也会一头雾水：

### 1. `FGameplayTag` —— 引擎的"标签"类型
阶段一 GameplayTags 已讲过。`FGameplayTag` 就是 UE 里表示"一个标签"的类型（如 `Status.Death.Dead`）。`GameplayTagStack` 用它在容器里**标识"是哪个标签在叠层数"**。

### 2. `FFastArraySerializer` —— 数组"增量复制"框架（本文件核心）
这是引擎提供的一套**网络同步工具**：
- 普通 `TArray` 属性要复制，只能**整个数组**打包发送（又大又慢）。
- `FFastArraySerializer` 让一个 `TArray` 具备**增量复制**能力——只发送**变化的那几个元素**，不变的不发。
- 实现方式是：条目结构继承 `FFastArraySerializerItem`，容器结构继承 `FFastArraySerializer`，再加一个 `NetDeltaSerialize` + `TStructOpsTypeTraits` 开关。
- **先知道它"让数组复制更省流量"即可，具体回调机制教 `.cpp` 时再深讲。**

> 为什么 `GameplayTagStack` 要用它？因为"哪个角色叠了 3 层燃烧"这种状态**要同步到所有客户端**，用 FastArray 增量复制最省、最标准。

---

## 二、整份 `.h` 一比一还原（照着写）

> **注释中文化**（铁律 21/28）：下面代码注释已翻成中文；代码本体（类名/宏/类型）与 Lyra 原样一字不差。

```cpp
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
```

---

## 三、逐段讲解（每一段是干嘛的）

### 段 1：头文件 + 前置声明（1~14 行）
```cpp
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagStack.generated.h"
struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;
```
- 引入两个依赖头文件：`GameplayTagContainer.h`（FGameplayTag 类型）、`FastArraySerializer.h`（增量复制框架）。
- `.generated.h` 由 UHT 自动生成，放最后。
- 两个 `struct` 前置声明：`FGameplayTagStackContainer`（本文件内用，先声明后面定义）、`FNetDeltaSerializeInfo`（引擎类型，只声明不展开）。

### 段 2：`FGameplayTagStack` 条目结构（16~40 行）
- `USTRUCT(BlueprintType)`：UE 结构体，可在蓝图里用。
- `: public FFastArraySerializerItem`：继承 FastArray 的"条目"基类，让它可以被增量复制。
- 两个字段：`Tag`（哪个标签）、`StackCount`（几层），都是 `private`，**只有容器能读写**（`friend FGameplayTagStackContainer`）。
- `GetDebugString()`：声明一个调试打印函数，实现放 `.cpp`。

### 段 3：`FGameplayTagStackContainer` 容器结构（43~90 行）
- `: public FFastArraySerializer`：继承 FastArray 的"容器"基类。
- 对外方法：`AddStack` / `RemoveStack` / `GetStackCount` / `ContainsTag`——加层、减层、查询、判断。
- 三个复制回调（`PreReplicatedRemove` / `PostReplicatedAdd` / `PostReplicatedChange`）：FastArray 同步时，客户端收到"删/加/改"通知后，用来同步维护 `TagToCountMap` 查询表。**实现放 `.cpp`。**
- `NetDeltaSerialize`：增量复制的入口，调用框架的 `FastArrayDeltaSerialize`。
- 两个字段：`Stacks`（真正的复制数据）、`TagToCountMap`（加速查询，不复制）。

### 段 4：`TStructOpsTypeTraits` 特化（92~99 行）
- 模板特化：告诉引擎 `FGameplayTagStackContainer` **开启了 `WithNetDeltaSerializer`**。
- 没有这个开关，引擎就不会走增量复制路径。这是"让容器具备增量复制"的关键一行。

---

## 四、写完后自查（对照铁律 11/12 两个核对）

写完 `.h` 后，对照这份源码核对：
1. **缺行核对**：`AddStack/RemoveStack/GetStackCount/ContainsTag/PreReplicatedRemove/PostReplicatedAdd/PostReplicatedChange/NetDeltaSerialize/TStructOpsTypeTraits` 一个都不能少。
2. **声明成对**：`.h` 里声明了 `GetDebugString`、`AddStack`、`RemoveStack`、三个复制回调——它们**都要在 `.cpp` 里实现**（下一步教）。

---

## 五、确认点

按铁律 29，你写完 `.h` 后说"下一步"，**我会先读你工程里的 `GameplayTagStack.h` 确认写完、内容对得上**，再教 `.cpp`（建 `_cpp.md`）。