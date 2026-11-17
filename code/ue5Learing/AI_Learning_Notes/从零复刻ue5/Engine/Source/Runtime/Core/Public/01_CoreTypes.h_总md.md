# 01 — `CoreTypes.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`（**15 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`
> **✅ 本节可独立编译**——纯头文件，只做 include，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`CoreTypes.h` 是 `Public\` 里**最短的散头**（15 行），但它干的事最关键：**把「基础类型层」的 4 份定义头一次性拉进来**。

**说个场景**：你写 `int32` / `uint8` / `PLATFORM_WINDOWS` / `UE_BUILD_DEBUG` 这些最基础的东西时，它们不是 C++ 自带的，是 UE 自己定义的。这些定义分散在 4 个头里，`CoreTypes.h` 就是「把这 4 个打包成一份」的入口。

> 🔑 **一句话**：`CoreTypes.h` = **基础类型层的总闸**，15 行只干一件事——把 4 份定义头拉进来。

---

## 二、内部结构全景

```
CoreTypes.h（15 行）
├─ 第 1 行    版权头
├─ 第 3 行    #pragma once（头文件保护）
├─ 第 5~7 行  分节注释「Low level includes」
└─ 第 9~14 行 IWYU 导出区
     ├─ 第 9 行   // IWYU pragma: begin_exports
     ├─ 第 10 行  #include "HAL/Platform.h"                        ← 平台宏 + 基础整数类型
     ├─ 第 11 行  #include "ProfilingDebugging/UMemoryDefines.h"   ← 分配器开关
     ├─ 第 12 行  #include "Misc/CoreMiscDefines.h"                ← 构建开关宏
     ├─ 第 13 行  #include "Misc/CoreDefines.h"                    ← 元数据宏
     └─ 第 14 行  // IWYU pragma: end_exports
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `CoreTypes.h` | 无（只 include 4 个 Core 内部头） | ✅ 能 | 否 |

**4 个被包含文件**（实查行数）：

| 文件 | 行数 | 提供什么 |
|---|---|---|
| `HAL/Platform.h` | 1345 | `PLATFORM_WINDOWS` / `PLATFORM_MAC` / `PLATFORM_ANDROID` 等平台宏 + `int8`~`int64` / `uint8`~`uint64` |
| `ProfilingDebugging/UMemoryDefines.h` | 17 | `TBB_ALLOCATOR_ALLOWED` / `MIMALLOC_ALLOCATOR_ALLOWED` |
| `Misc/CoreMiscDefines.h` | 594 | `WITH_EDITORONLY_DATA` / `WITH_METADATA` / `UE_DISABLE_OPTIMIZATION` / `UE_ENABLE_OPTIMIZATION` |
| `Misc/CoreDefines.h` | 80 | `STUBBED` / `CPP` / `STRUCTCPP` / `DEFAULTS` / `STANDALONE_SEEKFREE_SUFFIX` |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整代码（`01_CoreTypes.h_完整代码.md`）→ 停 → 教 `CoreFwd.h`（44 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `CoreTypes.h` 的完整 15 行代码（每行都有详细注释）。