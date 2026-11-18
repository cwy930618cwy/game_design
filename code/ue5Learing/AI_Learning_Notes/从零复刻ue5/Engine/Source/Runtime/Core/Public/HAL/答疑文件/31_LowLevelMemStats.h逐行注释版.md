# 31 — `HAL/LowLevelMemStats.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`（**98 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`
> **✅ 本节可独立编译**——纯宏定义文件，只 include 2 个头。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

LLM（Low Level Memory Tracker）能追踪「引擎各部分用了多少内存」。但追踪到的数据要**显示在 `stat LLM` 面板上**，就需要把它接到 UE 的**统计系统（Stats）**。`LowLevelMemStats.h` 就是这座桥：**把 LLM 的 19 个内存分类注册成统计计数器**。

**它全是宏，分 4 组**：

| 组 | 内容 |
|---|---|
| **① 统计声明宏**（第 12~17 行） | `DECLARE_LLM_MEMORY_STAT` / `DECLARE_LLM_MEMORY_STAT_EXTERN` |
| **② 5 个统计组**（第 19~23 行） | `STATGROUP_LLMFULL` / `LLMPlatform` / `LLM` / `LLMOverhead` / `LLMAssets` |
| **③ 19 个汇总统计**（第 28~46 行） | `STAT_EngineSummaryLLM` / `Project` / `Networking` / `Audio` / `Total` / `Meshes` / `Physics` / `PhysX` / `Chaos` / `UObject` / `Animation` / `StaticMesh` / `Materials` / `Particles` / `Niagara` / `UI` / `Navigation` / `Textures` / `MediaStreaming` |
| **④ 作用域宏**（第 58~70 行） | `LLM_SCOPED_TAG_WITH_STAT` / `LLM_SCOPED_SINGLE_STAT_TAG` / `LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH` 等 11 个 |

**核心设计 ①：双分支（启用 / 空宏）**

```cpp
#if ENABLE_LOW_LEVEL_MEM_TRACKER && STATS
    // 真实实现
#else
    #define DECLARE_LLM_MEMORY_STAT(...)          // 空宏
    #define DECLARE_LLM_MEMORY_STAT_EXTERN(...)   // 空宏
#endif
```

关闭 LLM 或关闭 Stats 时，所有宏变成空 —— **零开销**。

**核心设计 ②：`DECLARE_LLM_MEMORY_STAT` 的两段式**

```cpp
#define DECLARE_LLM_MEMORY_STAT(CounterName, StatId, GroupId) \
    DECLARE_STAT(CounterName, StatId, GroupId, EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM); \
    static DEFINE_STAT(StatId)
```

- `DECLARE_STAT(...)` —— 声明统计（`int64` 类型 + `MCR_PhysicalLLM` 内存计数器）
- `static DEFINE_STAT(StatId)` —— 定义（本编译单元内可见）

**`EXTERN` 版**把 `static` 换成 `extern API`，用于跨模块共享。

**核心设计 ③：`LLM_SCOPED_SINGLE_STAT_TAG` 的「声明 + 打标签」二合一**

```cpp
#define LLM_SCOPED_SINGLE_STAT_TAG(Stat) \
    DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMFULL); \
    LLM_SCOPED_TAG_WITH_STAT(Stat, ELLMTracker::Default);
```

`#Stat` 是**字符串化运算符**：把 `Stat` 这个标识符变成字符串 `"Stat"`。所以调用方只需写：

```cpp
LLM_SCOPED_SINGLE_STAT_TAG(MyCustomTag);
```

就自动完成了「声明统计 + 打标签」两件事。

**核心设计 ④：`LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH`**

```cpp
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set) \
    LLM_SCOPE_DYNAMIC(FName(*Object->GetPathName()), \
        ELLMTracker::Default, Set, FLLMDynamicTagConstructorStatString(Object->GetPathName()))
```

用**对象路径名**作为动态标签 —— 这样 `stat LLM` 里能看到「每个资产用了多少内存」。

**说个场景**：

```cpp
// 在加载资产的代码里
LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Asset, ELLMTagSet::Assets);
// 之后 stat LLM 面板里能看到 "T_Rock.uasset: 12.5 MB"
```

> 🔑 **一句话**：`LowLevelMemStats.h` = **LLM 与 Stats 的「接线盒」**，把 19 个内存分类接成统计计数器，关闭时全变空宏。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 2 个 include（第 1~6 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入低层内存追踪器：ENABLE_LOW_LEVEL_MEM_TRACKER / LLM_ENABLED_STAT_TAGS 等宏。
#include "HAL/LowLevelMemTracker.h"
// 引入统计系统：DECLARE_STAT / DEFINE_STAT / DECLARE_STATS_GROUP。
#include "Stats/Stats.h"
```

**本段新元素**

- `HAL/LowLevelMemTracker.h` —— `ENABLE_LOW_LEVEL_MEM_TRACKER` / `LLM_ENABLED_STAT_TAGS` / `FLLMScope` / `ELLMTracker` / `ELLMTagSet`
- `Stats/Stats.h` —— `DECLARE_STAT` / `DEFINE_STAT` / `DECLARE_STATS_GROUP` / `EStatDataType` / `EStatFlags`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~6 行 | 2 个 include |

> 类比：像接线前先备两根线：一根接 LLM，一根接 Stats。

### 二、统计声明宏（双分支）（第 8~52 行）

```cpp
// 如果启用了低层内存追踪且启用了统计系统。
#if ENABLE_LOW_LEVEL_MEM_TRACKER && STATS
// 说明：LLM 统计实现宏；这些宏在 LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG 里公开使用，在 LLM 实现里私有使用。
/**
 * LLM 统计实现宏；这些宏在 LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG 里公开使用，在 LLM 实现里私有使用
 */
// DECLARE_LLM_MEMORY_STAT：声明并定义一个 LLM 内存统计计数器。
	#define DECLARE_LLM_MEMORY_STAT(CounterName,StatId,GroupId) \
// 声明统计（int64 类型，无标志，物理 LLM 内存计数器）。
		DECLARE_STAT(CounterName,StatId,GroupId,EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM); \
// 定义统计（static，本编译单元内可见）。
		static DEFINE_STAT(StatId)
// DECLARE_LLM_MEMORY_STAT_EXTERN：声明一个外部定义的 LLM 内存统计计数器。
	#define DECLARE_LLM_MEMORY_STAT_EXTERN(CounterName,StatId,GroupId, API) \
// 声明统计（int64 类型，无标志，物理 LLM 内存计数器）。
		DECLARE_STAT(CounterName,StatId,GroupId,EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM); \
// 外部定义（带 API 导出标记）。
		extern API DEFINE_STAT(StatId);

// 声明统计组 LLM FULL（完整 LLM 数据）。
	DECLARE_STATS_GROUP(TEXT("LLM FULL"), STATGROUP_LLMFULL, STATCAT_Advanced);
// 声明统计组 LLM Platform（平台层数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Platform"), STATGROUP_LLMPlatform, STATCAT_Advanced);
// 声明统计组 LLM Summary（汇总数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Summary"), STATGROUP_LLM, STATCAT_Advanced);
// 声明统计组 LLM Overhead（LLM 自身开销）。
	DECLARE_STATS_GROUP(TEXT("LLM Overhead"), STATGROUP_LLMOverhead, STATCAT_Advanced);
// 声明统计组 LLM Assets（资产数据）。
	DECLARE_STATS_GROUP(TEXT("LLM Assets"), STATGROUP_LLMAssets, STATCAT_Advanced);

// 说明：LLM 汇总统计，被 ELLMTagNames 引用。
	/*
	* LLM 汇总统计，被 ELLMTagNames 引用
	*/
// Engine：引擎自身内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Engine"), STAT_EngineSummaryLLM, STATGROUP_LLM, CORE_API);
// Project：项目内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Project"), STAT_ProjectSummaryLLM, STATGROUP_LLM, CORE_API);
// Networking：网络内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Networking"), STAT_NetworkingSummaryLLM, STATGROUP_LLM, CORE_API);
// Audio：音频内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Audio"), STAT_AudioSummaryLLM, STATGROUP_LLM, CORE_API);
// Total：总追踪内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Total"), STAT_TrackedTotalSummaryLLM, STATGROUP_LLM, CORE_API);
// Meshes：网格内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Meshes"), STAT_MeshesSummaryLLM, STATGROUP_LLM, CORE_API);
// Physics：物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Physics"), STAT_PhysicsSummaryLLM, STATGROUP_LLM, CORE_API);
// PhysX：PhysX 物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("PhysX"), STAT_PhysXSummaryLLM, STATGROUP_LLM, CORE_API);
// Chaos：Chaos 物理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Chaos"), STAT_ChaosSummaryLLM, STATGROUP_LLM, CORE_API);
// UObject：UObject 内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("UObject"), STAT_UObjectSummaryLLM, STATGROUP_LLM, CORE_API);
// Animation：动画内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Animation"), STAT_AnimationSummaryLLM, STATGROUP_LLM, CORE_API);
// StaticMesh：静态网格内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("StaticMesh"), STAT_StaticMeshSummaryLLM, STATGROUP_LLM, CORE_API);
// Materials：材质内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Materials"), STAT_MaterialsSummaryLLM, STATGROUP_LLM, CORE_API);
// Particles：粒子内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Particles"), STAT_ParticlesSummaryLLM, STATGROUP_LLM, CORE_API);
// Niagara：Niagara 特效内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Niagara"), STAT_NiagaraSummaryLLM, STATGROUP_LLM, CORE_API);
// UI：UI 内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("UI"), STAT_UISummaryLLM, STATGROUP_LLM, CORE_API);
// Navigation：导航内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Navigation"), STAT_NavigationSummaryLLM, STATGROUP_LLM, CORE_API);
// Textures：纹理内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("Textures"), STAT_TexturesSummaryLLM, STATGROUP_LLM, CORE_API);
// MediaStreaming：媒体流内存。
	DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("MediaStreaming"), STAT_MediaStreamingSummaryLLM, STATGROUP_LLM, CORE_API);

// 否则（未启用 LLM 或未启用统计）。
#else
// DECLARE_LLM_MEMORY_STAT：空宏。
	#define DECLARE_LLM_MEMORY_STAT(CounterName,StatId,GroupId)
// DECLARE_LLM_MEMORY_STAT_EXTERN：空宏。
	#define DECLARE_LLM_MEMORY_STAT_EXTERN(CounterName,StatId,GroupId, API)

// 条件编译结束。
#endif //ENABLE_LOW_LEVEL_MEM_TRACKER && STATS
```

**本段新元素**

- `#if ENABLE_LOW_LEVEL_MEM_TRACKER && STATS` —— 启用 LLM 且启用 Stats
- `DECLARE_LLM_MEMORY_STAT(CounterName, StatId, GroupId)` —— 声明 + 定义统计
- `DECLARE_STAT(...)` —— 声明统计（`ST_int64` + `MCR_PhysicalLLM`）
- `static DEFINE_STAT(StatId)` —— 定义（本编译单元可见）
- `DECLARE_LLM_MEMORY_STAT_EXTERN(..., API)` —— 声明 + 外部定义
- `extern API DEFINE_STAT(StatId)` —— 外部定义（带导出标记）
- `DECLARE_STATS_GROUP(TEXT("LLM FULL"), STATGROUP_LLMFULL, STATCAT_Advanced)` —— 声明统计组
- 5 个统计组：`LLMFULL` / `LLMPlatform` / `LLM` / `LLMOverhead` / `LLMAssets`
- 19 个 `DECLARE_LLM_MEMORY_STAT_EXTERN` —— 19 个内存分类
- `#else` —— 未启用时全变空宏

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 8~11 行 | 条件编译 + 说明 |
| 第 12~14 行 | `DECLARE_LLM_MEMORY_STAT` |
| 第 15~17 行 | `DECLARE_LLM_MEMORY_STAT_EXTERN` |
| 第 19~23 行 | 5 个统计组 |
| 第 25~46 行 | 19 个汇总统计 |
| 第 48~52 行 | `#else` 空宏 |

> 类比：像给 19 个部门各装一个电表：Engine、Project、Audio、Textures……每个都单独计量。

### 三、作用域宏（双分支）（第 54~94 行）

```cpp
// 说明：LLM 统计作用域宏（LLM 关闭或 LLM 统计标签关闭时都是空操作）。
/**
 * LLM 统计作用域宏（LLM 关闭或 LLM 统计标签关闭时都是空操作）
 */
// 如果启用了 LLM 且启用了 LLM 统计标签。
#if ENABLE_LOW_LEVEL_MEM_TRACKER && LLM_ENABLED_STAT_TAGS
// LLM_SCOPED_TAG_WITH_STAT：用统计名打一个 LLM 作用域标签。
#define LLM_SCOPED_TAG_WITH_STAT(Stat, Tracker) FLLMScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, ELLMTagSet::None, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, Tracker) FLLMScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, Set, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_NAME：用字符串名打标签。
#define LLM_SCOPED_TAG_WITH_STAT_NAME(StatName, Tracker) FLLMScope SCOPE_NAME(StatName, true /* bIsStatTag */, ELLMTagSet::None, Tracker);
// LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET(StatName, Set, Tracker) FLLMScope SCOPE_NAME(StatName, true /* bIsStatTag */, Set, Tracker);
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG：声明 + 打一个平台层统计标签。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG(Stat) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMPlatform); LLM_SCOPED_TAG_WITH_STAT(Stat, ELLMTracker::Platform);
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET(Stat, Set) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMPlatform); LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, ELLMTracker::Platform);
// LLM_SCOPED_SINGLE_STAT_TAG：声明 + 打一个完整层统计标签。
#define LLM_SCOPED_SINGLE_STAT_TAG(Stat) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMFULL); LLM_SCOPED_TAG_WITH_STAT(Stat, ELLMTracker::Default);
// LLM_SCOPED_SINGLE_STAT_TAG_IN_SET：同上，但指定标签集。
#define LLM_SCOPED_SINGLE_STAT_TAG_IN_SET(Stat, Set) DECLARE_LLM_MEMORY_STAT(TEXT(#Stat), Stat, STATGROUP_LLMFULL); LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, ELLMTracker::Default);
// LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT：暂停追踪并记录指定数量。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT(Stat, Amount, Tracker) FLLMPauseScope SCOPE_NAME(GET_STATFNAME(Stat), true /* bIsStatTag */, Amount, Tracker, ELLMAllocType::None);
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH：用对象路径名打动态标签。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set) LLM_SCOPE_DYNAMIC(FName(*Object->GetPathName()), \
// 续行：默认追踪器 + 标签集 + 动态标签构造器。
					ELLMTracker::Default, Set, FLLMDynamicTagConstructorStatString(Object->GetPathName()))
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME：用 FName 路径打动态标签。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME(ObjectPath, Set) LLM_SCOPE_DYNAMIC(ObjectPath, \
// 续行：默认追踪器 + 标签集 + 动态标签构造器。
					ELLMTracker::Default, Set, FLLMDynamicTagConstructorStatString((ObjectPath).ToString()))

// 说明：追踪资产时，每个资产后推送统计以更新线程。
// 追踪资产时，每个资产后推送统计以更新线程
// 说明：目前未使用，但以后可能用于优化。
// 目前未使用，但以后可能用于优化
// 如果允许资产标签。
#if LLM_ALLOW_ASSETS_TAGS
// 被注释掉的旧实现。
//#define LLM_PUSH_STATS_FOR_ASSET_TAGS() if (FLowLevelMemTracker::Get().IsTagSetActive(ELLMTagSet::Assets)) {}
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 否则。
#else
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 条件编译结束。
#endif

// 否则（未启用 LLM 或未启用统计标签）。
#else
// LLM_SCOPED_TAG_WITH_STAT：空宏。
#define LLM_SCOPED_TAG_WITH_STAT(...)
// LLM_SCOPED_TAG_WITH_STAT_IN_SET：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_IN_SET(...)
// LLM_SCOPED_TAG_WITH_STAT_NAME：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_NAME(...)
// LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET：空宏。
#define LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET(...)
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG：空宏。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG(...)
// LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET：空宏。
#define LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET(...)
// LLM_SCOPED_SINGLE_STAT_TAG：空宏。
#define LLM_SCOPED_SINGLE_STAT_TAG(...)
// LLM_SCOPED_SINGLE_STAT_TAG_IN_SET：空宏。
#define LLM_SCOPED_SINGLE_STAT_TAG_IN_SET(...)
// LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT：空宏。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT(...)
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH：空宏。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(...)
// LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME：空宏。
#define LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME(...)
// LLM_PUSH_STATS_FOR_ASSET_TAGS：空宏。
#define LLM_PUSH_STATS_FOR_ASSET_TAGS()
// 条件编译结束。
#endif
```

**本段新元素**

- `#if ENABLE_LOW_LEVEL_MEM_TRACKER && LLM_ENABLED_STAT_TAGS` —— 启用 LLM 且启用统计标签
- `LLM_SCOPED_TAG_WITH_STAT(Stat, Tracker)` —— 用统计名打标签
- `LLM_SCOPED_TAG_WITH_STAT_IN_SET(Stat, Set, Tracker)` —— 指定标签集
- `LLM_SCOPED_TAG_WITH_STAT_NAME(StatName, Tracker)` —— 用字符串名
- `LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET(...)` —— 字符串名 + 标签集
- `LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG(Stat)` —— 声明 + 打平台层标签
- `LLM_SCOPED_SINGLE_STAT_TAG(Stat)` —— 声明 + 打完整层标签
- `TEXT(#Stat)` —— **字符串化运算符**（标识符 → 字符串）
- `LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT(Stat, Amount, Tracker)` —— 暂停追踪并记录数量
- `LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set)` —— 用对象路径打动态标签
- `LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH_FNAME(ObjectPath, Set)` —— 用 FName 路径
- `LLM_PUSH_STATS_FOR_ASSET_TAGS()` —— 空宏（目前未使用）
- `#else` —— 12 个空宏

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 54~57 行 | 条件编译 + 说明 |
| 第 58~61 行 | 4 个 `LLM_SCOPED_TAG_WITH_STAT*` |
| 第 62~65 行 | 4 个 `LLM_SCOPED_SINGLE_*_STAT_TAG*` |
| 第 66 行 | `LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT` |
| 第 67~70 行 | 2 个 `LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH*` |
| 第 72~79 行 | `LLM_PUSH_STATS_FOR_ASSET_TAGS`（双分支） |
| 第 81~94 行 | `#else` 12 个空宏 |

> 类比：像给每个动作贴标签：这个动作属于哪个部门、花了多少内存。

### 四、已废弃宏（第 96~98 行）

```cpp
// LLM_SCOPED_TAG_WITH_OBJECT_IN_SET：已废弃（5.3），改用 LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH。
#define LLM_SCOPED_TAG_WITH_OBJECT_IN_SET(Object, Set) UE_DEPRECATED_MACRO(5.3, "Use LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH instead") LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH(Object, Set);


```

**本段新元素**

- `LLM_SCOPED_TAG_WITH_OBJECT_IN_SET(Object, Set)` —— 已废弃（5.3）
- `UE_DEPRECATED_MACRO(5.3, "...")` —— 废弃标记
- 改用 `LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 96 行 | 废弃宏定义 |

> 类比：像老式插头：还能用，但已经贴了「请换新标准」的标签。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 2 个 include | 1~6 | 版权头 + 2 个 include |
| 统计声明宏（双分支） | 8~52 | 统计声明宏（双分支） |
| 作用域宏（双分支） | 54~94 | 作用域宏（双分支） |
| 已废弃宏 | 96~98 | 已废弃宏 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#define X(a, b) \` | 多行宏 | `#define DECLARE_LLM_MEMORY_STAT(CounterName,StatId,GroupId) \` |
| `#Stat` | 字符串化运算符（标识符 → 字符串） | `TEXT(#Stat)` |
| `#if A && B` | 条件编译（与） | `#if ENABLE_LOW_LEVEL_MEM_TRACKER && STATS` |
| `#define X(...)` | 可变参数空宏 | `#define LLM_SCOPED_TAG_WITH_STAT(...)` |
| `UE_DEPRECATED_MACRO(v, "msg")` | 标记宏废弃 | `UE_DEPRECATED_MACRO(5.3, "...")` |
| `DECLARE_STATS_GROUP(Name, Id, Cat)` | 声明统计组 | `DECLARE_STATS_GROUP(TEXT("LLM FULL"), STATGROUP_LLMFULL, STATCAT_Advanced)` |
| `DECLARE_STAT(Name, Id, Group, Type, Flags, MemCounter)` | 声明统计 | `DECLARE_STAT(..., EStatDataType::ST_int64, EStatFlags::None, FPlatformMemory::MCR_PhysicalLLM)` |
| `DEFINE_STAT(Id)` | 定义统计 | `static DEFINE_STAT(StatId)` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 98 行 | 177 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 72 行 | 72 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |
| 被注释掉的代码 | 保留原文 | 保留原文 | 引擎原文如此 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`（177 行）
2. 脚本验收：`codeOurs=72 / codeEngine=72 / diff=0`
3. 确认后下一个：`HAL\LowLevelMemTracker.h`（59199 字节，HAL 里最大的头）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`（98 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`（177 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\18_LowLevelMemStats.h_总md.md`