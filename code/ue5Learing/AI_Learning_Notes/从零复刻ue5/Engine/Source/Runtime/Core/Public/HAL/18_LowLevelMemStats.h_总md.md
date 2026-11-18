# 18 — `HAL/LowLevelMemStats.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`（**98 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemStats.h`
> **✅ 本节可独立编译**——纯宏定义文件，只 include 2 个头。

---

## 一、这个文件解决什么问题（为什么）

LLM（Low Level Memory Tracker）能追踪「引擎各部分用了多少内存」。但追踪到的数据要**显示在 `stat LLM` 面板上**，就需要把它接到 UE 的**统计系统（Stats）**。`LowLevelMemStats.h` 就是这座桥：**把 LLM 的 19 个内存分类注册成统计计数器**。

**它全是宏，分 3 组**：

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

## 二、内部结构全景

```
LowLevelMemStats.h（98 行）
├─ 第 1~6 行      版权头 + #pragma once + 2 个 include
├─ 第 8~52 行     ① 统计声明宏（双分支）
│    ├─ 第 12~14 行   DECLARE_LLM_MEMORY_STAT
│    ├─ 第 15~17 行   DECLARE_LLM_MEMORY_STAT_EXTERN
│    ├─ 第 19~23 行   5 个 DECLARE_STATS_GROUP
│    ├─ 第 28~46 行   19 个 DECLARE_LLM_MEMORY_STAT_EXTERN
│    └─ 第 48~52 行   #else 空宏
├─ 第 54~94 行    ② 作用域宏（双分支）
│    ├─ 第 58~61 行   4 个 LLM_SCOPED_TAG_WITH_STAT*
│    ├─ 第 62~65 行   4 个 LLM_SCOPED_SINGLE_*_STAT_TAG*
│    ├─ 第 66 行      LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT
│    ├─ 第 67~70 行   2 个 LLM_SCOPE_DYNAMIC_STAT_OBJECTPATH*
│    ├─ 第 74~79 行   LLM_PUSH_STATS_FOR_ASSET_TAGS（双分支）
│    └─ 第 81~94 行   #else 空宏（12 个）
└─ 第 96 行       ③ LLM_SCOPED_TAG_WITH_OBJECT_IN_SET（已废弃 5.3）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/LowLevelMemStats.h` | 无（纯宏） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `HAL/LowLevelMemTracker.h` | `ENABLE_LOW_LEVEL_MEM_TRACKER` / `LLM_ENABLED_STAT_TAGS` / `LLM_ALLOW_ASSETS_TAGS` / `FLLMScope` / `ELLMTracker` / `ELLMTagSet` |
| `Stats/Stats.h` | `DECLARE_STAT` / `DEFINE_STAT` / `DECLARE_STATS_GROUP` / `EStatDataType` / `EStatFlags` |

**注意**：`FPlatformMemory::MCR_PhysicalLLM` 来自 `HAL/PlatformMemory.h`，本文件**没有 include**，靠 `LowLevelMemTracker.h` 间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\31_LowLevelMemStats.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `LowLevelMemTracker.h`（59199 字节，HAL 里最大的头）。