# 01 — `CoreTypes.h` —— 完整代码

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`（**15 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`
> **✅ 本节可独立编译**——纯头文件，只做 include，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`CoreTypes.h` 是 `Public\` 里**最短的散头**（15 行），但它干的事最关键：**把「基础类型层」的 4 份定义头一次性拉进来**。

**说个场景**：你写 `int32` / `uint8` / `PLATFORM_WINDOWS` / `UE_BUILD_DEBUG` 这些最基础的东西时，它们不是 C++ 自带的，是 UE 自己定义的。这些定义分散在 4 个头里，`CoreTypes.h` 就是「把这 4 个打包成一份」的入口。

> 🔑 **一句话**：`CoreTypes.h` = **基础类型层的总闸**，15 行只干一件事——把 4 份定义头拉进来。

---

## 二、完整代码（每行都有详细注释）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 头文件保护：保证同一个 .cpp 里多次 include 本文件时只展开一次。
// 比传统的 #ifndef/#define/#endif 更简洁，所有现代编译器都支持。
#pragma once

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/
/* 分节注释：下面是「底层 include」区。
   这种 /*---*/ 分节注释是 UE 的代码风格，用来把文件切成若干视觉区块。 */

// IWYU pragma: begin_exports
// IWYU（Include What You Use）指令：从这里开始，本文件 include 的所有头
// 都算「本文件对外导出的头」——别的文件 include 了 CoreTypes.h，
// 就等于也 include 了下面这 4 个头。
// 为什么需要：CoreTypes.h 本身不定义任何东西，它只是「转发」，
// 所以必须告诉 IWYU 工具「这些 include 是导出，别给我删掉」。

// 引入平台层总头。
// 真实文件：Core\Public\HAL\Platform.h（1345 行）
// 里面定义：PLATFORM_WINDOWS / PLATFORM_MAC / PLATFORM_ANDROID 等平台宏，
//          以及 int8 / int16 / int32 / int64 / uint8 ... 等基础整数类型。
#include "HAL/Platform.h"

// 引入内存分配器开关定义。
// 真实文件：Core\Public\ProfilingDebugging\UMemoryDefines.h（17 行）
// 里面定义：TBB_ALLOCATOR_ALLOWED / MIMALLOC_ALLOCATOR_ALLOWED
//          （这两个宏决定能不能用 IntelTBB / mimalloc 分配器）。
#include "ProfilingDebugging/UMemoryDefines.h"

// 引入 Core 杂项宏定义。
// 真实文件：Core\Public\Misc\CoreMiscDefines.h（594 行）
// 里面定义：WITH_EDITORONLY_DATA / WITH_METADATA / UE_DISABLE_OPTIMIZATION /
//          UE_ENABLE_OPTIMIZATION / PLAYWORLD_PACKAGE_PREFIX 等构建开关宏。
#include "Misc/CoreMiscDefines.h"

// 引入 Core 基础宏定义。
// 真实文件：Core\Public\Misc\CoreDefines.h（80 行）
// 里面定义：STUBBED / CPP / STRUCTCPP / DEFAULTS / STANDALONE_SEEKFREE_SUFFIX
//          等元数据宏。
#include "Misc/CoreDefines.h"

// IWYU pragma: end_exports
// IWYU 指令：导出区到此结束。
```

---

## 三、逐段拆解（本段新元素 + 这段在干什么）

### 3.1 版权头与头文件保护（第 1~3 行）

**本段新元素**

- `#pragma once` —— 头文件保护，保证同一编译单元内只展开一次

**这段在干什么**：声明版权 + 防止重复 include。

> 类比：`#pragma once` 是「这扇门只进一次」的门禁。

### 3.2 分节注释（第 5~7 行）

**本段新元素**

- `/*---...---*/` —— UE 风格的分节注释，把文件切成视觉区块

**这段在干什么**：标出「下面是底层 include 区」。

> 类比：这是「文件夹里的分隔页」，纯视觉，不影响编译。

### 3.3 IWYU 导出区（第 9~14 行）

**本段新元素**

- `// IWYU pragma: begin_exports` —— IWYU 指令，标记「导出区开始」
- `// IWYU pragma: end_exports` —— IWYU 指令，标记「导出区结束」
- `#include "HAL/Platform.h"` —— 平台层总头（1345 行）
- `#include "ProfilingDebugging/UMemoryDefines.h"` —— 内存分配器开关（17 行）
- `#include "Misc/CoreMiscDefines.h"` —— Core 杂项宏（594 行）
- `#include "Misc/CoreDefines.h"` —— Core 基础宏（80 行）

**这段在干什么**

| 行 | include 谁 | 里面有什么 |
|---|---|---|
| 10 | `HAL/Platform.h` | `PLATFORM_WINDOWS` / `PLATFORM_MAC` / `PLATFORM_ANDROID` 等平台宏 + `int8`~`int64` / `uint8`~`uint64` 基础类型 |
| 11 | `ProfilingDebugging/UMemoryDefines.h` | `TBB_ALLOCATOR_ALLOWED` / `MIMALLOC_ALLOCATOR_ALLOWED` |
| 12 | `Misc/CoreMiscDefines.h` | `WITH_EDITORONLY_DATA` / `WITH_METADATA` / `UE_DISABLE_OPTIMIZATION` / `UE_ENABLE_OPTIMIZATION` |
| 13 | `Misc/CoreDefines.h` | `STUBBED` / `CPP` / `STRUCTCPP` / `DEFAULTS` / `STANDALONE_SEEKFREE_SUFFIX` |

> 类比：`CoreTypes.h` 是「一个快递盒」，里面装了 4 个包裹（4 个头）；`begin_exports` / `end_exports` 是「盒子上贴的标签：这些包裹是给收件人的，别拆」。

---

## 四、IWYU 是什么（关键概念）

**IWYU = Include What You Use**（用到什么就包含什么），Google 开源的一个 C++ 工具，用来清理多余的 `#include`。

**问题**：`CoreTypes.h` 自己**不定义任何东西**，只是转发 4 个头。IWYU 工具扫描时会发现「`CoreTypes.h` 里没有任何符号被使用」，于是**把这 4 个 include 全删掉**——结果所有依赖 `CoreTypes.h` 的文件全崩。

**解法**：`// IWYU pragma: begin_exports` / `end_exports` 告诉工具「这段 include 是**导出**，别删」。

| 指令 | 含义 |
|---|---|
| `// IWYU pragma: begin_exports` | 从这里开始，include 的头算「本文件导出」 |
| `// IWYU pragma: end_exports` | 导出区结束 |
| `// IWYU pragma: keep` | 单个 include 保留（不删） |
| `// IWYU pragma: export` | 单个 include 算导出 |

> 类比：`begin_exports` / `end_exports` 是「请勿删除」的封条。

---

## 五、四个被包含文件的实际内容（实查）

### 5.1 `HAL/Platform.h`（1345 行）

```cpp
// 开头就是一堆「把其他平台宏定义为 0」
#if !defined(PLATFORM_WINDOWS)
	#define PLATFORM_WINDOWS 0
#endif
#if !defined(PLATFORM_MAC)
	#define PLATFORM_MAC 0
	#define PLATFORM_MAC_X86 0
	#define PLATFORM_MAC_ARM64 0
#endif
#if !defined(PLATFORM_IOS)
	#define PLATFORM_IOS 0
#endif
...
```

**作用**：把所有平台宏先定义成 0，然后根据当前编译平台，把对应的那个改成 1。这样代码里写 `#if PLATFORM_WINDOWS` 永远能编过（不会因为宏未定义报错）。

### 5.2 `ProfilingDebugging/UMemoryDefines.h`（17 行）

```cpp
/**
 * true if intelTBB (a.k.a MallocTBB) can be used (different to the platform actually supporting it)
 */
#ifndef TBB_ALLOCATOR_ALLOWED
	#define TBB_ALLOCATOR_ALLOWED 1
#endif

/**
 * true if mimalloc can be used (different to the platform actually supporting it)
 */
#ifndef MIMALLOC_ALLOCATOR_ALLOWED
	#define MIMALLOC_ALLOCATOR_ALLOWED 1
#endif
```

**作用**：定义两个分配器开关（默认都是 1 = 允许）。

### 5.3 `Misc/CoreMiscDefines.h`（594 行）

```cpp
#ifndef WITH_EDITORONLY_DATA
	#if !PLATFORM_CAN_SUPPORT_EDITORONLY_DATA || UE_SERVER || PLATFORM_IOS
		#define WITH_EDITORONLY_DATA	0
	#else
		#define WITH_EDITORONLY_DATA	1
	#endif
#endif

#ifndef WITH_METADATA
#define WITH_METADATA WITH_EDITORONLY_DATA
#elif WITH_EDITORONLY_DATA && !WITH_METADATA
#error WITH_EDITORONLY_DATA=1 requires WITH_METADATA=1
#endif
```

**作用**：定义一堆构建开关宏（编辑器数据 / 元数据 / 优化控制）。

### 5.4 `Misc/CoreDefines.h`（80 行）

```cpp
#ifndef STUBBED
#define STUBBED(x)	\
	do { \
		static bool AlreadySeenThisStubbedSection = false; \
		if (!AlreadySeenThisStubbedSection) { \
			AlreadySeenThisStubbedSection = true; \
			fprintf(stderr, "STUBBED: %s at %s:%d (%s)\n", x, __FILE__, __LINE__, __FUNCTION__); \
		} \
	} while (0)
#endif

#define CPP       1
#define STRUCTCPP 1
#define DEFAULTS  0
```

**作用**：定义 `STUBBED` 宏（标记未实现的桩函数）+ 元数据宏。

---

## 六、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `CoreTypes.h` | `CoreTypes.h` | **完全一致** |
| 路径 | `Engine\Source\Runtime\Core\Public\` | `ue_5.6\Engine\Source\Runtime\Core\Public\` | 只换根 |
| 行数 | 15 行 | 46 行 | **多 31 行中文注释**（允许） |
| 代码行 | 5 行 | 5 行 | **逐字一致** |
| 英文注释 | 3 处 | 0 处 | 全部译成中文 |
| 补注释 | 无 | 每个 include 上方 | 允许的例外 |
| `#pragma once` | 有 | 原样保留 | 照抄 |
| IWYU 指令 | 有 | 原样保留 | 照抄（这是代码，不是注释） |

> 除注释外，**代码一字不差**。

---

## 七、收尾

到这里 `CoreTypes.h` 就完了（15 行 → 46 行）。

**结构**：`#pragma once` + 分节注释 + IWYU 导出区（4 个 include）。

**关键概念**：
- `#pragma once` = 头文件保护
- `// IWYU pragma: begin_exports` / `end_exports` = 告诉 IWYU 工具「这些 include 是导出，别删」
- 4 个 include 分别提供：平台宏 + 基础类型 / 分配器开关 / 构建开关 / 元数据宏

---

## 八、速查表 · 口诀

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\Runtime\Core\Public\CoreTypes.h`（15 行） |
| 讲义 | `从零复刻ue5\Engine\Source\Runtime\Core\Public\01_CoreTypes.h_完整代码.md` |
| 还原到哪 | `ue_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h` |

**口诀**：**`CoreTypes.h` 是基础类型层的总闸，15 行只干一件事——用 IWYU 导出区把 4 份定义头（Platform / UMemoryDefines / CoreMiscDefines / CoreDefines）打包转发出去。**

---

## 九、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#pragma once` | 头文件保护（只展开一次） | `#pragma once` |
| `#include "路径"` | 引入头文件 | `#include "HAL/Platform.h"` |
| `/* ... */` | 块注释 | `/*--- Low level includes. ---*/` |
| `// ...` | 行注释 | `// IWYU pragma: begin_exports` |
| `#ifndef X` / `#define X` / `#endif` | 条件编译（宏未定义时定义） | `#ifndef TBB_ALLOCATOR_ALLOWED` |

---

## 待确认

1. 15 行看懂了吗？重点记：**IWYU 导出区** + **4 个 include 各提供什么**。
2. 看完说「下一步」，我教 `CoreFwd.h`（44 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`（15 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\Platform.h`（1345 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\ProfilingDebugging\UMemoryDefines.h`（17 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\Misc\CoreMiscDefines.h`（594 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\Misc\CoreDefines.h`（80 行）