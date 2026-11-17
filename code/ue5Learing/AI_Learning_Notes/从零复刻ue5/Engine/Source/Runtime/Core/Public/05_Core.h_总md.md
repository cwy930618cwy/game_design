# 05 — `Core.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\Core.h`（**319 行 / 294 个 include**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\Core.h`
> **✅ 本节可独立编译**——纯头文件，只做 include，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`Core.h` 是 **Core 模块的「总入口」**——它把 Core 里**几乎所有头**（294 个）一次性再导出一遍。写代码时 `#include "Core.h"` 就等于「把整个 Core 都拉进来」。

**和前面几个头的区别**：

| 文件 | include 数 | 定位 |
|---|---|---|
| `CoreTypes.h` | 4 | 基础类型层（最小） |
| `CoreFwd.h` | 4 + 34 个前置声明 | 前置声明 |
| `CoreMinimal.h` | 148 | 最小可用集（写普通 `.h` 用） |
| `CoreSharedPCH.h` | 252 | 预编译头（编译加速） |
| **`Core.h`** | **294** | **模块总入口（全量）** |

> 🔑 **一句话**：`Core.h` = **Core 的「全家桶」**，294 个头一次全给你；但它**不推荐**在头文件里用（会拖慢编译），只在 `.cpp` 里用。

---

## 二、内部结构全景

```
Core.h（319 行 / 294 include）
├─ 第 1~3 行    版权头 + #pragma once
├─ 第 5~6 行    单体头警告
│    ├─ #include "Misc/MonolithicHeaderBoilerplate.h"（11 行）
│    └─ MONOLITHIC_HEADER_BOILERPLATE()  ← 展开成 COMPILE_WARNING
├─ 第 8~100 行  ① 底层 include（89 个）
│    ├─ CoreMinimal.h（148 个）
│    ├─ 平台层：HAL 32 / GenericPlatform 15
│    ├─ 模板：Templates 37 / Traits 1
│    └─ 基础工具：Misc / Logging / Math 工具
├─ 第 102~314 行 ② 正式 include（200 个）
│    ├─ Misc              64  ← App / Paths / ConfigCacheIni / OutputDevice*（9 个）
│    ├─ Math              52  ← Vector / Matrix / Quat / Transform / SHMath
│    ├─ Containers        28  ← Array / Map / Set / Queue / Ticker
│    ├─ UObject           19  ← NameTypes + 13 个 *ObjectVersion.h
│    ├─ Serialization     16  ← Archive / MemoryReader / MemoryWriter
│    ├─ Internationalization 8
│    ├─ Delegates          6
│    ├─ Logging            6
│    ├─ ProfilingDebugging 3
│    ├─ Stats              2
│    └─ Async / Algo / Features 各 1
└─ 第 317~319 行 兼容块 5_5（IsTriviallyDestructible.h）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `Core.h` | 无（只 include Core 内部头） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 行数 | 作用 |
|---|---|---|
| `Misc/MonolithicHeaderBoilerplate.h` | 11 | 单体头警告宏 |
| `CoreMinimal.h` | 183 | 最小可用集（148 个） |
| `CoreGlobals.h` | 854 | 全局量 |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整代码（`05_Core.h_完整代码.md`）→ 停 → 教 `PixelFormat.h`（527 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `Core.h` 的完整 319 行代码（每行都有详细注释）。