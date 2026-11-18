# 32 — `HAL/LowLevelMemTracker.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`（**1127 行 / 59KB**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`
> **⚠️ 本节不能独立编译**——依赖 `FTagTrace` / `AutoRTFM` / `FName`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。
> **本文件特殊**：28 个块注释统一压成单行 `//` 中文注释，保证代码行零差异。

---

## 一、这个文件解决什么问题（为什么）

`stat LLM` 面板能告诉你「Textures 用了 800MB、Meshes 用了 300MB」。这套系统的核心就是 **LLM（Low Level Memory Tracker）**：它在**每一次 malloc/free** 时记录「这次分配属于哪个标签」，然后按标签汇总。

`LowLevelMemTracker.h` 是 LLM 的**全部公开接口**，1127 行，包含：

| 类别 | 内容 |
|---|---|
| **配置宏** | `ENABLE_LOW_LEVEL_MEM_TRACKER` / `LLM_ALLOW_ASSETS_TAGS` / `LLM_ALLOW_STATS` / `LLM_ENABLED_STAT_TAGS` |
| **断言宏** | `LLMCheck` / `LLMCheckf` / `LLMEnsure` |
| **枚举** | `ELLMTracker`（2 个）/ `ELLMTagSet`（3 个）/ `ELLMTag`（100+ 个）/ `ELLMAllocType`（4 个）/ `ESizeParams` |
| **标签列表** | `LLM_ENUM_GENERIC_TAGS(macro)` —— 100+ 个标签的 X-Macro |
| **作用域宏** | `LLM_SCOPE` / `LLM_SCOPE_DYNAMIC` / `LLM_PLATFORM_SCOPE` / `LLM_SCOPED_PAUSE_TRACKING` / `LLM_REALLOC_SCOPE` 等 20+ 个 |
| **标签定义宏** | `LLM_DEFINE_TAG` / `LLM_DECLARE_TAG` / `LLM_DEFINE_BOOTSTRAP_TAG` |
| **核心类** | `FLowLevelMemTracker`（主追踪器）/ `FLLMScope` / `FLLMScopeDynamic` / `FLLMPauseScope` / `FLLMClearScope` / `FLLMScopeFromPtr` / `FLLMTagDeclaration` / `FLLMAllocator` |

**核心设计 ①：两个追踪器**（第 120~126 行）

```cpp
enum class ELLMTracker : uint8
{
    Platform,   // 平台层：追踪内存管理系统（如 Binned2 的页）
    Default,    // 默认层：追踪引擎分配（GMalloc）
    Max,
};
```

**为什么要两个**：`Platform` 追踪「分配器自己占了多少内存」，`Default` 追踪「游戏逻辑占了多少内存」。两者相加才是真实占用。

**核心设计 ②：X-Macro 标签列表**（第 163~280 行）

```cpp
#define LLM_ENUM_GENERIC_TAGS(macro) \
    macro(Untagged, "Untagged", NAME_None, NAME_None, -1) \
    macro(Total, "Total", GET_STATFNAME(STAT_TotalLLM), ..., -1) \
    macro(Audio, "Audio", ..., ELLMTag::Audio) \
    ... 100+ 行
```

**为什么用 X-Macro**：同一份列表要展开成 3 种东西（枚举值、名字表、父标签表），X-Macro 保证三者永远同步。

**核心设计 ③：`LLM_SCOPE` 的 RAII**（第 344~345 行）

```cpp
#define LLM_SCOPE(Tag) \
    FLLMScope SCOPE_NAME(Tag, false, ELLMTagSet::None, ELLMTracker::Default); \
    UE_MEMSCOPE(Tag)
```

`SCOPE_NAME` 用 `__LINE__` 生成唯一变量名，所以同一作用域可以写多个 `LLM_SCOPE`。

**核心设计 ④：`LLM_SCOPE_DYNAMIC` 的 do/while**（第 346~371 行）

```cpp
#define LLM_SCOPE_DYNAMIC(UniqueName, Tracker, TagSet, Constructor) \
    FLLMScopeDynamic SCOPE_NAME(Tracker, TagSet); \
    UE_MEMSCOPE_UNINITIALIZED(__LINE__); \
    do { ... } while (false) /* do/while is added to require a semicolon */
```

`do/while(false)` 是为了**强制要求分号** —— 否则 `if (x) LLM_SCOPE_DYNAMIC(...)` 会出问题。

**核心设计 ⑤：`LLM_DEFINE_TAG` 的全局实例**（第 423 行）

```cpp
#define LLM_DEFINE_TAG(UniqueNameWithUnderscores, ...) \
    FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)(TEXT(#UniqueNameWithUnderscores), ##__VA_ARGS__)
```

定义一个**全局 `FLLMTagDeclaration` 实例**，它的构造函数会自动把标签注册到 LLM。

**说个场景**：

```cpp
// 定义一个自定义标签
LLM_DEFINE_TAG(MyGame_Weapons, TEXT("Weapons"), NAME_None, GET_STATFNAME(STAT_MyWeaponsLLM));

void LoadWeapon()
{
    LLM_SCOPE_BYTAG(MyGame_Weapons);   // 之后所有分配都归到 Weapons
    // ... 加载武器 ...
}
```

**双分支设计**：`#if ENABLE_LOW_LEVEL_MEM_TRACKER` ... `#else`（第 1086~1127 行）—— 关闭 LLM 时所有宏变空，**零开销**。

> 🔑 **一句话**：`LowLevelMemTracker.h` = **内存追踪的「总控台」**，2 个追踪器 + 100+ 个标签 + 20+ 个作用域宏。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 平台支持判断 + 配置宏（第 1~52 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint8 / uint32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入 LLM 配置开关：LLM_ENABLED_IN_CONFIG。
#include "LowLevelMemTrackerDefines.h" // LLM_ENABLED_IN_CONFIG
// 引入标签追踪：FTagTrace。
#include "ProfilingDebugging/TagTrace.h"
// 引入 AutoRTFM：事务内存支持。
#include "AutoRTFM.h"

// 如果平台没定义 PLATFORM_SUPPORTS_LLM（是否支持 LLM）。
#ifndef PLATFORM_SUPPORTS_LLM
// 默认支持。
#define PLATFORM_SUPPORTS_LLM 1
// 条件编译结束。
#endif

// LLM 目前与 PLATFORM_USES_FIXED_GMalloc_CLASS 不兼容，因为 LLM 激活得太早
// 这不是问题，因为固定 GMalloc 只在 Test/Shipping 构建里用
// LLM_ENABLED_ON_PLATFORM：平台支持 LLM 且没用固定 GMalloc 类。
#define LLM_ENABLED_ON_PLATFORM (PLATFORM_SUPPORTS_LLM && !PLATFORM_USES_FIXED_GMalloc_CLASS)

// 如果外部单独定义了 ENABLE_LOW_LEVEL_MEM_TRACKER。
#ifdef ENABLE_LOW_LEVEL_MEM_TRACKER
// 直接编译报错：这个宏现在是派生宏，不能单独定义。
#error ENABLE_LOW_LEVEL_MEM_TRACKER is now a derived define that should not be defined separately. Define LLM_ENABLED_IN_CONFIG (build environment only) or LLM_ENABLED_ON_PLATFORM (build environment or c++ header) instead.
// 条件编译结束。
#endif
// ENABLE_LOW_LEVEL_MEM_TRACKER：配置启用 且 平台支持。
#define ENABLE_LOW_LEVEL_MEM_TRACKER (LLM_ENABLED_IN_CONFIG && LLM_ENABLED_ON_PLATFORM)

// 如果启用了 LLM。
#if ENABLE_LOW_LEVEL_MEM_TRACKER

// 配置 LLM 的公开宏；私有宏见 LowLevelMemTracker.cpp

// LLM_ALLOW_ASSETS_TAGS：设为 1 启用资产标签的运行时开关，0 禁用。
// 启用该宏即使资产标签关闭也会产生额外 CPU 开销。
// 启用后可用命令行 -llmtagsets=assets 在运行时打开。
// 打开会产生大量 stat id，CPU 开销很高。
// 启用且运行时打开后，资产标签会报告每次分配所属的资产。
// 游戏里可用 'Stat LLMAssets' 查看 LLM 资产数据。
// 如果没定义 LLM_ALLOW_ASSETS_TAGS（是否允许资产标签）。
#ifndef LLM_ALLOW_ASSETS_TAGS
// 默认 0（关闭）。
	#define LLM_ALLOW_ASSETS_TAGS 0
// 条件编译结束。
#endif

// LLM_ALLOW_UOBJECTCLASSES_TAGS 在 LLM_ALLOW_ASSETS_TAGS 关闭时启用
// 目的是避免同时激活 3 个标签集带来的内存开销。
// 如果没定义 LLM_ALLOW_UOBJECTCLASSES_TAGS。
#ifndef LLM_ALLOW_UOBJECTCLASSES_TAGS
// 默认与 LLM_ALLOW_ASSETS_TAGS 相反（避免同时开 3 个标签集）。
	#define LLM_ALLOW_UOBJECTCLASSES_TAGS !LLM_ALLOW_ASSETS_TAGS
// 条件编译结束。
#endif

// LLM_ALLOW_STATS：设为 1 允许统计作为标签，0 禁用。
// 启用后 LLM_SCOPED_TAG_WITH_STAT 宏可用，每个 stat 创建一个 LLM 标签，代价是每次分配更多 LLM 内存。
// 打开它和 LLM_ALLOW_NAMES_TAGS 的每次分配内存开销相同。
// 两个都打开没有额外开销。
// 如果没定义 LLM_ALLOW_STATS（是否允许统计作为标签）。
#ifndef LLM_ALLOW_STATS
// 默认 0（关闭）。
	#define LLM_ALLOW_STATS 0
// 条件编译结束。
#endif

// 启用统计标签的条件：(1) 允许统计 或 (2) 允许资产标签（资产标签用统计宏记录资产作用域）
// LLM_ENABLED_STAT_TAGS：允许统计 或 允许资产标签 时启用统计标签。
#define LLM_ENABLED_STAT_TAGS (LLM_ALLOW_STATS || LLM_ALLOW_ASSETS_TAGS)
```

**本段新元素**

- `CoreTypes.h` / `LowLevelMemTrackerDefines.h` / `ProfilingDebugging/TagTrace.h` / `AutoRTFM.h` —— 4 个 include
- `PLATFORM_SUPPORTS_LLM` —— 平台是否支持 LLM（默认 1）
- `LLM_ENABLED_ON_PLATFORM` —— 平台支持 且 没用固定 GMalloc 类
- `#error` —— 单独定义 `ENABLE_LOW_LEVEL_MEM_TRACKER` 直接报错
- `ENABLE_LOW_LEVEL_MEM_TRACKER` —— 配置启用 且 平台支持（派生宏）
- `LLM_ALLOW_ASSETS_TAGS` —— 是否允许资产标签（默认 0）
- `LLM_ALLOW_UOBJECTCLASSES_TAGS` —— 默认与资产标签相反
- `LLM_ALLOW_STATS` —— 是否允许统计作为标签（默认 0）
- `LLM_ENABLED_STAT_TAGS` —— 允许统计 或 允许资产标签

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1~8 行 | 版权头 + 4 个 include |
| 第 10~16 行 | 平台支持判断 |
| 第 18~21 行 | `ENABLE_LOW_LEVEL_MEM_TRACKER` 派生 |
| 第 23~52 行 | 4 个配置宏 |

> 类比：像开工前的总开关：先确认平台支不支持，再决定开哪些子功能。

### 二、断言宏（LLMCheck / LLMCheckf / LLMEnsure）（第 54~108 行）

```cpp
// 引入 TArray：动态数组。
#include "Containers/Array.h"
// 引入 TArrayView：数组视图。
#include "Containers/ArrayView.h"
// 引入字符串前置声明。
#include "Containers/StringFwd.h"
// 引入临界区：FCriticalSection。
#include "HAL/CriticalSection.h"
// 引入平台 C 运行时。
#include "HAL/PlatformCrt.h"
// 引入平台杂项：FPlatformMisc。
#include "HAL/PlatformMisc.h"
// 引入对齐模板：Align。
#include "Templates/AlignmentTemplates.h"
// 引入模板工具：Forward。
#include "Templates/UnrealTemplate.h"
// 引入 FName：名字类型。
#include "UObject/NameTypes.h"
// 引入内置名字：NAME_None。
#include "UObject/UnrealNames.h"

// 引入标准库 atomic。
#include <atomic>

// 前置声明 FTagTrace。
class FTagTrace;

// 如果开启了 check。
#if DO_CHECK

// 命名空间 UE::LLMPrivate：LLM 内部实现。
namespace UE::LLMPrivate
{

// HandleAssert：处理断言（打日志 + 可选抛异常）。
bool HandleAssert(bool bLog, const TCHAR* Format, ...);

// LLMEnsure 用这个生成「每个调用点一个 bool」，靠 lambda 唯一实例化模板。
// TrueOnFirstCallOnly：第一次调用返回 true，之后返回 false。
template <typename Type>
// 函数体开始。
bool TrueOnFirstCallOnly(const Type&)
{
// 静态标志，初始 true。
	static bool bValue = true;
// 保存当前值。
	bool Result = bValue;
// 置为 false。
	bValue = false;
// 返回结果。
	return Result;
// 函数体结束。
}

// 命名空间结束。
} // UE::LLMPrivate

// 如果不是代码分析模式。
#if !USING_CODE_ANALYSIS
// LLMTrueOnFirstCallOnly：用空 lambda 实例化模板。
	#define LLMTrueOnFirstCallOnly			UE::LLMPrivate::TrueOnFirstCallOnly([]{})
// 否则（代码分析模式）。
#else
// 直接返回 false。
	#define LLMTrueOnFirstCallOnly			false
// 条件编译结束。
#endif

// LLMCheckMessage：拼出 check 失败消息。
#define LLMCheckMessage(expr)          TEXT("LLM check failed: %s [File:%s] [Line: %d]\r\n"),             TEXT(#expr), TEXT(__FILE__), __LINE__
// LLMCheckfMessage：拼出带格式的 check 失败消息。
#define LLMCheckfMessage(expr, format) TEXT("LLM check failed: %s [File:%s] [Line: %d]\r\n") format TEXT("\r\n"),      TEXT(#expr), TEXT(__FILE__), __LINE__
// LLMEnsureMessage：拼出 ensure 失败消息。
#define LLMEnsureMessage(expr)         TEXT("LLM ensure failed: %s [File:%s] [Line: %d]\r\n"),            TEXT(#expr), TEXT(__FILE__), __LINE__

// LLMCheck：断言失败就抛异常。
#define LLMCheck(expr)					do { if (UNLIKELY(!(expr))) { UE::LLMPrivate::HandleAssert(true, LLMCheckMessage(expr));                         FPlatformMisc::RaiseException(1); } } while(false)
// LLMCheckf：带格式的断言。
#define LLMCheckf(expr,format,...)		do { if (UNLIKELY(!(expr))) { UE::LLMPrivate::HandleAssert(true, LLMCheckfMessage(expr, format), ##__VA_ARGS__); FPlatformMisc::RaiseException(1); } } while(false)
// LLMEnsure：断言失败只报一次（不抛异常）。
#define LLMEnsure(expr) (LIKELY(!!(expr)) || UE::LLMPrivate::HandleAssert(LLMTrueOnFirstCallOnly, LLMEnsureMessage(expr)))

// 否则（未开启 check）。
#else

// LLMCheck：空宏。
#define LLMCheck(expr)
// LLMCheckf：空宏。
#define LLMCheckf(expr,...)
// LLMEnsure：直接求值。
#define LLMEnsure(expr)	(!!(expr))

// 条件编译结束。
#endif
```

**本段新元素**

- 9 个 include + `<atomic>` —— 容器、锁、平台、模板、FName
- `UE::LLMPrivate::HandleAssert(bLog, Format, ...)` —— 处理断言
- `TrueOnFirstCallOnly<T>(const T&)` —— 第一次调用返回 true（靠 lambda 唯一实例化模板）
- `LLMTrueOnFirstCallOnly` —— 宏包装
- `LLMCheckMessage(expr)` / `LLMCheckfMessage` / `LLMEnsureMessage` —— 3 个消息宏
- `LLMCheck(expr)` —— 断言失败抛异常
- `LLMCheckf(expr, format, ...)` —— 带格式的断言
- `LLMEnsure(expr)` —— 断言失败只报一次（不抛异常）
- `#else` —— 未开启 check 时全变空宏

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 54~65 行 | 9 个 include + `<atomic>` |
| 第 69~86 行 | `HandleAssert` + `TrueOnFirstCallOnly` |
| 第 88~92 行 | `LLMTrueOnFirstCallOnly` 双分支 |
| 第 94~96 行 | 3 个消息宏 |
| 第 98~100 行 | 3 个断言宏 |
| 第 102~108 行 | `#else` 空宏 |

> 类比：像安全检查：`LLMCheck` 是「出事就停机」，`LLMEnsure` 是「出事只报一次，继续跑」。

### 三、`ELLMTracker` + `ELLMTagSet` + `ESizeParams`（第 110~160 行）

```cpp
// LLM_TAG_TYPE：标签类型（uint8）。
#define LLM_TAG_TYPE uint8

// 估算 LLM 在约 400 万次分配的游戏上需要的最大内存量。
// 确保主机上开启了调试内存（没开会显示屏幕警告）
// （目前只在 PS4 上用来阻止它预先保留一大块。新的内存系统会去掉这个）
// LLM_MEMORY_OVERHEAD：LLM 自身预估内存开销（600MB）。
#define LLM_MEMORY_OVERHEAD (600LL*1024*1024)

// LLM 追踪器
// ELLMTracker：LLM 追踪器类型。
enum class ELLMTracker : uint8
{
// Platform：平台层追踪器（追踪内存管理系统）。
	Platform,
// Default：默认追踪器（追踪引擎分配）。
	Default,

// Max：哨兵值。
	Max,
// 枚举体结束。
};

// 可选标签，需用命令行 -llmtagsets=x,y,z 启用
// ELLMTagSet：可选标签集（需命令行 -llmtagsets=x,y,z 启用）。
enum class ELLMTagSet : uint8
{
// None：无标签集。
	None,
// 如果允许资产标签。
#if LLM_ALLOW_ASSETS_TAGS
// Assets：资产标签集。
	Assets,
// AssetClasses：资产类标签集。
	AssetClasses,
// 否则如果允许 UObject 类标签。
#elif LLM_ALLOW_UOBJECTCLASSES_TAGS
// UObjectClasses：UObject 类标签集。
	UObjectClasses,
// 条件编译结束。
#endif // LLM_ALLOW_UOBJECTCLASSES_TAGS
	
// Max：哨兵值（加新标签集时注意 ShouldReduceThreads / IsAssetTagForAssets）。
	Max, // note: see FLowLevelMemTracker::ShouldReduceThreads and IsAssetTagForAssets if you add any asset-style tagsets
// 枚举体结束。
};

// 请求标签数据大小时用的尺寸参数标志。
// 命名空间 UE::LLM。
namespace UE::LLM
{

// ESizeParams：请求标签数据大小时的参数标志。
enum class ESizeParams : uint8
{
// Default：默认（同 ReportCurrent）。
	Default = 0,
// ReportCurrent：报告当前值。
	ReportCurrent = 0,
// ReportPeak：报告峰值。
	ReportPeak = 1,
// RelativeToSnapshot：相对于快照。
	RelativeToSnapshot = 2
// 枚举体结束。
};

// 生成位运算。
ENUM_CLASS_FLAGS(ESizeParams);

// 命名空间结束。
} // UE::LLM
```

**本段新元素**

- `LLM_TAG_TYPE` —— 标签类型（`uint8`）
- `LLM_MEMORY_OVERHEAD` —— LLM 自身预估开销（600MB）
- `enum class ELLMTracker : uint8` —— 追踪器类型
- `Platform` —— 平台层（追踪内存管理系统）
- `Default` —— 默认层（追踪引擎分配）
- `enum class ELLMTagSet : uint8` —— 可选标签集
- `None` / `Assets` / `AssetClasses` / `UObjectClasses` / `Max` —— 5 个值（条件编译）
- `enum class ESizeParams : uint8` —— 尺寸参数
- `Default` / `ReportCurrent` / `ReportPeak` / `RelativeToSnapshot` —— 4 个值
- `ENUM_CLASS_FLAGS(ESizeParams)` —— 生成位运算

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 110~115 行 | `LLM_TAG_TYPE` / `LLM_MEMORY_OVERHEAD` |
| 第 120~126 行 | `ELLMTracker` |
| 第 131~142 行 | `ELLMTagSet` |
| 第 147~160 行 | `ESizeParams` |

> 类比：像两本账本：一本记「分配器自己花的钱」（Platform），一本记「业务花的钱」（Default）。

### 四、`LLM_ENUM_GENERIC_TAGS` —— 100+ 个标签（第 163~280 行）

```cpp
#define LLM_ENUM_GENERIC_TAGS(macro) \
	macro(Untagged,								"Untagged",						NAME_None,													NAME_None,										-1)\
	macro(Paused,								"Paused",						NAME_None,													NAME_None,										-1)\
	macro(Total,								"Total",						GET_STATFNAME(STAT_TotalLLM),								GET_STATFNAME(STAT_TrackedTotalSummaryLLM),		-1)\
	macro(Untracked,							"Untracked",					GET_STATFNAME(STAT_UntrackedLLM),							GET_STATFNAME(STAT_TrackedTotalSummaryLLM),		-1)\
	macro(PlatformTotal,						"Total",						GET_STATFNAME(STAT_PlatformTotalLLM),						NAME_None,										-1)\
	macro(TrackedTotal,							"TrackedTotal",					GET_STATFNAME(STAT_TrackedTotalLLM),						GET_STATFNAME(STAT_TrackedTotalSummaryLLM),		-1)\
	macro(UntaggedTotal,						"Untagged",						GET_STATFNAME(STAT_UntaggedTotalLLM),						NAME_None,										-1)\
	macro(WorkingSetSize,						"WorkingSetSize",				GET_STATFNAME(STAT_WorkingSetSizeLLM),						GET_STATFNAME(STAT_TrackedTotalSummaryLLM),		-1)\
	macro(PagefileUsed,							"PagefileUsed",					GET_STATFNAME(STAT_PagefileUsedLLM),						GET_STATFNAME(STAT_TrackedTotalSummaryLLM),		-1)\
	macro(PlatformTrackedTotal,					"TrackedTotal",					GET_STATFNAME(STAT_PlatformTrackedTotalLLM),				NAME_None,										-1)\
	macro(PlatformUntaggedTotal,				"Untagged",						GET_STATFNAME(STAT_PlatformUntaggedTotalLLM),				NAME_None,										-1)\
	macro(PlatformUntracked,					"Untracked",					GET_STATFNAME(STAT_PlatformUntrackedLLM),					NAME_None,										-1)\
	macro(PlatformOverhead,						"LLMOverhead",					GET_STATFNAME(STAT_PlatformOverheadLLM),					NAME_None,										-1)\
	macro(PlatformOSAvailable,					"OSAvailable",					GET_STATFNAME(STAT_PlatformOSAvailableLLM),					NAME_None,										-1)\
	/*FMalloc 是只保留给平台追踪器的特殊标签。它配合 ELLMAllocType::FMalloc 计算 ELLMTag::FMallocUnused。 */
	macro(FMalloc,								"FMalloc",						GET_STATFNAME(STAT_FMallocLLM),								NAME_None,										-1)\
	macro(FMallocUnused,						"FMallocUnused",				GET_STATFNAME(STAT_FMallocUnusedLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(RHIUnused,							"RHIUnused",					GET_STATFNAME(STAT_RHIUnusedLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(ThreadStack,							"ThreadStack",					GET_STATFNAME(STAT_ThreadStackLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(ThreadStackPlatform,					"ThreadStack",					GET_STATFNAME(STAT_ThreadStackPlatformLLM),					NAME_None,										-1)\
	macro(ProgramSizePlatform,					"ProgramSize",					GET_STATFNAME(STAT_ProgramSizePlatformLLM),					NAME_None,										-1)\
	macro(ProgramSize,							"ProgramSize",					GET_STATFNAME(STAT_ProgramSizeLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(BackupOOMMemoryPoolPlatform,			"OOMBackupPool",				GET_STATFNAME(STAT_OOMBackupPoolPlatformLLM),				NAME_None,										-1)\
	macro(BackupOOMMemoryPool,					"OOMBackupPool",				GET_STATFNAME(STAT_OOMBackupPoolLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(GenericPlatformMallocCrash,			"GenericPlatformMallocCrash",	GET_STATFNAME(STAT_GenericPlatformMallocCrashLLM),			GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(GenericPlatformMallocCrashPlatform,	"GenericPlatformMallocCrash",	GET_STATFNAME(STAT_GenericPlatformMallocCrashPlatformLLM),	GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	/* 任何未被其他分类追踪的低层内存。 */
	macro(EngineMisc,							"EngineMisc",					GET_STATFNAME(STAT_EngineMiscLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	/* 任何从任务图启动但没有自己分类的任务。应该相当少。 */
	macro(TaskGraphTasksMisc,					"TaskGraphMiscTasks",			GET_STATFNAME(STAT_TaskGraphTasksMiscLLM),					GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(LinearAllocator,						"LinearAllocator",				GET_STATFNAME(STAT_LinearAllocatorLLM),						NAME_None,										-1)\
	macro(Audio,								"Audio",						GET_STATFNAME(STAT_AudioLLM),								GET_STATFNAME(STAT_AudioSummaryLLM),			-1)\
	macro(AudioMisc,							"AudioMisc",					GET_STATFNAME(STAT_AudioMiscLLM),							GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioSoundWaves,						"AudioSoundWaves",				GET_STATFNAME(STAT_AudioSoundWavesLLM),						GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioSoundWaveProxies,				"AudioSoundWaveProxies",		GET_STATFNAME(STAT_AudioSoundWaveProxiesLLM),				GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioMixer,							"AudioMixer",					GET_STATFNAME(STAT_AudioMixerLLM),							GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioMixerPlugins,					"AudioMixerPlugins",			GET_STATFNAME(STAT_AudioMixerPluginsLLM),					GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioPrecache,						"AudioPrecache",				GET_STATFNAME(STAT_AudioPrecacheLLM),						GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioDecompress,						"AudioDecompress",				GET_STATFNAME(STAT_AudioDecompressLLM),						GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioRealtimePrecache,				"AudioRealtimePrecache",		GET_STATFNAME(STAT_AudioRealtimePrecacheLLM),				GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioFullDecompress,					"AudioFullDecompress",			GET_STATFNAME(STAT_AudioFullDecompressLLM),					GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioStreamCache,						"AudioStreamCache",				GET_STATFNAME(STAT_AudioStreamCacheLLM),					GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioStreamCacheCompressedData,		"AudioStreamCacheCompressedData",GET_STATFNAME(STAT_AudioStreamCacheCompressedDataLLM),		GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(AudioSynthesis,						"AudioSynthesis",				GET_STATFNAME(STAT_AudioSynthesisLLM),						GET_STATFNAME(STAT_AudioSummaryLLM),			ELLMTag::Audio)\
	macro(RealTimeCommunications,				"RealTimeCommunications",		GET_STATFNAME(STAT_RealTimeCommunicationsLLM),				NAME_None,										-1)\
	macro(FName,								"FName",						GET_STATFNAME(STAT_FNameLLM),								GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(Networking,							"Networking",					GET_STATFNAME(STAT_NetworkingLLM),							GET_STATFNAME(STAT_NetworkingSummaryLLM),		-1)\
	macro(Meshes,								"Meshes",						GET_STATFNAME(STAT_MeshesLLM),								GET_STATFNAME(STAT_MeshesSummaryLLM),			-1)\
	macro(Stats,								"Stats",						GET_STATFNAME(STAT_StatsLLM),								GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(Shaders,								"Shaders",						GET_STATFNAME(STAT_ShadersLLM),								GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(PSO,									"PSO",							GET_STATFNAME(STAT_PSOLLM),									GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(Textures,								"Textures",						GET_STATFNAME(STAT_TexturesLLM),							GET_STATFNAME(STAT_TexturesSummaryLLM),			-1)\
	macro(TextureMetaData,						"TextureMetaData",				GET_STATFNAME(STAT_TextureMetaDataLLM),						GET_STATFNAME(STAT_TexturesSummaryLLM),			-1)\
	macro(VirtualTextureSystem,					"VirtualTextureSystem",			GET_STATFNAME(STAT_VirtualTextureSystemLLM),				GET_STATFNAME(STAT_TexturesSummaryLLM),			-1)\
	macro(RenderTargets,						"RenderTargets",				GET_STATFNAME(STAT_RenderTargetsLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(SceneRender,							"SceneRender",					GET_STATFNAME(STAT_SceneRenderLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(RHIMisc,								"RHIMisc",						GET_STATFNAME(STAT_RHIMiscLLM),								GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(AsyncLoading,							"AsyncLoading",					GET_STATFNAME(STAT_AsyncLoadingLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
    /* UObject 是所有未被其他分类追踪的引擎和游戏内存的兜底分类。 */
	/* 它包括任何继承自 UObject 的类，以及该类序列化的任何东西（含属性）。 */
	/* 注意这个 stat 不含 Mesh 或 Animation 数据，它们单独追踪。 */
	/* 它与关卡里放置的对象数量相关。 */
	macro(UObject,								"UObject",						GET_STATFNAME(STAT_UObjectLLM),								GET_STATFNAME(STAT_UObjectSummaryLLM),			-1)\
	macro(Animation,							"Animation",					GET_STATFNAME(STAT_AnimationLLM),							GET_STATFNAME(STAT_AnimationSummaryLLM),		-1)\
	/* 这是 UStaticMesh 类及相关属性，不含实际的网格数据。 */
	macro(StaticMesh,							"StaticMesh",					GET_STATFNAME(STAT_StaticMeshLLM),							GET_STATFNAME(STAT_StaticMeshSummaryLLM),		ELLMTag::Meshes)\
	macro(Materials,							"Materials",					GET_STATFNAME(STAT_MaterialsLLM),							GET_STATFNAME(STAT_MaterialsSummaryLLM),		-1)\
	macro(Particles,							"Particles",					GET_STATFNAME(STAT_ParticlesLLM),							GET_STATFNAME(STAT_ParticlesSummaryLLM),		-1)\
	macro(Niagara,								"Niagara",						GET_STATFNAME(STAT_NiagaraLLM),								GET_STATFNAME(STAT_NiagaraSummaryLLM),			-1)\
	macro(GPUSort,								"GPUSort",						GET_STATFNAME(STAT_GPUSortLLM),								GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(GC,									"GC",							GET_STATFNAME(STAT_GCLLM),									GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(UI,									"UI",							GET_STATFNAME(STAT_UILLM),									GET_STATFNAME(STAT_UISummaryLLM),				-1)\
	macro(NavigationRecast,						"NavigationRecast",				GET_STATFNAME(STAT_NavigationRecastLLM),					GET_STATFNAME(STAT_NavigationSummaryLLM),		-1)\
	macro(Physics,								"Physics",						GET_STATFNAME(STAT_PhysicsLLM),								GET_STATFNAME(STAT_PhysicsSummaryLLM),			-1)\
	macro(PhysX,								"PhysX",						GET_STATFNAME(STAT_PhysXLLM),								GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(PhysXGeometry,						"PhysXGeometry",				GET_STATFNAME(STAT_PhysXGeometryLLM),						GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(PhysXTrimesh,							"PhysXTrimesh",					GET_STATFNAME(STAT_PhysXTrimeshLLM),						GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(PhysXConvex,							"PhysXConvex",					GET_STATFNAME(STAT_PhysXConvexLLM),							GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(PhysXAllocator,						"PhysXAllocator",				GET_STATFNAME(STAT_PhysXAllocatorLLM),						GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(PhysXLandscape,						"PhysXLandscape",				GET_STATFNAME(STAT_PhysXLandscapeLLM),						GET_STATFNAME(STAT_PhysXSummaryLLM),			ELLMTag::Physics)\
	macro(Chaos,								"Chaos",						GET_STATFNAME(STAT_ChaosLLM),								GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosGeometry,						"ChaosGeometry",				GET_STATFNAME(STAT_ChaosGeometryLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosAcceleration,					"ChaosAcceleration",			GET_STATFNAME(STAT_ChaosAccelerationLLM),					GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosParticles,						"ChaosParticles",				GET_STATFNAME(STAT_ChaosParticlesLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosLandscape,						"ChaosLandscape",				GET_STATFNAME(STAT_ChaosLandscapeLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosTrimesh,							"ChaosTrimesh",					GET_STATFNAME(STAT_ChaosTrimeshLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosConvex,							"ChaosConvex",					GET_STATFNAME(STAT_ChaosConvexLLM),							GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosScene,							"ChaosScene",					GET_STATFNAME(STAT_ChaosSceneLLM),							GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosUpdate,							"ChaosUpdate",					GET_STATFNAME(STAT_ChaosUpdateLLM),							GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosActor,							"ChaosActor",					GET_STATFNAME(STAT_ChaosActorLLM),							GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosBody,							"ChaosBody",					GET_STATFNAME(STAT_ChaosBodyLLM),							GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosConstraint,						"ChaosConstraint",				GET_STATFNAME(STAT_ChaosConstraintLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(ChaosMaterial,						"ChaosMaterial",				GET_STATFNAME(STAT_ChaosMaterialLLM),						GET_STATFNAME(STAT_ChaosSummaryLLM),			ELLMTag::Physics)\
	macro(EnginePreInitMemory,					"EnginePreInit",				GET_STATFNAME(STAT_EnginePreInitLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(EngineInitMemory,						"EngineInit",					GET_STATFNAME(STAT_EngineInitLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(RenderingThreadMemory,				"RenderingThread",				GET_STATFNAME(STAT_RenderingThreadLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(LoadMapMisc,							"LoadMapMisc",					GET_STATFNAME(STAT_LoadMapMiscLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(StreamingManager,						"StreamingManager",				GET_STATFNAME(STAT_StreamingManagerLLM),					GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(GraphicsPlatform,						"Graphics",						GET_STATFNAME(STAT_GraphicsPlatformLLM),					NAME_None,										-1)\
	macro(FileSystem,							"FileSystem",					GET_STATFNAME(STAT_FileSystemLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(Localization,							"Localization",					GET_STATFNAME(STAT_LocalizationLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(AssetRegistry,						"AssetRegistry",				GET_STATFNAME(STAT_AssetRegistryLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(ConfigSystem,							"ConfigSystem",					GET_STATFNAME(STAT_ConfigSystemLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(InitUObject,							"InitUObject",					GET_STATFNAME(STAT_InitUObjectLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(VideoRecording,						"VideoRecording",				GET_STATFNAME(STAT_VideoRecordingLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(Replays,								"Replays",						GET_STATFNAME(STAT_ReplaysLLM),								GET_STATFNAME(STAT_NetworkingSummaryLLM),		ELLMTag::Networking)\
	macro(MaterialInstance,						"MaterialInstance",				GET_STATFNAME(STAT_MaterialInstanceLLM),					GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(SkeletalMesh,							"SkeletalMesh",					GET_STATFNAME(STAT_SkeletalMeshLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			ELLMTag::Meshes)\
	macro(InstancedMesh,						"InstancedMesh",				GET_STATFNAME(STAT_InstancedMeshLLM),						GET_STATFNAME(STAT_EngineSummaryLLM),			ELLMTag::Meshes)\
	macro(Landscape,							"Landscape",					GET_STATFNAME(STAT_LandscapeLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			ELLMTag::Meshes)\
	macro(CsvProfiler,							"CsvProfiler",					GET_STATFNAME(STAT_CsvProfilerLLM),							GET_STATFNAME(STAT_EngineSummaryLLM),			-1)\
	macro(MediaStreaming,						"MediaStreaming",				GET_STATFNAME(STAT_MediaStreamingLLM),						GET_STATFNAME(STAT_MediaStreamingSummaryLLM),	-1)\
	macro(ElectraPlayer,						"ElectraPlayer",				GET_STATFNAME(STAT_ElectraPlayerLLM),						GET_STATFNAME(STAT_MediaStreamingSummaryLLM),	ELLMTag::MediaStreaming)\
	macro(WMFPlayer,							"WMFPlayer",					GET_STATFNAME(STAT_WMFPlayerLLM),							GET_STATFNAME(STAT_MediaStreamingSummaryLLM),	ELLMTag::MediaStreaming)\
	macro(PlatformMMIO,							"MMIO",							GET_STATFNAME(STAT_PlatformMMIOLLM),						NAME_None,										-1)\
	macro(PlatformVM,							"Virtual Memory",				GET_STATFNAME(STAT_PlatformVMLLM),							NAME_None,										-1)\
	macro(CustomName,							"CustomName",					GET_STATFNAME(STAT_CustomName),								NAME_None,										-1)\
```

**本段新元素**

- `LLM_ENUM_GENERIC_TAGS(macro)` —— X-Macro，每个标签一行
- `macro(名字, 显示名, StatName, SummaryStatName, 父标签)` —— 5 个参数
- `Untagged` / `Paused` / `Total` / `Untracked` —— 4 个基础标签
- `PlatformTotal` / `TrackedTotal` / `UntaggedTotal` / `WorkingSetSize` / `PagefileUsed` —— 5 个平台标签
- `FMalloc` / `FMallocUnused` / `RHIUnused` / `ThreadStack` —— 4 个分配器标签
- `Audio` + 12 个 Audio 子标签 —— 音频
- `Networking` / `Meshes` / `Stats` / `Shaders` / `PSO` —— 5 个引擎标签
- `Textures` + 3 个子标签 —— 纹理
- `UObject` / `Animation` / `StaticMesh` / `Materials` / `Particles` / `Niagara` —— 6 个游戏标签
- `Physics` + 5 个 PhysX 子标签 + 15 个 Chaos 子标签 —— 物理
- `EnginePreInitMemory` / `EngineInitMemory` / `RenderingThreadMemory` —— 3 个启动标签
- `MediaStreaming` + 2 个子标签 —— 媒体
- `CustomName` —— 自定义名标签
- **为什么用 X-Macro** —— 同一份列表展开成枚举值 / 名字表 / 父标签表，保证同步

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 163~177 行 | 基础 + 平台标签（14 个） |
| 第 178~189 行 | 分配器标签（12 个） |
| 第 190~208 行 | 引擎 + 音频标签（19 个） |
| 第 209~221 行 | 引擎子系统标签（13 个） |
| 第 222~235 行 | 游戏对象标签（14 个） |
| 第 236~256 行 | 物理标签（21 个） |
| 第 257~280 行 | 启动 + 媒体 + 自定义（24 个） |

> 类比：像一本分类账：每个部门一个科目，科目还能有子科目（Audio → AudioMixer → AudioMixerPlugins）。

### 五、`ELLMTag` 枚举 + 4 个常量（第 282~311 行）

```cpp
// 传给 LLM_SCOPE() 宏的枚举值
enum class ELLMTag : LLM_TAG_TYPE
{
// 用 LLM_ENUM_GENERIC_TAGS 展开成枚举值。
#define LLM_ENUM(Enum,Str,Stat,Group,Parent) Enum,
// 展开宏。
	LLM_ENUM_GENERIC_TAGS(LLM_ENUM)
// 取消宏定义。
#undef LLM_ENUM

// GenericTagCount：通用标签数量。
	GenericTagCount,

	//------------------------------
	// Platform tags
// PlatformTagStart：平台标签起始值。
	PlatformTagStart = 111,
// PlatformTagEnd：平台标签结束值。
	PlatformTagEnd = 149,

	//------------------------------
	// Project tags
// ProjectTagStart：项目标签起始值。
	ProjectTagStart = 150,
// ProjectTagEnd：项目标签结束值。
	ProjectTagEnd = 255,

	// 高于这个值的都当作 stat 段的 FName 处理
};
// 静态断言：通用标签数量不能超过平台标签起始值。
static_assert( ELLMTag::GenericTagCount <= ELLMTag::PlatformTagStart,
	"too many LLM tags defined -- Instead of adding a new tag and updating the limits, please use the LLM_DECLARE_TAG macros below"); 

// LLM_TAG_COUNT：标签总数（256）。
constexpr inline uint32 LLM_TAG_COUNT = 256;
// LLM_CUSTOM_TAG_START：自定义标签起始值。
constexpr inline uint32 LLM_CUSTOM_TAG_START = (int32)ELLMTag::PlatformTagStart;
// LLM_CUSTOM_TAG_END：自定义标签结束值。
constexpr inline uint32 LLM_CUSTOM_TAG_END = (int32)ELLMTag::ProjectTagEnd;
// LLM_CUSTOM_TAG_COUNT：自定义标签数量。
constexpr inline uint32 LLM_CUSTOM_TAG_COUNT = LLM_CUSTOM_TAG_END + 1 - LLM_CUSTOM_TAG_START;
```

**本段新元素**

- `enum class ELLMTag : LLM_TAG_TYPE` —— 标签枚举
- `#define LLM_ENUM(Enum,Str,Stat,Group,Parent) Enum,` —— 展开成枚举值
- `LLM_ENUM_GENERIC_TAGS(LLM_ENUM)` —— 展开 100+ 个值
- `GenericTagCount` —— 通用标签数量
- `PlatformTagStart = 111` / `PlatformTagEnd = 149` —— 平台标签范围
- `ProjectTagStart = 150` / `ProjectTagEnd = 255` —— 项目标签范围
- `static_assert(...)` —— 断言通用标签不超范围
- `LLM_TAG_COUNT = 256` —— 标签总数
- `LLM_CUSTOM_TAG_START` / `END` / `COUNT` —— 自定义标签范围

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 282~289 行 | 枚举声明 + X-Macro 展开 |
| 第 291~301 行 | 数量 + 平台/项目范围 |
| 第 305~306 行 | 静态断言 |
| 第 308~311 行 | 4 个常量 |

> 类比：像编号规则：0~110 是内置科目，111~149 留给平台，150~255 留给项目。

### 六、`ELLMAllocType` + 4 个基础宏（第 313~335 行）

```cpp
// 传给 OnLowLevelAlloc 指定分配类型。用于追踪 FMalloc 总量 以及暂停特定分配类型。
// ELLMAllocType：分配类型（传给 OnLowLevelAlloc）。
enum class ELLMAllocType
{
// None：无类型。
	None = 0,
// FMalloc：通过 GMalloc 的常规分配。
	FMalloc,
// System：系统分配。
	System,
// RHI：RHI 分配。
	RHI,
// Count：哨兵值。
	Count
// 枚举体结束。
};

extern const ANSICHAR* LLMGetTagNameANSI(ELLMTag Tag);
extern const TCHAR* LLMGetTagName(ELLMTag Tag);
extern CORE_API const FName LLMGetUntaggedTagName(ELLMTagSet TagSet);
/*
 * LLM utility macros
 */
// LLM(x)：直接展开 x。
#define LLM(x) x
// LLM_IF_ENABLED(x)：LLM 启用时才执行 x。
#define LLM_IF_ENABLED(x) if (FLowLevelMemTracker::IsEnabled()) { x; }
// LLM_IS_ENABLED()：LLM 是否启用。
#define LLM_IS_ENABLED() FLowLevelMemTracker::IsEnabled()
// SCOPE_NAME：用行号生成唯一变量名。
#define SCOPE_NAME PREPROCESSOR_JOIN(LLMScope,__LINE__)
```

**本段新元素**

- `enum class ELLMAllocType` —— 分配类型
- `None = 0` / `FMalloc` / `System` / `RHI` / `Count` —— 5 个值
- `LLM(x)` —— 直接展开 `x`
- `LLM_IF_ENABLED(x)` —— LLM 启用时才执行
- `LLM_IS_ENABLED()` —— LLM 是否启用
- `SCOPE_NAME` —— 用 `__LINE__` 生成唯一变量名

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 313~324 行 | `ELLMAllocType` |
| 第 332~335 行 | 4 个基础宏 |

> 类比：像给每笔账标来源：GMalloc 的、系统的、RHI 的。

### 七、作用域宏（LLM_SCOPE 系列）（第 341~388 行）

```cpp
// LLM 作用域宏
#define LLM_SCOPE(Tag)												FLLMScope SCOPE_NAME(Tag, false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Default);\
																	UE_MEMSCOPE(Tag) 
#define LLM_SCOPE_DYNAMIC(UniqueName, Tracker, TagSet, Constructor) \
	FLLMScopeDynamic SCOPE_NAME(Tracker, TagSet); \
	UE_MEMSCOPE_UNINITIALIZED(__LINE__); \
	do \
	{ \
		if (SCOPE_NAME.IsEnabled()) \
		{ \
			FName UniqueNameEvaluated(UniqueName); \
			if (!UniqueNameEvaluated.IsNone()) \
			{ \
				if (SCOPE_NAME.TryFindTag(UniqueNameEvaluated)) \
				{ \
					SCOPE_NAME.Activate(); \
				} \
				else \
				{ \
					SCOPE_NAME.TryAddTagAndActivate(UniqueNameEvaluated, Constructor); \
				} \
				constexpr int DefaultTracker = (int)ELLMTracker::Default; \
				if (TagSet == ELLMTagSet::None && (int)Tracker == DefaultTracker) \
				{ \
					UE_MEMSCOPE_ACTIVATE(__LINE__, UniqueNameEvaluated); \
				} \
			} \
		} \
	} while (false) /* do/while is added to require a semicolon */

// LLM_TAGSET_SCOPE：指定标签集打标签。
#define LLM_TAGSET_SCOPE(Tag, TagSet)								FLLMScope SCOPE_NAME(Tag, false /* bIsStaTag */, TagSet, ELLMTracker::Default);
#define LLM_SCOPE_BYNAME(Tag) 										static FName PREPROCESSOR_JOIN(LLMScope_Name,__LINE__)(Tag);\
																	FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMScope_Name,__LINE__), false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Default);\
																	UE_MEMSCOPE(PREPROCESSOR_JOIN(LLMScope_Name,__LINE__));
#define LLM_SCOPE_BYTAG(TagDeclName)								FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMTagDeclaration_, TagDeclName).GetUniqueName(), false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Default);\
																	UE_MEMSCOPE(PREPROCESSOR_JOIN(LLMTagDeclaration_,TagDeclName).GetUniqueName());
#define LLM_SCOPE_RENDER_RESOURCE(Tag)								static const FString PREPROCESSOR_JOIN(LLMScope_NamePrefix,__LINE__)(TEXT("RenderResources.")); \
																	FName PREPROCESSOR_JOIN(LLMScope_Name,__LINE__)(PREPROCESSOR_JOIN(LLMScope_NamePrefix, __LINE__) + (Tag ? Tag : TEXT("Unknown"))); \
																	FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMScope_Name,__LINE__), false /* bIsStatTag */, ELLMTagSet::Assets, ELLMTracker::Default, false /* bOverride */);
// LLM_PLATFORM_SCOPE：平台追踪器作用域。
#define LLM_PLATFORM_SCOPE(Tag)										FLLMScope SCOPE_NAME(Tag, false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Platform);
#define LLM_PLATFORM_SCOPE_BYNAME(Tag) 								static FName PREPROCESSOR_JOIN(LLMScope_Name,__LINE__)(Tag);\
																	FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMScope_Name,__LINE__), false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Platform);
// LLM_PLATFORM_SCOPE_BYTAG：平台追踪器 + 标签声明。
#define LLM_PLATFORM_SCOPE_BYTAG(TagDeclName)						FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMTagDeclaration_, TagDeclName).GetUniqueName(), false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Platform);
#define LLM_SCOPE_CLEAR()											FLLMClearScope SCOPE_NAME(ELLMTagSet::None, ELLMTracker::Default);\
																	UE_MEMSCOPE(0)
// LLM_TAGSET_SCOPE_CLEAR：清除指定标签集。
#define LLM_TAGSET_SCOPE_CLEAR(TagSet)								FLLMClearScope SCOPE_NAME(TagSet, ELLMTracker::Default);
```

**本段新元素**

- `LLM_SCOPE(Tag)` —— 打一个 LLM 作用域标签（默认追踪器）
- `LLM_SCOPE_DYNAMIC(UniqueName, Tracker, TagSet, Constructor)` —— 动态名字打标签（do/while 强制分号）
- `LLM_TAGSET_SCOPE(Tag, TagSet)` —— 指定标签集
- `LLM_SCOPE_BYNAME(Tag)` —— 用字符串名（静态缓存 `FName`）
- `LLM_SCOPE_BYTAG(TagDeclName)` —— 用标签声明
- `LLM_SCOPE_RENDER_RESOURCE(Tag)` —— 渲染资源专用（前缀 `RenderResources.`）
- `LLM_PLATFORM_SCOPE(Tag)` —— 平台追踪器
- `LLM_PLATFORM_SCOPE_BYNAME` / `BYTAG` —— 平台追踪器 + 名字/声明
- `LLM_SCOPE_CLEAR()` —— 清除当前标签
- `LLM_TAGSET_SCOPE_CLEAR(TagSet)` —— 清除指定标签集
- `UE_MEMSCOPE(Tag)` —— 同时打内存作用域

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 341~345 行 | `LLM_SCOPE` |
| 第 346~371 行 | `LLM_SCOPE_DYNAMIC`（do/while） |
| 第 373~378 行 | `TAGSET_SCOPE` / `BYNAME` / `BYTAG` |
| 第 379~381 行 | `SCOPE_RENDER_RESOURCE` |
| 第 382~385 行 | 3 个 `PLATFORM_SCOPE` |
| 第 386~388 行 | 2 个 `CLEAR` |

> 类比：像给每段代码贴标签：这段在加载纹理、那段在编译 Shader。

### 八、暂停宏 + realloc 宏 + 转储宏（第 390~408 行）

```cpp
// LLM 暂停作用域宏
// LLM_SCOPED_PAUSE_TRACKING：暂停追踪指定分配类型。
#define LLM_SCOPED_PAUSE_TRACKING(AllocType) 						FLLMPauseScope SCOPE_NAME(ELLMTag::Untagged, false /* bIsStatTag */, 0, ELLMTracker::Max, AllocType);
// LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER：暂停指定追踪器。
#define LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER(Tracker, AllocType) 	FLLMPauseScope SCOPE_NAME(ELLMTag::Untagged, false /* bIsStatTag */, 0, Tracker, AllocType);
// LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT：暂停并记录数量。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT(Tag, Amount, Tracker, AllocType) FLLMPauseScope SCOPE_NAME(Tag, false /* bIsStatTag */, Amount, Tracker, AllocType);
// LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT_BYTAG：同上，用标签声明。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT_BYTAG(TagDeclName, Amount, Tracker, AllocType) FLLMPauseScope SCOPE_NAME(PREPROCESSOR_JOIN(LLMTagDeclaration_, TagDeclName).GetUniqueName(), false /* bIsStatTag */, Amount, Tracker, AllocType);

// LLM realloc 作用域宏。重新分配指针且希望保留源指针标签时用
// LLM_REALLOC_SCOPE：realloc 时继承原指针的标签。
#define LLM_REALLOC_SCOPE(Ptr)										FLLMScopeFromPtr SCOPE_NAME(Ptr, ELLMTracker::Default);
// LLM_REALLOC_PLATFORM_SCOPE：平台追踪器版。
#define LLM_REALLOC_PLATFORM_SCOPE(Ptr)								FLLMScopeFromPtr SCOPE_NAME(Ptr, ELLMTracker::Platform);

// LLM 标签转储，帮助识别打错标签的项。大概不想带着这些提交代码！
// LLM_DUMP_TAG：把当前标签打到日志（调试用）。
#define LLM_DUMP_TAG()  FLowLevelMemTracker::Get().DumpTag(ELLMTracker::Default,__FILE__,__LINE__)
// LLM_DUMP_PLATFORM_TAG：平台追踪器版。
#define LLM_DUMP_PLATFORM_TAG()  FLowLevelMemTracker::Get().DumpTag(ELLMTracker::Platform,__FILE__,__LINE__)
```

**本段新元素**

- `LLM_SCOPED_PAUSE_TRACKING(AllocType)` —— 暂停追踪指定分配类型
- `LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER(Tracker, AllocType)` —— 暂停指定追踪器
- `LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT(Tag, Amount, Tracker, AllocType)` —— 暂停并记录数量
- `LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT_BYTAG(...)` —— 同上，用标签声明
- `LLM_REALLOC_SCOPE(Ptr)` —— realloc 时继承原指针标签
- `LLM_REALLOC_PLATFORM_SCOPE(Ptr)` —— 平台追踪器版
- `LLM_DUMP_TAG()` —— 把当前标签打到日志（调试用）
- `LLM_DUMP_PLATFORM_TAG()` —— 平台追踪器版

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 390~396 行 | 4 个暂停宏 |
| 第 398~402 行 | 2 个 realloc 宏 |
| 第 404~408 行 | 2 个转储宏 |

> 类比：像暂停录音：这段别记（比如 LLM 自己分配内存时）。

### 九、标签定义宏（DEFINE / DECLARE / BOOTSTRAP）（第 410~468 行）

```cpp
// 定义一个标签，可用于 LLM_SCOPE_BYTAG 或在其他 LLM_SCOPE 里按名字引用。 @param UniqueNameWithUnderscores - 标签名的下划线版本。用于按名字查找， 必须在所有传给 LLM_DEFINE_TAG、LLM_SCOPE 或 ELLMTag 的标签里唯一。 修改方式：父标签的常规分隔符 / 在 LLM_DEFINE_TAG 里必须换成 _。 @param DisplayName - （可选）追踪标签时显示的名字；有父标签时用 / 与父名拼接， 或 NAME_None 表示用 UniqueName。 @param ParentTagName - （可选）父标签的唯一名，或 NAME_None 表示无父。 @param StatName - （可选）每帧发布 LLM 数据时用该标签的量填充的 stat 名， 或 NAME_None 表示不填充 stat。 @param SummaryStatName - （可选）每帧发布 LLM 数据时把该标签的量加到的 stat 组名， 或 NAME_None 表示不加到任何 stat 组。
// LLM_DEFINE_TAG：定义一个标签（可用于 LLM_SCOPE_BYTAG）。
#define LLM_DEFINE_TAG(UniqueNameWithUnderscores, ...) FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)(TEXT(#UniqueNameWithUnderscores), ##__VA_ARGS__)

// 静态定义一个标签，可用于 LLM_SCOPE_BYTAG 或在其他 LLM_SCOPE 里按名字引用。 @param UniqueNameWithUnderscores - 标签名的下划线版本。用于按名字查找， 必须在所有传给 LLM_DEFINE_TAG、LLM_SCOPE 或 ELLMTag 的标签里唯一。 修改方式：父标签的常规分隔符 / 在 LLM_DEFINE_TAG 里必须换成 _。 @param DisplayName - （可选）追踪标签时显示的名字；有父标签时用 / 与父名拼接， 或 NAME_None 表示用 UniqueName。 @param ParentTagName - （可选）父标签的唯一名，或 NAME_None 表示无父。 @param StatName - （可选）每帧发布 LLM 数据时用该标签的量填充的 stat 名， 或 NAME_None 表示不填充 stat。 @param SummaryStatName - （可选）每帧发布 LLM 数据时把该标签的量加到的 stat 组名， 或 NAME_None 表示不加到任何 stat 组。
// LLM_DEFINE_STATIC_TAG：定义静态标签。
#define LLM_DEFINE_STATIC_TAG(UniqueNameWithUnderscores, ...) static FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)(TEXT(#UniqueNameWithUnderscores), ##__VA_ARGS__)

// 声明一个由 LLM_DEFINE_TAG 定义的标签。用于 LLM_SCOPE_BYTAG 或在其他 LLM_SCOPE 里按名字引用。 @param UniqueName - 用于按名字查找的标签名，必须在所有传给 LLM_DEFINE_TAG、LLM_SCOPE 或 ELLMTag 的标签里唯一。 @param ModuleName - （可选，只在 LLM_DECLARE_TAG_API 里）用于声明模块链接的 MODULENAME_API 符号， 即 ENGINE_API。省略则不用模块链接，从其他模块使用会产生链接错误。
// LLM_DECLARE_TAG：声明一个已定义的标签。
#define LLM_DECLARE_TAG(UniqueNameWithUnderscores) extern FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)
// LLM_DECLARE_TAG_API：带模块导出标记的声明。
#define LLM_DECLARE_TAG_API(UniqueNameWithUnderscores, ModuleAPI) extern ModuleAPI FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)

/** 取标签的唯一名。可传给 OnLowLevelAlloc 这类接收标签唯一名的函数。 */
// LLM_TAG_NAME：取标签的唯一名。
#define LLM_TAG_NAME(UniqueNameWithUnderscores) (PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores).GetUniqueName())

// LLM_DEFINE_TAG、LLM_SCOPE_BYTAG 和 LLM_DECLARE_TAG 的 BootStrap 版本支持在全局 C++ 构造函数里、main 之前的作用域中使用。这些标签在所有非 Shipping 配置里都稍贵一些 （即使 LLM 禁用），因为它们用函数静态变量而不是全局变量。
#define LLM_DEFINE_BOOTSTRAP_TAG(UniqueNameWithUnderscores, ...) \
	FLLMTagDeclaration& PREPROCESSOR_JOIN(GetLLMTagDeclaration_, UniqueNameWithUnderscores)() \
	{ \
		static FLLMTagDeclaration PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores)(TEXT(#UniqueNameWithUnderscores), ##__VA_ARGS__); \
		return PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueNameWithUnderscores); \
	}
#define LLM_SCOPE_BY_BOOTSTRAP_TAG(TagDeclName) FLLMScope SCOPE_NAME(PREPROCESSOR_JOIN(GetLLMTagDeclaration_, TagDeclName)().GetUniqueName(), false /* bIsStatTag */, ELLMTagSet::None, ELLMTracker::Default);\
												UE_MEMSCOPE(PREPROCESSOR_JOIN(GetLLMTagDeclaration_,TagDeclName)().GetUniqueName());
// LLM_DECLARE_BOOTSTRAP_TAG：声明引导期标签。
#define LLM_DECLARE_BOOTSTRAP_TAG(UniqueNameWithUnderscores) extern FLLMTagDeclaration& PREPROCESSOR_JOIN(GetLLMTagDeclaration_, UniqueNameWithUnderscores)();
// LLM_DECLARE_BOOTSTRAP_TAG_API：带导出标记的声明。
#define LLM_DECLARE_BOOTSTRAP_TAG_API(UniqueNameWithUnderscores, ModuleAPI) extern ModuleAPI FLLMTagDeclaration& PREPROCESSOR_JOIN(GetLLMTagDeclaration_, UniqueNameWithUnderscores)();
```

**本段新元素**

- `LLM_DEFINE_TAG(UniqueNameWithUnderscores, ...)` —— 定义一个标签（全局实例）
- `LLM_DEFINE_STATIC_TAG(...)` —— 定义静态标签
- `LLM_DECLARE_TAG(UniqueNameWithUnderscores)` —— 声明已定义的标签
- `LLM_DECLARE_TAG_API(..., ModuleAPI)` —— 带模块导出标记
- `LLM_TAG_NAME(UniqueNameWithUnderscores)` —— 取标签唯一名
- `LLM_DEFINE_BOOTSTRAP_TAG(...)` —— 定义引导期标签（全局构造期可用）
- `LLM_SCOPE_BY_BOOTSTRAP_TAG(TagDeclName)` —— 用引导期标签打作用域
- `LLM_DECLARE_BOOTSTRAP_TAG(...)` / `_API` —— 声明引导期标签
- **为什么有 BOOTSTRAP 版** —— 全局 C++ 构造函数里、main 之前也能用（用函数静态变量延迟初始化）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 410~423 行 | `LLM_DEFINE_TAG` |
| 第 425~438 行 | `LLM_DEFINE_STATIC_TAG` |
| 第 440~452 行 | `DECLARE_TAG` / `TAG_NAME` |
| 第 454~468 行 | 5 个 BOOTSTRAP 宏 |

> 类比：像给新科目开户：DEFINE 是开户，DECLARE 是告诉别人「这个户存在」，BOOTSTRAP 是「开业前就能用的临时户」。

### 十、`UE::LLMPrivate` 命名空间（FLLMAllocator 等）（第 470~587 行）

```cpp
// LLMAllocFunction：分配函数指针类型。
typedef void*(*LLMAllocFunction)(size_t);
// LLMFreeFunction：释放函数指针类型。
typedef void(*LLMFreeFunction)(void*, size_t);

// 前置声明 FLLMTagDeclaration。
class FLLMTagDeclaration;

// 前置声明 FLLMTagSetAllocationFilter。
struct FLLMTagSetAllocationFilter;

// 命名空间 UE::LLMPrivate。
namespace UE::LLMPrivate
{

// 前置声明 FLLMCsvProfilerWriter。
class FLLMCsvProfilerWriter;
// 前置声明 FLLMCsvWriter。
class FLLMCsvWriter;
// 前置声明 FLLMThreadState。
class FLLMThreadState;
// 前置声明 FLLMTraceWriter。
class FLLMTraceWriter;
// 前置声明 FLLMTracker。
class FLLMTracker;
// 前置声明 FTagData。
class FTagData;
// 前置声明 FTagDataArray。
class FTagDataArray;
// 前置声明 FTagDataNameMap。
class FTagDataNameMap;

// 命名空间 AllocatorPrivate：分配器内部实现。
namespace AllocatorPrivate
{
// 前置声明 FBin。
	struct FBin;
// 前置声明 FPage。
	struct FPage;
// 命名空间结束。
}
// LLM 用来分配内部内存的分配器。用平台定义的分配函数直接从 OS 拿内存。
// FLLMAllocator：LLM 内部内存分配器（直接从 OS 拿内存）。
class FLLMAllocator
{
// 公有区。
public:
// 构造函数。
	FLLMAllocator();
// 析构函数。
	~FLLMAllocator();

// Get：取单例引用。
	static FLLMAllocator*& Get();

// Initialise：初始化（传入平台分配/释放函数 + 页大小）。
	void Initialise(LLMAllocFunction InAlloc, LLMFreeFunction InFree, int32 InPageSize);
// Clear：清空。
	void Clear();
// Alloc：分配（不构造）。
	void* Alloc(size_t Size);
// Malloc：分配（同 Alloc）。
	void* Malloc(size_t Size);
// Free：释放。
	void Free(void* Ptr, size_t Size);
// Realloc：重新分配。
	void* Realloc(void* Ptr, size_t OldSize, size_t NewSize);
// GetTotal：取总分配量。
	int64 GetTotal() const;

// New：分配并构造对象。
	template <typename T, typename... ArgsType>
// 函数体开始。
	T* New(ArgsType&&... Args)
	{
// 分配内存。
		T* Ptr = reinterpret_cast<T*>(Alloc(sizeof(T)));
// placement new 构造。
		new (Ptr) T(Forward<ArgsType>(Args)...);
// 返回指针。
		return Ptr;
// 函数体结束。
	}

// Delete：析构并释放对象。
	template <typename T>
// 函数体开始。
	void Delete(T* Ptr)
// 如果指针非空。
	{
// 条件体开始。
		if (Ptr)
		{
// 显式调析构。
			Ptr->~T();
// 释放内存。
			Free(Ptr, sizeof(T));
// 条件体结束。
		}
// 函数体结束。
	}

// GetPlatformAlloc：取平台分配函数。
	LLMAllocFunction GetPlatformAlloc()
// 函数体开始。
	{
// 返回。
		return PlatformAlloc;
// 函数体结束。
	}
// GetPlatformFree：取平台释放函数。
	LLMFreeFunction GetPlatformFree()
// 函数体开始。
	{
// 返回。
		return PlatformFree;
// 函数体结束。
	}

// 私有区。
private:
// AllocPages：分配页。
	void* AllocPages(size_t Size);
// FreePages：释放页。
	void FreePages(void* Ptr, size_t Size);
// GetBinIndex：按大小算桶索引。
	int32 GetBinIndex(size_t Size) const;

// CriticalSection：保护分配器的锁。
	FCriticalSection CriticalSection;
// PlatformAlloc：平台分配函数。
	LLMAllocFunction PlatformAlloc;
// PlatformFree：平台释放函数。
	LLMFreeFunction PlatformFree;
// Bins：桶数组。
	AllocatorPrivate::FBin* Bins;
// Total：总分配量。
	int64 Total;
// PageSize：页大小。
	int32 PageSize;
// NumBins：桶数量。
	int32 NumBins;

// 友元：FPage 可访问私有成员。
	friend struct UE::LLMPrivate::AllocatorPrivate::FPage;
// 友元：FBin 可访问私有成员。
	friend struct UE::LLMPrivate::AllocatorPrivate::FBin;
// 类结束。
};

// ETagReferenceSource：标签引用来源。
enum class ETagReferenceSource
{
// Scope：来自作用域。
	Scope,
// Declare：来自声明。
	Declare,
// EnumTag：来自枚举标签。
	EnumTag,
// CustomEnumTag：来自自定义枚举标签。
	CustomEnumTag,
// FunctionAPI：来自函数 API。
	FunctionAPI,
// ImplicitParent：来自隐式父标签。
	ImplicitParent
// 枚举体结束。
};

// FLLMInitialisedCallback：初始化完成回调类型。
typedef void (*FLLMInitialisedCallback)(UPTRINT UserData);
// FTagCreationCallback：标签创建回调类型。
typedef void (*FTagCreationCallback)(const UE::LLMPrivate::FTagData* TagData, UPTRINT UserData);

// LLM 初始化期间可能发生的回调。这些发生在 main 之前，因此使用有风险，所以是私有的。
// FPrivateCallbacks：LLM 初始化期间的回调（私有，因为发生在 main 之前）。
struct FPrivateCallbacks
{
// 私有区。
private:
// AddInitialisedCallback：注册初始化完成回调。
	CORE_API static void AddInitialisedCallback(FLLMInitialisedCallback Callback, UPTRINT UserData);
	// 没有 RemoveInitialiseCallback，因为它在 main 之前就触发并清空了，
	// 在那之前应该不需要移除回调。
// AddTagCreationCallback：注册标签创建回调。
	CORE_API static void AddTagCreationCallback(FTagCreationCallback Callback, UPTRINT UserData);
// RemoveTagCreationCallback：移除标签创建回调。
	CORE_API static void RemoveTagCreationCallback(FTagCreationCallback Callback);

// 友元：FTagTrace 可访问私有成员。
	friend class ::FTagTrace;
// 结构体结束。
};

// 命名空间结束。
} // UE::LLMPrivate
```

**本段新元素**

- `LLMAllocFunction` / `LLMFreeFunction` —— 分配/释放函数指针类型
- 8 个前置声明 —— `FLLMCsvProfilerWriter` / `FLLMCsvWriter` / `FLLMThreadState` / `FLLMTraceWriter` / `FLLMTracker` / `FTagData` / `FTagDataArray` / `FTagDataNameMap`
- `class FLLMAllocator` —— LLM 内部内存分配器（直接从 OS 拿内存）
- `Initialise(Alloc, Free, PageSize)` —— 初始化
- `Alloc` / `Malloc` / `Free` / `Realloc` / `GetTotal` —— 5 个内存操作
- `template <typename T, typename... ArgsType> T* New(ArgsType&&... Args)` —— 分配并构造
- `template <typename T> void Delete(T* Ptr)` —— 析构并释放
- `FCriticalSection CriticalSection` —— 保护分配器的锁
- `AllocatorPrivate::FBin* Bins` —— 桶数组
- `enum class ETagReferenceSource` —— 标签引用来源（6 个值）
- `FLLMInitialisedCallback` / `FTagCreationCallback` —— 2 个回调类型
- `struct FPrivateCallbacks` —— 初始化期间的回调（私有）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 470~475 行 | 2 个函数指针类型 + 2 个前置声明 |
| 第 477~493 行 | 命名空间 + 8 个前置声明 |
| 第 498~512 行 | `FLLMAllocator` 声明 + 7 个方法 |
| 第 514~530 行 | `New` / `Delete` 模板 |
| 第 532~556 行 | 访问器 + 私有成员 |
| 第 558~566 行 | `ETagReferenceSource` |
| 第 568~585 行 | 2 个回调类型 + `FPrivateCallbacks` |

> 类比：像 LLM 自己的小金库：不通过 GMalloc，直接从 OS 拿内存，避免「追踪自己」的死循环。

### 十一、`FLLMCustomTag`（废弃）+ `FLLMTagInfo`（第 589~604 行）

```cpp
struct UE_DEPRECATED(4.27, "FLLMCustomTag was an implementation detail that has been modified, switch to FLLMTagInfo or to your own local struct") FLLMCustomTag
{
// Tag：标签值。
	int32 Tag;
// Name：标签名。
	const TCHAR* Name;
// StatName：统计名。
	FName StatName;
// SummaryStatName：汇总统计名。
	FName SummaryStatName;
// 结构体结束。
};

/** 一个方便的结构体，收集 RegisterProjectTag 需要的字段 */
// FLLMTagInfo：注册项目标签时用的字段集合。
struct FLLMTagInfo
{
// Name：标签名。
	const TCHAR* Name;
// StatName：显示在 LLMFULL 统计组。
	FName StatName;				// shows in the LLMFULL stat group
// SummaryStatName：显示在 LLM 汇总统计组。
	FName SummaryStatName;		// shows in the LLM summary stat group
// ParentTag：父标签（-1 表示无）。
	int32 ParentTag = -1;
// 结构体结束。
};
```

**本段新元素**

- `struct FLLMCustomTag` —— **已废弃（4.27）**，改用 `FLLMTagInfo`
- `int32 Tag` / `const TCHAR* Name` / `FName StatName` / `FName SummaryStatName` —— 4 个字段
- `struct FLLMTagInfo` —— 注册项目标签时用的字段集合
- `Name` / `StatName` / `SummaryStatName` / `ParentTag = -1` —— 4 个字段

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 589~595 行 | `FLLMCustomTag`（废弃） |
| 第 597~604 行 | `FLLMTagInfo` |

> 类比：像老式表格：还能用，但已经贴了「请换新表」的标签。

### 十二、`FLowLevelMemTracker` 单例 + 核心 API（第 606~700 行）

```cpp
/** LLM 主追踪器类。 */
// FLowLevelMemTracker：LLM 主追踪器类。
class FLowLevelMemTracker
{
// 公有区。
public:

	/** 取单例，保证永远有有效对象 */
// Get：取单例。
	inline static FLowLevelMemTracker& Get()
// 函数体开始。
	{
// 如果实例已存在。
		if (TrackerInstance)
// 返回实例。
			return *TrackerInstance;
// 否则。
		else
// 构造。
			return Construct();
// 函数体结束。
	}

// Construct：构造单例。
	static CORE_API FLowLevelMemTracker& Construct();

// 返回 LLM 是否启用 —— 由命令行激活并正在追踪分配。 启动早期、命令行还没处理时也返回 true。
// IsEnabled：LLM 是否启用（启动早期也返回 true）。
	inline static bool IsEnabled();

// 我们总是启动时就在运行，但如果命令行禁用了我们，会在 main 之后禁用 （跨平台方式下没法更早拿到命令行）。
// ProcessCommandLine：处理命令行（可能禁用 LLM）。
	CORE_API void ProcessCommandLine(const TCHAR* CmdLine);

	/** 返回正在追踪的内存总量。 */
// GetTotalTrackedMemory：取总追踪内存。
	CORE_API uint64 GetTotalTrackedMemory(ELLMTracker Tracker);

// 记录为某个指针分配的内存使用。 @param Tracker 用哪个追踪器：常规引擎分配用高层默认追踪器，内存管理系统用低层平台追踪器。 @param Ptr 被分配的指针。 @param Size 为该指针分配的内存大小。 @param DefaultTag 调用栈上没有标签时用的标签。 @param AllocType 分配类型：通过 GMalloc 的常规分配用 FMalloc，其他用 System。 @param bTrackInMemPro 是否把分配/释放信息传给 FMemProProfiler。
// OnLowLevelAlloc：记录一次分配（枚举标签版）。
	CORE_API void OnLowLevelAlloc(ELLMTracker Tracker, const void* Ptr, uint64 Size, ELLMTag DefaultTag = ELLMTag::Untagged,
// 续行：参数默认值。
		ELLMAllocType AllocType = ELLMAllocType::None, bool bTrackInMemPro = true);
// OnLowLevelAlloc：记录一次分配（FName 标签版）。
	CORE_API void OnLowLevelAlloc(ELLMTracker Tracker, const void* Ptr, uint64 Size, FName DefaultTag,
// 续行：参数默认值。
		ELLMAllocType AllocType = ELLMAllocType::None, bool bTrackInMemPro = true);

// 记录为某个指针分配的内存的释放。 @param Tracker 用哪个追踪器：常规引擎分配用高层默认追踪器，内存管理系统用低层平台追踪器。 @param Ptr 被释放的指针。 @param AllocType 分配类型：通过 GMalloc 的常规分配用 FMalloc，其他用 System。 @param bTrackInMemPro 是否把分配/释放信息传给 FMemProProfiler。
// OnLowLevelFree：记录一次释放。
	CORE_API void OnLowLevelFree(ELLMTracker Tracker, const void* Ptr,
// 续行：参数默认值。
		ELLMAllocType AllocType = ELLMAllocType::None, bool bTrackInMemPro = true);

// 记录不直接关联单个指针的系统分配或释放。 @param Tracker 用哪个追踪器：常规引擎分配用高层默认追踪器，内存管理系统用低层平台追踪器。 @param DeltaMemory 分配（正）或释放（负）的内存量 @param DefaultTag 调用栈上没有标签时用的标签。 @param AllocType 分配类型：通过 GMalloc 的常规分配用 FMalloc，其他用 System。
// OnLowLevelChangeInMemoryUse：记录内存增减（枚举标签版）。
	CORE_API void OnLowLevelChangeInMemoryUse(ELLMTracker Tracker, int64 DeltaMemory, ELLMTag DefaultTag = ELLMTag::Untagged, ELLMAllocType AllocType = ELLMAllocType::None);
// OnLowLevelChangeInMemoryUse：记录内存增减（FName 标签版）。
	CORE_API void OnLowLevelChangeInMemoryUse(ELLMTracker Tracker, int64 DeltaMemory, FName DefaultTag, ELLMAllocType AllocType = ELLMAllocType::None);

	/** 分配在内存里被移动时调用，如碎片整理器。 */
// OnLowLevelAllocMoved：记录分配被移动（如碎片整理）。
	CORE_API void OnLowLevelAllocMoved(ELLMTracker Tracker, const void* Dest, const void* Source,
// 续行：参数默认值。
		ELLMAllocType AllocType = ELLMAllocType::None);

	/** 更新内存统计，可选发布给观察者。 */
// UpdateStatsPerFrame：每帧更新统计并发布。
	CORE_API void UpdateStatsPerFrame(const TCHAR* LogName=nullptr);
	/** 更新内存统计。 */
// Tick：更新统计。
	CORE_API void Tick();

	/** 可选设置游戏启动前可执行文件和数据段占用的内存量。 */
// SetProgramSize：设置程序本身占用的大小。
	CORE_API void SetProgramSize(uint64 InProgramSize);

	/** 控制台命令处理 */
// Exec：控制台命令处理。
	CORE_API bool Exec(const TCHAR* Cmd, FOutputDevice& Ar);

	/** 报告指定标签集是否激活，如资产追踪。 */
// IsTagSetActive：指定标签集是否激活。
	CORE_API bool IsTagSetActive(ELLMTagSet Set);

	/** 对某些标签集，减少线程非常有用，比如把分配归因到资产。 */
// ShouldReduceThreads：是否应减少线程（便于归因到资产）。
	CORE_API bool ShouldReduceThreads();

	/** 取指定追踪器当前顶层激活标签的不透明标识。 */
// GetActiveTagData：取当前激活标签的不透明标识。
	CORE_API const UE::LLMPrivate::FTagData* GetActiveTagData(ELLMTracker Tracker, ELLMTagSet TagSet = ELLMTagSet::None);

```

**本段新元素**

- `class FLowLevelMemTracker` —— LLM 主追踪器
- `static Get()` —— 取单例（懒构造）
- `static Construct()` —— 构造单例
- `static IsEnabled()` —— LLM 是否启用（启动早期也返回 true）
- `ProcessCommandLine(CmdLine)` —— 处理命令行（可能禁用 LLM）
- `GetTotalTrackedMemory(Tracker)` —— 取总追踪内存
- `OnLowLevelAlloc(Tracker, Ptr, Size, DefaultTag, AllocType, bTrackInMemPro)` —— 记录分配（2 个重载）
- `OnLowLevelFree(Tracker, Ptr, AllocType, bTrackInMemPro)` —— 记录释放
- `OnLowLevelChangeInMemoryUse(Tracker, DeltaMemory, DefaultTag, AllocType)` —— 记录内存增减（2 个重载）
- `OnLowLevelAllocMoved(Tracker, Dest, Source, AllocType)` —— 记录分配被移动
- `UpdateStatsPerFrame(LogName)` —— 每帧更新统计并发布
- `Tick()` —— 更新统计
- `SetProgramSize(InProgramSize)` —— 设置程序本身大小
- `Exec(Cmd, Ar)` —— 控制台命令处理
- `IsTagSetActive(Set)` —— 标签集是否激活
- `ShouldReduceThreads()` —— 是否应减少线程
- `GetActiveTagData(Tracker, TagSet)` —— 取当前激活标签

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 606~620 行 | 类声明 + `Get` / `Construct` |
| 第 622~635 行 | `IsEnabled` / `ProcessCommandLine` / `GetTotalTrackedMemory` |
| 第 637~651 行 | `OnLowLevelAlloc` ×2 |
| 第 653~663 行 | `OnLowLevelFree` |
| 第 665~675 行 | `OnLowLevelChangeInMemoryUse` ×2 |
| 第 677~690 行 | `OnLowLevelAllocMoved` / `UpdateStatsPerFrame` / `Tick` / `SetProgramSize` / `Exec` |
| 第 692~699 行 | `IsTagSetActive` / `ShouldReduceThreads` / `GetActiveTagData` |

> 类比：像总账房：所有分配/释放都要来这里登记，它负责汇总。

### 十三、`FLowLevelMemTracker` 标签查询 API（第 701~765 行）

```cpp
	/** 注册自定义 ELLMTag。 */
// RegisterPlatformTag：注册平台标签。
	CORE_API void RegisterPlatformTag(int32 Tag, const TCHAR* Name, FName StatName, FName SummaryStatName, int32 ParentTag = -1);
// RegisterProjectTag：注册项目标签。
	CORE_API void RegisterProjectTag(int32 Tag, const TCHAR* Name, FName StatName, FName SummaryStatName, int32 ParentTag = -1);
    
	/** 取所有被追踪的标签。 */
// GetTrackedTags：取所有被追踪的标签。
	CORE_API TArray<const UE::LLMPrivate::FTagData*> GetTrackedTags(ELLMTagSet TagSet = ELLMTagSet::None);

	/** 取指定追踪器追踪的所有标签。 */
// GetTrackedTags：取指定追踪器的所有标签。
	CORE_API TArray<const UE::LLMPrivate::FTagData*> GetTrackedTags(ELLMTracker Tracker, ELLMTagSet TagSet = ELLMTagSet::None);

// GetTrackedTagsNamesWithAmount：取标签名 + 数量。
	CORE_API void GetTrackedTagsNamesWithAmount(TMap<FName, uint64>& TagsNamesWithAmount, ELLMTracker Tracker, ELLMTagSet TagSet);
// GetTrackedTagsNamesWithAmountFiltered：带过滤版。
	CORE_API void GetTrackedTagsNamesWithAmountFiltered(TMap<FName, uint64>& TagsNamesWithAmount, ELLMTracker Tracker, ELLMTagSet TagSet, TArray<FLLMTagSetAllocationFilter>& Filters);

	/** 按显示名查对应的 ELLMTag。 */
// FindTagByName：按显示名查标签。
	CORE_API bool FindTagByName(const TCHAR* Name, uint64& OutTag, ELLMTagSet InTagSet = ELLMTagSet::None) const;

	UE_DEPRECATED(4.27, "Use FindTagDisplayName instead")
// FindTagName：已废弃（4.27），改用 FindTagDisplayName。
	CORE_API const TCHAR* FindTagName(uint64 Tag) const;

	/** 取指定 ELLMTag 的显示名。 */
// FindTagDisplayName：取标签显示名。
	CORE_API FName FindTagDisplayName(uint64 Tag) const;
// FindPtrDisplayName：取指针的显示名。
	CORE_API FName FindPtrDisplayName(void* Ptr) const;

	/** 取指定 FTagData 的显示名。 */
// GetTagDisplayName：取 FTagData 的显示名。
	CORE_API FName GetTagDisplayName(const UE::LLMPrivate::FTagData* TagData) const;

	/** 从父标签显示名链取指定 FTagData 的路径名。 */
// GetTagDisplayPathName：取完整路径名（含父标签）。
	CORE_API FString GetTagDisplayPathName(const UE::LLMPrivate::FTagData* TagData) const;
// GetTagDisplayPathName：写入 StringBuilder 版。
	CORE_API void GetTagDisplayPathName(const UE::LLMPrivate::FTagData* TagData,
// 续行：参数默认值。
		FStringBuilderBase& OutPathName, int32 MaxLen=-1) const;

	/** 取指定 FTagData 的唯一标识名。 */
// GetTagUniqueName：取唯一名。
	CORE_API FName GetTagUniqueName(const UE::LLMPrivate::FTagData* TagData) const;

	/** 返回给定 TagData 的父作用域 TagData，无父则返回 nullptr。 */
// GetTagParent：取父标签。
	CORE_API const UE::LLMPrivate::FTagData* GetTagParent(const UE::LLMPrivate::FTagData* TagData) const;

	/** 当且仅当 TagData 是 ELLMTag 或自定义声明的平台/项目 ELLMTag 时返回 true。 */
// GetTagIsEnumTag：是否是枚举标签。
	CORE_API bool GetTagIsEnumTag(const UE::LLMPrivate::FTagData* TagData) const;

// 返回最近的父标签（可能包括 TagData 自己）中 GetTagIsEnumTag 为 true 的 ELLMTag。 对没有 ELLMTag 父的 FName 标签，返回 ELLMTag::CustomName。
// GetTagClosestEnumTag：取最近的枚举标签祖先。
	CORE_API ELLMTag GetTagClosestEnumTag(const UE::LLMPrivate::FTagData* TagData) const;

	/** 取指定追踪器下某个 ELLMTag 的内存量。 */
// GetTagAmountForTracker：取标签内存量（枚举版）。
	CORE_API int64 GetTagAmountForTracker(ELLMTracker Tracker, ELLMTag Tag, UE::LLM::ESizeParams SizeParams = UE::LLM::ESizeParams::Default);

	/** 取指定追踪器下某个 FTagData 的内存量。 */
// GetTagAmountForTracker：取标签内存量（FTagData 版）。
	CORE_API int64 GetTagAmountForTracker(ELLMTracker Tracker, const UE::LLMPrivate::FTagData* TagData, UE::LLM::ESizeParams SizeParams = UE::LLM::ESizeParams::Default);

// GetTagAmountForTracker：取标签内存量（FName 版）。
    CORE_API int64 GetTagAmountForTracker(ELLMTracker Tracker, FName Tag, ELLMTagSet TagSet, UE::LLM::ESizeParams SizeParams = UE::LLM::ESizeParams::Default);

	/** 设置指定追踪器下某个 ELLMTag 的内存量，可选更新总追踪内存。 */
// SetTagAmountForTracker：设置标签内存量（枚举版）。
	CORE_API void SetTagAmountForTracker(ELLMTracker Tracker, ELLMTag Tag, int64 Amount, bool bAddToTotal);

	/** 设置指定追踪器下某个标签名的内存量，可选更新总追踪内存。 */
// SetTagAmountForTracker：设置标签内存量（FName 版）。
	CORE_API void SetTagAmountForTracker(ELLMTracker Tracker, FName Tag, ELLMTagSet TagSet, int64 Amount, bool bAddToTotal);
	
	/** 把指定追踪器当前 TagData 的显示名转储到输出。 */
// DumpTag：把当前标签打到输出。
	CORE_API uint64 DumpTag(ELLMTracker Tracker, const char* FileName, int LineNumber);

	/** 在当前帧发布激活的 LLM 统计，用于单次定向 LLM 快照。 */
// PublishDataSingleFrame：发布单帧数据。
	CORE_API void PublishDataSingleFrame();
```

**本段新元素**

- `RegisterPlatformTag(Tag, Name, StatName, SummaryStatName, ParentTag)` —— 注册平台标签
- `RegisterProjectTag(...)` —— 注册项目标签
- `GetTrackedTags(TagSet)` / `GetTrackedTags(Tracker, TagSet)` —— 取所有被追踪的标签（2 个重载）
- `GetTrackedTagsNamesWithAmount(...)` —— 取标签名 + 数量
- `GetTrackedTagsNamesWithAmountFiltered(...)` —— 带过滤版
- `FindTagByName(Name, OutTag, InTagSet)` —— 按显示名查标签
- `FindTagName(Tag)` —— **已废弃（4.27）**
- `FindTagDisplayName(Tag)` / `FindPtrDisplayName(Ptr)` —— 取显示名
- `GetTagDisplayName(TagData)` —— 取 FTagData 显示名
- `GetTagDisplayPathName(TagData)` —— 取完整路径名（2 个重载）
- `GetTagUniqueName(TagData)` —— 取唯一名
- `GetTagParent(TagData)` —— 取父标签
- `GetTagIsEnumTag(TagData)` —— 是否枚举标签
- `GetTagClosestEnumTag(TagData)` —— 取最近的枚举标签祖先
- `GetTagAmountForTracker(...)` —— 取标签内存量（3 个重载）
- `SetTagAmountForTracker(...)` —— 设置标签内存量（2 个重载）
- `DumpTag(Tracker, FileName, LineNumber)` —— 转储当前标签
- `PublishDataSingleFrame()` —— 发布单帧数据

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 701~703 行 | 2 个注册方法 |
| 第 705~712 行 | 4 个查询方法 |
| 第 714~725 行 | 5 个名字查询 |
| 第 727~736 行 | 4 个路径/父标签 |
| 第 738~745 行 | 2 个枚举标签判断 |
| 第 747~759 行 | 5 个数量查询/设置 |
| 第 761~765 行 | `DumpTag` / `PublishDataSingleFrame` |

> 类比：像查账窗口：能查某个科目的余额、能查科目全名、能查父子关系。

### 十四、`FLowLevelMemTracker` 转储 + 初始化（第 767~801 行）

```cpp
// EDumpFormat：转储格式。
	enum class EDumpFormat
	{
// PlainText：纯文本。
		PlainText,
// CSV：CSV 格式。
		CSV,
// 枚举体结束。
	};
// DumpToLog：转储到日志。
	CORE_API void DumpToLog(EDumpFormat DumpFormat = EDumpFormat::PlainText, FOutputDevice* OutputDevice = nullptr, UE::LLM::ESizeParams SizeParams = UE::LLM::ESizeParams::Default, ELLMTagSet TagSet = ELLMTagSet::None);

// OnPreFork：fork 前回调。
	CORE_API void OnPreFork();

// IsInitialized：是否已初始化。
	FORCEINLINE bool IsInitialized() const
// 函数体开始。
	{
// 返回标志。
		return bFullyInitialised;
// 函数体结束。
	}

// IsConfigured：是否已配置完成。
	FORCEINLINE bool IsConfigured() const
// 函数体开始。
	{
// 返回标志。
		return bConfigurationComplete;
// 函数体结束。
	}

// 如果还没做，初始化追踪分配所需的数据。 这可以在单例 FLowLevelMemTracker 构造之后发生。 任何需要它的 LLM 函数都会自动调用，但外部系统需要小心管理静态初始化顺序时也可主动触发。 BootstrapInitialize 不会让 GetTagUniqueName 可用；那需要 FinishInitialize。
// BootstrapInitialise：引导期初始化（不提供 GetTagUniqueName）。
	CORE_API void BootstrapInitialise();

// 如果还没做就完成初始化。必要时会调 BootstrapInitialise。调用后 LLM 里所有函数（包括返回 FName 的）都可用。 应在 FName 可用后尽快调用，以便依赖 LLM FName 数据的其他追踪系统尽快开始追踪。
// FinishInitialise：完成初始化（所有函数可用）。
	CORE_API void FinishInitialise();
```

**本段新元素**

- `enum class EDumpFormat` —— 转储格式（`PlainText` / `CSV`）
- `DumpToLog(DumpFormat, OutputDevice, SizeParams, TagSet)` —— 转储到日志
- `OnPreFork()` —— fork 前回调
- `IsInitialized()` —— 是否已初始化
- `IsConfigured()` —— 是否已配置完成
- `BootstrapInitialise()` —— 引导期初始化（**不提供 `GetTagUniqueName`**）
- `FinishInitialise()` —— 完成初始化（所有函数可用）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 767~772 行 | `EDumpFormat` + `DumpToLog` |
| 第 774 行 | `OnPreFork` |
| 第 776~784 行 | `IsInitialized` / `IsConfigured` |
| 第 786~793 行 | `BootstrapInitialise` |
| 第 795~801 行 | `FinishInitialise` |

> 类比：像分两阶段开业：先开个临时窗口（Bootstrap），等 FName 系统就绪再正式开业（Finish）。

### 十五、`FLowLevelMemTracker` 私有实现（第 803~914 行）

```cpp
// 私有区。
private:
// EEnabled：启用状态。
	enum class EEnabled : uint8
	{
// NotYetKnown：还不知道（命令行未处理）。
		NotYetKnown = 0,
// Disabled：已禁用。
		Disabled,
// Enabled：已启用。
		Enabled,
// 枚举体结束。
	};
// 前置声明 FEnableStateScopeLock。
	struct FEnableStateScopeLock;

// 私有区。
private:
// 构造函数。
	FLowLevelMemTracker();

// 析构函数。
	~FLowLevelMemTracker();

// IsBootstrapping：是否处于引导期。
	bool IsBootstrapping() const { return bIsBootstrapping; }

	/** 释放所有内存。这会让追踪器进入永久禁用状态。 */
// Clear：清空。
	void Clear();
// InitialiseProgramSize：初始化程序大小。
	void InitialiseProgramSize();

// GetTracker：取追踪器（非 const）。
	class UE::LLMPrivate::FLLMTracker* GetTracker(ELLMTracker Tracker);
// GetTracker：取追踪器（const）。
	const class UE::LLMPrivate::FLLMTracker* GetTracker(ELLMTracker Tracker) const;

// TickInternal：内部 tick。
	void TickInternal();
// UpdateTags：更新标签。
	void UpdateTags();
// SortTags：排序标签。
	void SortTags(UE::LLMPrivate::FTagDataArray*& OutOldTagDatas);
// PublishDataPerFrame：每帧发布数据。
	void PublishDataPerFrame(const TCHAR* LogName);

// RegisterCustomTagInternal：注册自定义标签的内部实现。
	void RegisterCustomTagInternal(int32 Tag, ELLMTagSet TagSet, const TCHAR* Name, FName StatName, FName SummaryStatName, int32 ParentTag = -1);
// 在 GMalloc 可用（因此 FName 不可用）的 C++ 全局静态初始化期间调用 创建记录 GMalloc 和 FName 构造期间分配所需的最小标签子集
// BootstrapTagDatas：创建引导期标签数据。
	void BootstrapTagDatas();
// InitialiseTagDatas_SetLLMTagNames：设置 LLM 标签名。
	void InitialiseTagDatas_SetLLMTagNames();
// InitialiseTagDatas_FinishRegister：完成注册。
	void InitialiseTagDatas_FinishRegister();
// InitialiseTagDatas：初始化标签数据。
	void InitialiseTagDatas();
// ClearTagDatas：清空标签数据。
	void ClearTagDatas();
// RegisterTagDeclaration：注册标签声明。
	void RegisterTagDeclaration(FLLMTagDeclaration& TagDeclaration);
// RegisterTagData：注册标签数据。
	UE::LLMPrivate::FTagData& RegisterTagData(FName Name, FName DisplayName, FName ParentName, FName StatName, FName SummaryStatName, bool bHasEnumTag, ELLMTag EnumTag, bool bIsStatTag, UE::LLMPrivate::ETagReferenceSource ReferenceSource, ELLMTagSet TagSet = ELLMTagSet::None);
	/** 如果还没做，构造给定 FTagData 上依赖其他 TagData 存在的数据 */
// FinishConstruct：完成构造（依赖其他 TagData）。
	void FinishConstruct(UE::LLMPrivate::FTagData* TagData, UE::LLMPrivate::ETagReferenceSource ReferenceSource);
// ReportDuplicateTagName：报告重复标签名。
	void ReportDuplicateTagName(UE::LLMPrivate::FTagData* TagData, UE::LLMPrivate::ETagReferenceSource ReferenceSource);

// FindOrAddTagData：查找或新建（枚举版）。
	const UE::LLMPrivate::FTagData* FindOrAddTagData(ELLMTag EnumTag, UE::LLMPrivate::ETagReferenceSource ReferenceSource = UE::LLMPrivate::ETagReferenceSource::FunctionAPI);
// FindOrAddTagData：查找或新建（FName 版）。
	const UE::LLMPrivate::FTagData* FindOrAddTagData(FName Name, ELLMTagSet TagSet, bool bIsStatData=false, UE::LLMPrivate::ETagReferenceSource ReferenceSource = UE::LLMPrivate::ETagReferenceSource::FunctionAPI);
// FindOrAddTagData：查找或新建（FName + StatName 版）。
	const UE::LLMPrivate::FTagData* FindOrAddTagData(FName Name, ELLMTagSet TagSet, FName StatName, UE::LLMPrivate::ETagReferenceSource ReferenceSource = UE::LLMPrivate::ETagReferenceSource::FunctionAPI);
// FindTagData：查找（枚举版）。
	const UE::LLMPrivate::FTagData* FindTagData(ELLMTag EnumTag, UE::LLMPrivate::ETagReferenceSource ReferenceSource = UE::LLMPrivate::ETagReferenceSource::FunctionAPI);
// FindTagData：查找（FName 版）。
	const UE::LLMPrivate::FTagData* FindTagData(FName Name, ELLMTagSet TagSet, UE::LLMPrivate::ETagReferenceSource ReferenceSource = UE::LLMPrivate::ETagReferenceSource::FunctionAPI);

// 返回 IsEnabled()，但当前状态是 NotYetKnown 时也会加锁，因为 主线程调 ProcessCommandLine 且命令行指定 LLM 应禁用时状态可能改变。 只有 PLATFORM_HAS_MULTITHREADED_PREMAIN 的平台需要锁；其他平台在 ProcessCommandLine 调用前不是多线程的。这个函数只在 new/delete 处理器 （OnLowLevelAlloc 等）里需要，因为只有这些函数可能在 ProcessCommandLine 前被其他线程调用。
// TryEnterEnabled：尝试进入启用状态（必要时加锁）。
	inline static bool TryEnterEnabled(FEnableStateScopeLock& ScopeLock);

// 友元：FLLMPauseScope。
	friend class FLLMPauseScope;
// 友元：FLLMScope。
	friend class FLLMScope;
// 友元：FLLMScopeDynamic。
	friend class FLLMScopeDynamic;
// 友元：FLLMScopeFromPtr。
	friend class FLLMScopeFromPtr;
// 友元：FLLMCsvWriter。
	friend class UE::LLMPrivate::FLLMCsvWriter;
// 友元：FLLMTracker。
	friend class UE::LLMPrivate::FLLMTracker;
// 友元：FLLMThreadState。
	friend class UE::LLMPrivate::FLLMThreadState;
// 友元：FLLMTraceWriter。
	friend class UE::LLMPrivate::FLLMTraceWriter;
// 友元：FLLMCsvProfilerWriter。
	friend class UE::LLMPrivate::FLLMCsvProfilerWriter;
// 友元：GlobalRegisterTagDeclaration。
	friend void GlobalRegisterTagDeclaration(FLLMTagDeclaration& TagDeclaration);

// 私有区。
private:
// Allocator：LLM 内部内存分配器。
	UE::LLMPrivate::FLLMAllocator Allocator;
	/** 所有已构造的 TagData，按 TagData->GetIndex() 排序的数组 */
// TagDatas：所有已构造的 TagData（按索引排序）。
	UE::LLMPrivate::FTagDataArray* TagDatas;
	/** 从 TagData->GetName() 到 TagData 的映射，用于处理带 FName 的 LLM_SCOPE */
// TagDataNameMap：名字 → TagData 映射。
	UE::LLMPrivate::FTagDataNameMap* TagDataNameMap;
	/** 从 ELLMTag 到该标签 TagData 的映射数组，用于处理带 ELLMTag 的 LLM_SCOPE */
// TagDataEnumMap：枚举值 → TagData 映射。
	UE::LLMPrivate::FTagData** TagDataEnumMap;

// Trackers：追踪器数组。
	UE::LLMPrivate::FLLMTracker* Trackers[static_cast<int32>(ELLMTracker::Max)];

// TagDataLock：保护 TagData 的读写锁。
	mutable FRWLock TagDataLock;
// UpdateLock：保护更新的锁。
	FCriticalSection UpdateLock;
// BootstrapLock：保护引导期的锁。
	FCriticalSection BootstrapLock;

// ProgramSize：程序本身大小。
	uint64 ProgramSize;
// MemoryUsageCurrentOverhead：当前 LLM 自身开销。
	int64 MemoryUsageCurrentOverhead;
// MemoryUsagePlatformTotalUntracked：平台未追踪总量。
	int64 MemoryUsagePlatformTotalUntracked;

// ActiveSets：各标签集是否激活。
	bool ActiveSets[(int32)ELLMTagSet::Max];

// bFirstTimeUpdating：是否首次更新。
	bool bFirstTimeUpdating;
// bCanEnable：是否可以启用。
	bool bCanEnable;
// bCsvWriterEnabled：CSV 写入器是否启用。
	bool bCsvWriterEnabled;
// bTraceWriterEnabled：Trace 写入器是否启用。
	bool bTraceWriterEnabled;
// bInitialisedTracking：是否已初始化追踪。
	bool bInitialisedTracking;
// bIsBootstrapping：是否处于引导期。
	bool bIsBootstrapping;
// bBeganInitialisedTracking：是否已开始初始化追踪。
	bool bBeganInitialisedTracking = false;
// bFullyInitialised：是否完全初始化。
	bool bFullyInitialised;
// bConfigurationComplete：配置是否完成（原子）。
	std::atomic<bool> bConfigurationComplete;
// bTagAdded：是否已添加标签。
	bool bTagAdded;
// bAutoPublish：是否自动发布。
	bool bAutoPublish;
// bPublishSingleFrame：是否发布单帧。
	bool bPublishSingleFrame;
// bCapturedSizeSnapshot：是否已捕获大小快照。
	bool bCapturedSizeSnapshot;

// TrackerInstance：单例指针。
	static CORE_API FLowLevelMemTracker* TrackerInstance;
// EnabledState：启用状态。
	static CORE_API EEnabled EnabledState;
// 公有区。
public:
// 标记 bIsDisabled 已废弃（5.5），改用 IsEnabled。
	UE_DEPRECATED(5.5, "Use IsEnabled instead.")
// bIsDisabled：是否禁用（已废弃）。
	static CORE_API bool bIsDisabled;
// 类结束。
};
```

**本段新元素**

- `enum class EEnabled` —— 启用状态（`NotYetKnown` / `Disabled` / `Enabled`）
- `struct FEnableStateScopeLock` —— 启用状态作用域锁
- `IsBootstrapping()` —— 是否处于引导期
- `Clear()` —— 释放所有内存（永久禁用）
- `GetTracker(Tracker)` —— 取追踪器（2 个重载）
- `TickInternal` / `UpdateTags` / `SortTags` / `PublishDataPerFrame` —— 4 个内部方法
- `RegisterCustomTagInternal(...)` —— 注册自定义标签
- `BootstrapTagDatas()` —— 创建引导期标签数据
- `InitialiseTagDatas_*` —— 3 个初始化方法
- `RegisterTagDeclaration` / `RegisterTagData` —— 2 个注册方法
- `FindOrAddTagData(...)` —— 查找或新建（3 个重载）
- `FindTagData(...)` —— 查找（2 个重载）
- `TryEnterEnabled(ScopeLock)` —— 尝试进入启用状态（必要时加锁）
- 10 个 `friend class` —— 友元
- `FLLMAllocator Allocator` —— LLM 内部分配器
- `FTagDataArray* TagDatas` —— 所有 TagData（按索引排序）
- `FTagDataNameMap* TagDataNameMap` —— 名字 → TagData
- `FTagData** TagDataEnumMap` —— 枚举值 → TagData
- `FLLMTracker* Trackers[Max]` —— 追踪器数组
- `FRWLock TagDataLock` / `FCriticalSection UpdateLock` / `BootstrapLock` —— 3 个锁
- `uint64 ProgramSize` / `int64 MemoryUsageCurrentOverhead` / `MemoryUsagePlatformTotalUntracked` —— 3 个量
- `bool ActiveSets[Max]` —— 各标签集是否激活
- 13 个 `bool` 状态标志
- `static FLowLevelMemTracker* TrackerInstance` —— 单例指针
- `static EEnabled EnabledState` —— 启用状态
- `static bool bIsDisabled` —— **已废弃（5.5）**

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 803~810 行 | `EEnabled` + `FEnableStateScopeLock` |
| 第 812~821 行 | 构造/析构 + `IsBootstrapping` / `Clear` |
| 第 823~831 行 | `GetTracker` + 4 个内部方法 + `RegisterCustomTagInternal` |
| 第 832~845 行 | 标签数据初始化 8 个方法 |
| 第 847~851 行 | 5 个查找方法 |
| 第 853~861 行 | `TryEnterEnabled` |
| 第 863~872 行 | 10 个友元 |
| 第 874~883 行 | 分配器 + 3 个标签数据结构 + 追踪器数组 |
| 第 885~893 行 | 3 个锁 + 3 个量 + `ActiveSets` |
| 第 895~907 行 | 13 个状态标志 |
| 第 909~914 行 | 2 个静态成员 + 废弃成员 |

> 类比：像账房的后屋：账本（TagDatas）、索引（NameMap / EnumMap）、锁、各种状态标志。

### 十六、`FLLMScope` 作用域类（第 916~932 行）

```cpp
/** 用于追踪内存的 LLM 作用域。 */
// FLLMScope：LLM 内存追踪作用域。
class FLLMScope
{
// 公有区。
public:
// 构造函数（FName 版）。
	CORE_API explicit FLLMScope(FName TagName, bool bIsStatTag, ELLMTagSet InTagSet, ELLMTracker InTracker, bool bOverride = true);
// 构造函数（ELLMTag 版）。
	CORE_API explicit FLLMScope(ELLMTag TagEnum, bool bIsStatTag, ELLMTagSet InTagSet, ELLMTracker InTracker, bool bOverride = true);
// 构造函数（FTagData 版）。
	CORE_API explicit FLLMScope(const UE::LLMPrivate::FTagData* TagData, bool bIsStatTag, ELLMTagSet InTagSet, ELLMTracker InTracker, bool bOverride = true);
// 析构函数。
	CORE_API ~FLLMScope();

// 受保护区。
protected:
// Destruct：析构实现。
	CORE_API void Destruct();

	// 除 bEnabled 外的所有字段在 bEnabled 为 true 前都不由构造函数初始化
// Tracker：追踪器。
	ELLMTracker Tracker;
	bool bEnabled = false;  // 必须是 uint8 而不是位域，以减少 ALU 操作
// TagSet：标签集。
	ELLMTagSet TagSet;
// 类结束。
};
```

**本段新元素**

- `class FLLMScope` —— LLM 内存追踪作用域
- 3 个构造函数 —— `FName` / `ELLMTag` / `FTagData*` 版
- `~FLLMScope()` —— 析构时恢复父标签
- `Destruct()` —— 析构实现
- `ELLMTracker Tracker` —— 追踪器
- `bool bEnabled = false` —— 是否启用（**用 `uint8` 而非位域，减少 ALU 操作**）
- `ELLMTagSet TagSet` —— 标签集

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 916~923 行 | 类声明 + 3 个构造 + 析构 |
| 第 925~926 行 | `Destruct` |
| 第 928~931 行 | 3 个成员 |

> 类比：像进房间开灯、出门关灯：构造时打标签，析构时恢复。

### 十七、`ILLMDynamicTagConstructor` + `FLLMDynamicTagConstructorStatString`（第 934~959 行）

```cpp
// 为 TagConstruction 提供参数，通过 LLM_SCOPE_DYNAMIC 宏传递， 且只应在第一次 LLM_SCOPE_DYNAMIC 调用时求值。
// ILLMDynamicTagConstructor：动态标签构造器接口。
class ILLMDynamicTagConstructor
{
// 公有区。
public:
// 虚析构函数。
	virtual ~ILLMDynamicTagConstructor() = default;
// GetStatName：取统计名。
	virtual FString GetStatName() const
// 函数体开始。
	{
// 默认返回空字符串。
		return FString();
// 函数体结束。
	}
// NeedsStatConstruction：是否需要构造统计。
	virtual bool NeedsStatConstruction() const
// 函数体开始。
	{
// 默认 true。
		return true;
// 函数体结束。
	}
// 类结束。
};

/** ILLMDynamicTagConstructor，提供字符串传给 CreateMemoryStatId。 */
// FLLMDynamicTagConstructorStatString：提供字符串给 CreateMemoryStatId。
class FLLMDynamicTagConstructorStatString : public ILLMDynamicTagConstructor
{
// 公有区。
public:
// 构造函数：保存统计名。
	FLLMDynamicTagConstructorStatString(FString InStatName) : StatName(MoveTemp(InStatName)) {}
// GetStatName：返回统计名。
	virtual FString GetStatName() const override { return StatName; }
// StatName：统计名。
	FString StatName;
// 类结束。
};
```

**本段新元素**

- `class ILLMDynamicTagConstructor` —— 动态标签构造器接口
- `virtual ~ILLMDynamicTagConstructor() = default` —— 虚析构
- `GetStatName()` —— 取统计名（默认空）
- `NeedsStatConstruction()` —— 是否需要构造统计（默认 true）
- `class FLLMDynamicTagConstructorStatString : public ILLMDynamicTagConstructor` —— 提供字符串版
- `FLLMDynamicTagConstructorStatString(FString InStatName)` —— 构造
- `GetStatName()` —— 返回保存的统计名
- `FString StatName` —— 统计名

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 934~950 行 | `ILLMDynamicTagConstructor` |
| 第 952~959 行 | `FLLMDynamicTagConstructorStatString` |

> 类比：像给新科目起名的规则：告诉系统「这个动态标签该叫什么统计名」。

### 十八、`FLLMScopeDynamic` 动态作用域（第 961~999 行）

```cpp
// 用于追踪动态创建标签的内存作用域。和普通标签一样，动态创建的标签 按名字创建，而不是像 ELLMTag 或 LLM_DECLARE_TAG 那样的编译期常量， 由第一个使用它们的作用域按需创建。 与普通标签不同，动态创建的标签还能在首次调用时创建统计。
// FLLMScopeDynamic：动态创建标签的作用域。
class FLLMScopeDynamic
{
// 公有区。
public:
// 构造函数：LLM 启用时才初始化。
	FLLMScopeDynamic(ELLMTracker InTracker, ELLMTagSet InTagSet)
// 函数体开始。
	{
// 如果 LLM 启用。
		if (FLowLevelMemTracker::IsEnabled())
// 条件体开始。
		{
// 初始化。
			Init(InTracker, InTagSet);
// 条件体结束。
		}
// 函数体结束。
	}
// IsEnabled：是否启用。
	bool IsEnabled() const { return bEnabled; }
// TryFindTag：尝试查找标签。
	CORE_API bool TryFindTag(FName UniqueName); // Set name separately so the calculation of name can be skipped if TagSet is disabled
// TryAddTagAndActivate：新建标签并激活。
	CORE_API bool TryAddTagAndActivate(FName UniqueName, const ILLMDynamicTagConstructor& Constructor);
// Activate：激活。
	CORE_API void Activate();

// 析构函数：启用时才析构。
	~FLLMScopeDynamic()
// 函数体开始。
	{
// 如果启用。
		if (bEnabled)
// 条件体开始。
		{
// 析构。
			Destruct();
// 条件体结束。
		}
// 函数体结束。
	}

// 受保护区。
protected:
// Init：初始化实现。
	CORE_API void Init(ELLMTracker InTracker, ELLMTagSet InTagSet);
// Destruct：析构实现。
	CORE_API void Destruct();

	// 除 bEnabled 外的所有字段在 bEnabled 为 true 前都不由构造函数初始化
// TagData：标签数据。
	const UE::LLMPrivate::FTagData* TagData;
// Tracker：追踪器。
	ELLMTracker Tracker;
	bool bEnabled = false; // 必须是 uint8 而不是位域，以减少 ALU 操作
// TagSet：标签集。
	ELLMTagSet TagSet;
// 类结束。
};
```

**本段新元素**

- `class FLLMScopeDynamic` —— 动态创建标签的作用域
- 构造函数：LLM 启用时才 `Init`
- `IsEnabled()` —— 是否启用
- `TryFindTag(UniqueName)` —— 尝试查找标签
- `TryAddTagAndActivate(UniqueName, Constructor)` —— 新建标签并激活
- `Activate()` —— 激活
- `~FLLMScopeDynamic()` —— 启用时才 `Destruct`
- `Init` / `Destruct` —— 实现在 .cpp
- `const FTagData* TagData` —— 标签数据
- `ELLMTracker Tracker` / `bool bEnabled` / `ELLMTagSet TagSet` —— 3 个成员

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 961~976 行 | 类声明 + 构造 |
| 第 977~980 行 | 4 个方法 |
| 第 982~988 行 | 析构 |
| 第 990~998 行 | `Init` / `Destruct` + 4 个成员 |

> 类比：像按需开户：第一次用到才创建科目，之后复用。

### 十九、`FLLMPauseScope` + `FLLMClearScope` + `FLLMScopeFromPtr`（第 1001~1038 行）

```cpp
/** 暂停 LLM 的作用域（禁用分配钩子）。 */
// FLLMPauseScope：暂停 LLM 的作用域（禁用分配钩子）。
class FLLMPauseScope
{
// 公有区。
public:
// 构造函数（FName 版）。
	CORE_API FLLMPauseScope(FName TagName, bool bIsStatTag, uint64 Amount, ELLMTracker TrackerToPause, ELLMAllocType InAllocType);
// 构造函数（ELLMTag 版）。
	CORE_API FLLMPauseScope(ELLMTag TagEnum, bool bIsStatTag, uint64 Amount, ELLMTracker TrackerToPause, ELLMAllocType InAllocType);
// 析构函数。
	CORE_API ~FLLMPauseScope();
// 受保护区。
protected:
// Init：初始化实现。
	CORE_API void Init(FName TagName, ELLMTag EnumTag, bool bIsEnumTag, bool bIsStatTag, uint64 Amount,
// 续行：参数。
		ELLMTracker TrackerToPause, ELLMAllocType InAllocType);
// PausedTracker：被暂停的追踪器。
	ELLMTracker PausedTracker;
// AllocType：分配类型。
	ELLMAllocType AllocType;
// bEnabled：是否启用。
	bool bEnabled;
// 类结束。
};

/** 在作用域内清除顶层标签的 LLM 作用域 */
// FLLMClearScope：清除顶层标签的作用域。
class FLLMClearScope : public FLLMScope
{
// 公有区。
public:
// 构造函数。
	CORE_API FLLMClearScope(ELLMTagSet InTagSet, ELLMTracker InTracker);
// 类结束。
};

/** 从给定地址继承标签的 LLM 作用域。 */
// FLLMScopeFromPtr：从指针继承标签的作用域。
class FLLMScopeFromPtr
{
// 公有区。
public:
// 构造函数。
	CORE_API FLLMScopeFromPtr(void* Ptr, ELLMTracker Tracker);
// 析构函数。
	CORE_API ~FLLMScopeFromPtr();
// 受保护区。
protected:
// Tracker：追踪器。
	ELLMTracker Tracker;
// bEnabled：各标签集是否启用。
	bool bEnabled[static_cast<int32>(ELLMTagSet::Max)];

// 私有区。
private:
// DisableAll：禁用所有标签集。
	inline void DisableAll()
// 函数体开始。
	{
// 内存清零。
		FMemory::Memzero(bEnabled);
// 函数体结束。
	}
// 类结束。
};
```

**本段新元素**

- `class FLLMPauseScope` —— 暂停 LLM 的作用域（禁用分配钩子）
- 2 个构造函数 —— `FName` / `ELLMTag` 版
- `Init(...)` —— 初始化实现
- `ELLMTracker PausedTracker` / `ELLMAllocType AllocType` / `bool bEnabled` —— 3 个成员
- `class FLLMClearScope : public FLLMScope` —— 清除顶层标签的作用域
- `class FLLMScopeFromPtr` —— 从指针继承标签的作用域
- `FLLMScopeFromPtr(void* Ptr, ELLMTracker Tracker)` —— 构造
- `bool bEnabled[Max]` —— 各标签集是否启用
- `DisableAll()` —— 内存清零禁用所有

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1001~1014 行 | `FLLMPauseScope` |
| 第 1016~1021 行 | `FLLMClearScope` |
| 第 1023~1038 行 | `FLLMScopeFromPtr` |

> 类比：暂停像「这段别记账」，清除像「这段算公共开销」，继承像「跟着原指针走」。

### 二十、`FLLMTagDeclaration` + `FLLMTagSetAllocationFilter`（第 1040~1079 行）

```cpp
/** 全局实例，向 LLM 提供标签信息。 */
// FLLMTagDeclaration：全局实例，向 LLM 提供标签信息。
class FLLMTagDeclaration
{
// 公有区。
public:
// 构造函数。
	CORE_API FLLMTagDeclaration(const TCHAR* InCPPName, const FName InDisplayName=NAME_None, FName InParentTagName = NAME_None, FName InStatName = NAME_None, FName InSummaryStatName = NAME_None, ELLMTagSet TagSet = ELLMTagSet::None);
// GetUniqueName：取唯一名。
	FName GetUniqueName() const { return UniqueName; }

// FCreationCallback：创建回调类型。
	typedef void (*FCreationCallback)(FLLMTagDeclaration&);
// 受保护区。
protected:

// AddCreationCallback：添加创建回调。
	static CORE_API void AddCreationCallback(FCreationCallback InCallback);
// ClearCreationCallbacks：清空创建回调。
	static CORE_API void ClearCreationCallbacks();
// GetCreationCallbacks：取创建回调列表。
	static CORE_API TArrayView<FCreationCallback> GetCreationCallbacks();
// GetList：取标签声明链表。
	static CORE_API FLLMTagDeclaration* GetList();

// Register：注册。
	CORE_API void Register();

// 受保护区。
protected:
// ConstructUniqueName：构造唯一名。
	CORE_API void ConstructUniqueName();

// CPPName：C++ 名字。
	const TCHAR* CPPName;
// UniqueName：唯一名。
	FName UniqueName;
// DisplayName：显示名。
	FName DisplayName;
// ParentTagName：父标签名。
	FName ParentTagName;
// StatName：统计名。
	FName StatName;
// SummaryStatName：汇总统计名。
	FName SummaryStatName;
// TagSet：标签集。
	ELLMTagSet TagSet;
// Next：链表下一个。
	FLLMTagDeclaration* Next = nullptr;

// 友元：FLowLevelMemTracker。
	friend class FLowLevelMemTracker;
// 类结束。
};

/** 用于过滤标签分配，可指定匹配 TagSet 里的 Name、或只按 TagSet、或只按 Name。 */
// FLLMTagSetAllocationFilter：标签分配过滤器。
struct FLLMTagSetAllocationFilter
{
	/** 要匹配的标签名 */
// Name：标签名。
	FName Name;
	/** 要匹配的标签集 */
// TagSet：标签集。
	ELLMTagSet TagSet;
// 结构体结束。
};
```

**本段新元素**

- `class FLLMTagDeclaration` —— 全局实例，向 LLM 提供标签信息
- `FLLMTagDeclaration(InCPPName, InDisplayName, InParentTagName, InStatName, InSummaryStatName, TagSet)` —— 构造
- `GetUniqueName()` —— 取唯一名
- `FCreationCallback` —— 创建回调类型
- `AddCreationCallback` / `ClearCreationCallbacks` / `GetCreationCallbacks` —— 3 个回调管理
- `GetList()` —— 取标签声明链表
- `Register()` —— 注册
- `ConstructUniqueName()` —— 构造唯一名
- `CPPName` / `UniqueName` / `DisplayName` / `ParentTagName` / `StatName` / `SummaryStatName` / `TagSet` / `Next` —— 8 个成员
- `struct FLLMTagSetAllocationFilter` —— 标签分配过滤器
- `FName Name` / `ELLMTagSet TagSet` —— 2 个字段

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1040~1045 行 | 类声明 + 构造 + `GetUniqueName` |
| 第 1047~1055 行 | 回调管理 + `Register` |
| 第 1057~1070 行 | `ConstructUniqueName` + 8 个成员 |
| 第 1072~1079 行 | `FLLMTagSetAllocationFilter` |

> 类比：像科目登记表：每个科目一张表，登记后挂到链表上。

### 二十一、`IsEnabled` 内联实现 + `#else` 空宏分支（第 1081~1127 行）

```cpp
// IsEnabled：内联实现。
inline bool FLowLevelMemTracker::IsEnabled()
// 函数体开始。
{
// 只要不是 Disabled 就算启用。
	return EnabledState != EEnabled::Disabled;
// 函数体结束。
}

// 否则（未启用 LLM）。
#else

// 如果没定义 LLM_ALLOW_ASSETS_TAGS。
#ifndef LLM_ALLOW_ASSETS_TAGS
// 默认 0。
	#define LLM_ALLOW_ASSETS_TAGS 0
// 条件编译结束。
#endif

// 如果没定义 LLM_ALLOW_UOBJECTCLASSES_TAGS。
#ifndef LLM_ALLOW_UOBJECTCLASSES_TAGS
// 默认 0。
	#define LLM_ALLOW_UOBJECTCLASSES_TAGS 0
// 条件编译结束。
#endif

// LLM：空宏。
#define LLM(...)
// LLM_IF_ENABLED：空宏。
#define LLM_IF_ENABLED(...)
// LLM_IS_ENABLED：永远 false。
#define LLM_IS_ENABLED() false
// LLM_SCOPE：空宏。
#define LLM_SCOPE(...)
// LLM_SCOPE_DYNAMIC：空宏。
#define LLM_SCOPE_DYNAMIC(...)
// LLM_TAGSET_SCOPE：空宏。
#define LLM_TAGSET_SCOPE(...)
// LLM_SCOPE_BYNAME：空宏。
#define LLM_SCOPE_BYNAME(...)
// LLM_SCOPE_BYTAG：空宏。
#define LLM_SCOPE_BYTAG(...)
// LLM_SCOPE_RENDER_RESOURCE：空宏。
#define LLM_SCOPE_RENDER_RESOURCE(...)
// LLM_PLATFORM_SCOPE：空宏。
#define LLM_PLATFORM_SCOPE(...)
// LLM_PLATFORM_SCOPE_BYNAME：空宏。
#define LLM_PLATFORM_SCOPE_BYNAME(...)
// LLM_PLATFORM_SCOPE_BYTAG：空宏。
#define LLM_PLATFORM_SCOPE_BYTAG(...)
// LLM_SCOPE_CLEAR：空宏。
#define LLM_SCOPE_CLEAR()
// LLM_TAGSET_SCOPE_CLEAR：空宏。
#define LLM_TAGSET_SCOPE_CLEAR(...)
// LLM_REALLOC_SCOPE：空宏。
#define LLM_REALLOC_SCOPE(...)
// LLM_REALLOC_PLATFORM_SCOPE：空宏。
#define LLM_REALLOC_PLATFORM_SCOPE(...)
// LLM_SCOPED_PAUSE_TRACKING：空宏。
#define LLM_SCOPED_PAUSE_TRACKING(...)
// LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER：空宏。
#define LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER(...)
// LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT：空宏。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT(...)
// LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT_BYTAG：空宏。
#define LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT_BYTAG(...)
// LLM_DUMP_TAG：空宏。
#define LLM_DUMP_TAG()
// LLM_DUMP_PLATFORM_TAG：空宏。
#define LLM_DUMP_PLATFORM_TAG()
// LLM_DEFINE_TAG：空宏。
#define LLM_DEFINE_TAG(...)
// LLM_DEFINE_STATIC_TAG：空宏。
#define LLM_DEFINE_STATIC_TAG(...)
// LLM_DECLARE_TAG：空宏。
#define LLM_DECLARE_TAG(...)
// LLM_DECLARE_TAG_API：空宏。
#define LLM_DECLARE_TAG_API(...)
// LLM_DEFINE_BOOTSTRAP_TAG：空宏。
#define LLM_DEFINE_BOOTSTRAP_TAG(...)
// LLM_SCOPE_BY_BOOTSTRAP_TAG：空宏。
#define LLM_SCOPE_BY_BOOTSTRAP_TAG(...)
// LLM_DECLARE_BOOTSTRAP_TAG：空宏。
#define LLM_DECLARE_BOOTSTRAP_TAG(...) 
// LLM_DECLARE_BOOTSTRAP_TAG_API：空宏。
#define LLM_DECLARE_BOOTSTRAP_TAG_API(...)

// 条件编译结束（ENABLE_LOW_LEVEL_MEM_TRACKER）。
#endif		// #if ENABLE_LOW_LEVEL_MEM_TRACKER
```

**本段新元素**

- `inline bool FLowLevelMemTracker::IsEnabled()` —— 内联实现
- `return EnabledState != EEnabled::Disabled` —— 只要不是 Disabled 就算启用
- `#else` —— 未启用 LLM 时
- `LLM_ALLOW_ASSETS_TAGS` / `LLM_ALLOW_UOBJECTCLASSES_TAGS` —— 默认 0
- 30+ 个空宏 —— `LLM` / `LLM_IF_ENABLED` / `LLM_SCOPE` / `LLM_DEFINE_TAG` 等全变空
- `LLM_IS_ENABLED()` —— 永远 `false`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1081~1084 行 | `IsEnabled` 内联实现 |
| 第 1086~1094 行 | `#else` + 2 个默认宏 |
| 第 1096~1125 行 | 30+ 个空宏 |
| 第 1127 行 | `#endif` |

> 类比：像正式演出时把排练道具全收起来：不占地方、不影响性能。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 平台支持判断 + 配置宏 | 1~52 | 版权头 + 平台支持判断 + 配置宏 |
| 断言宏（LLMCheck / LLMCheckf / LLMEnsure） | 54~108 | 断言宏（LLMCheck / LLMCheckf / LLMEnsure） |
| `ELLMTracker` + `ELLMTagSet` + `ESizeParams` | 110~160 | `ELLMTracker` + `ELLMTagSet` + `ESizeParams` |
| `LLM_ENUM_GENERIC_TAGS` —— 100+ 个标签 | 163~280 | `LLM_ENUM_GENERIC_TAGS` —— 100+ 个标签 |
| `ELLMTag` 枚举 + 4 个常量 | 282~311 | `ELLMTag` 枚举 + 4 个常量 |
| `ELLMAllocType` + 4 个基础宏 | 313~335 | `ELLMAllocType` + 4 个基础宏 |
| 作用域宏（LLM_SCOPE 系列） | 341~388 | 作用域宏（LLM_SCOPE 系列） |
| 暂停宏 + realloc 宏 + 转储宏 | 390~408 | 暂停宏 + realloc 宏 + 转储宏 |
| 标签定义宏（DEFINE / DECLARE / BOOTSTRAP） | 410~468 | 标签定义宏（DEFINE / DECLARE / BOOTSTRAP） |
| `UE::LLMPrivate` 命名空间（FLLMAllocator 等） | 470~587 | `UE::LLMPrivate` 命名空间（FLLMAllocator 等） |
| `FLLMCustomTag`（废弃）+ `FLLMTagInfo` | 589~604 | `FLLMCustomTag`（废弃）+ `FLLMTagInfo` |
| `FLowLevelMemTracker` 单例 + 核心 API | 606~700 | `FLowLevelMemTracker` 单例 + 核心 API |
| `FLowLevelMemTracker` 标签查询 API | 701~765 | `FLowLevelMemTracker` 标签查询 API |
| `FLowLevelMemTracker` 转储 + 初始化 | 767~801 | `FLowLevelMemTracker` 转储 + 初始化 |
| `FLowLevelMemTracker` 私有实现 | 803~914 | `FLowLevelMemTracker` 私有实现 |
| `FLLMScope` 作用域类 | 916~932 | `FLLMScope` 作用域类 |
| `ILLMDynamicTagConstructor` + `FLLMDynamicTagConstructorStatString` | 934~959 | `ILLMDynamicTagConstructor` + `FLLMDynamicTagConstructorStatString` |
| `FLLMScopeDynamic` 动态作用域 | 961~999 | `FLLMScopeDynamic` 动态作用域 |
| `FLLMPauseScope` + `FLLMClearScope` + `FLLMScopeFromPtr` | 1001~1038 | `FLLMPauseScope` + `FLLMClearScope` + `FLLMScopeFromPtr` |
| `FLLMTagDeclaration` + `FLLMTagSetAllocationFilter` | 1040~1079 | `FLLMTagDeclaration` + `FLLMTagSetAllocationFilter` |
| `IsEnabled` 内联实现 + `#else` 空宏分支 | 1081~1127 | `IsEnabled` 内联实现 + `#else` 空宏分支 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#define X(macro) \` | X-Macro（列表展开） | `#define LLM_ENUM_GENERIC_TAGS(macro) \` |
| `#define X(a,b) a,` | X-Macro 展开成枚举值 | `#define LLM_ENUM(Enum,Str,Stat,Group,Parent) Enum,` |
| `PREPROCESSOR_JOIN(A, B)` | 宏拼接（生成唯一标识符） | `PREPROCESSOR_JOIN(LLMTagDeclaration_, UniqueName)` |
| `TEXT(#X)` | 字符串化运算符 | `TEXT(#UniqueNameWithUnderscores)` |
| `do { ... } while (false)` | 强制要求分号 | `do { ... } while (false) /* require semicolon */` |
| `static_assert(cond, msg)` | 编译期断言 | `static_assert(ELLMTag::GenericTagCount <= ELLMTag::PlatformTagStart, "...")` |
| `enum class X : uint8` | 强类型枚举 | `enum class ELLMTracker : uint8` |
| `ENUM_CLASS_FLAGS(X)` | 生成位运算 | `ENUM_CLASS_FLAGS(ESizeParams);` |
| `template <typename T, typename... Args> T* New(Args&&...)` | 可变参数模板 | `T* New(ArgsType&&... Args)` |
| `new (Ptr) T(...)` | placement new | `new (Ptr) T(Forward<ArgsType>(Args)...);` |
| `Ptr->~T()` | 显式调析构 | `Ptr->~T();` |
| `UE_DEPRECATED(v, "msg")` | 标记废弃 | `UE_DEPRECATED(5.5, "Use IsEnabled instead.")` |
| `friend class X;` | 友元类 | `friend class FLLMScope;` |
| `std::atomic<bool>` | 原子布尔 | `std::atomic<bool> bConfigurationComplete;` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 1127 行 | 1581 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 725 行 | 725 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| 块注释 | 28 个 `/** */` 多行块 | 27 个单行 `//` 中文注释 | 整块压成一行，保证代码行零差异 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |
| `UE_DEPRECATED` 字符串 | 英文原文 | 保留英文 | API 契约，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`（1581 行）
2. 脚本验收：`codeOurs=725 / codeEngine=725 / diff=0`
3. 确认后下一个：`HAL\LowLevelMemTrackerDefines.h`（1042 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`（1127 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTracker.h`（1581 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\19_LowLevelMemTracker.h_总md.md`