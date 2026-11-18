# 20 — `HAL/LowLevelMemTrackerDefines.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTrackerDefines.h`（**24 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\LowLevelMemTrackerDefines.h`
> **✅ 本节可独立编译**——纯宏定义文件，只 include 1 个头。

---

## 一、这个文件解决什么问题（为什么）

`LowLevelMemTracker.h` 第 6 行 include 了 `LowLevelMemTrackerDefines.h`，用它拿到 `LLM_ENABLED_IN_CONFIG`。**为什么单独拆一个文件**？因为：

> 本头文件可能被 **C 代码**包含，所以不能包含任何 C++ 代码或含 C++ 代码的头文件。

`LowLevelMemTracker.h` 里有 `class` / `template` / `namespace`，C 编译器看不懂。但某些 C 编译单元（比如平台层的 `.c` 文件）也需要知道「LLM 开没开」。所以把**纯宏配置**单独拆出来。

**它只有 2 个宏**：

| 宏 | 作用 |
|---|---|
| `ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST` | Test 构建是否允许 LLM（默认 1） |
| `LLM_ENABLED_IN_CONFIG` | 配置里是否启用 LLM（派生宏） |

**核心设计 ①：`ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST`**（第 10~12 行）

```cpp
#ifndef ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST
    #define ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST 1   // Test 构建默认启用
#endif
```

**为什么 Test 也开**：自动化性能测试需要抓 LLM 数据，所以 Test 构建默认启用。

**核心设计 ②：`LLM_ENABLED_IN_CONFIG` 的三重条件**（第 19~24 行）

```cpp
#ifndef LLM_ENABLED_IN_CONFIG
    #define LLM_ENABLED_IN_CONFIG ( \
        !UE_BUILD_SHIPPING && (!UE_BUILD_TEST || ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST) && \
        WITH_ENGINE \
    )
#endif
```

拆开看：

| 条件 | 含义 |
|---|---|
| `!UE_BUILD_SHIPPING` | **Shipping 构建永远不开**（性能优先） |
| `!UE_BUILD_TEST \|\| ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST` | Test 构建看开关（默认开） |
| `WITH_ENGINE` | 必须带引擎（纯程序不需要 LLM） |

**核心设计 ③：为什么不能放平台头文件**（第 14~17 行）

```cpp
// LLM_ENABLED_IN_CONFIG 只能在这里或构建环境里定义。
// 不能在平台头文件里定义，因为它会被 C 语言编译单元包含，而那些单元无法包含平台头。
```

平台头文件（如 `Windows/WindowsPlatform.h`）里有 C++ 代码，C 编译单元 include 不了。所以这个宏必须在**纯 C 兼容**的头里定义。

**说个场景**：

```
构建配置          UE_BUILD_SHIPPING  UE_BUILD_TEST  WITH_ENGINE  →  LLM
Development       false              false          true         →  ✅ 开
Test              false              true           true         →  ✅ 开（默认）
Shipping          true               false          true         →  ❌ 关
Program (无引擎)  false              false          false        →  ❌ 关
```

> 🔑 **一句话**：`LowLevelMemTrackerDefines.h` = **LLM 的「总闸」**，纯 C 兼容，决定编译期开不开 LLM。

---

## 二、内部结构全景

```
LowLevelMemTrackerDefines.h（24 行）
├─ 第 1~3 行      版权头 + #pragma once
├─ 第 5~6 行      文件说明（为什么不能含 C++）
├─ 第 8 行        include "Misc/Build.h"
├─ 第 10~12 行    ① ALLOW_LOW_LEVEL_MEM_TRACKER_IN_TEST（默认 1）
├─ 第 14~17 行    ② 说明：LLM_ENABLED_IN_CONFIG 只能在这里或构建环境定义
└─ 第 19~24 行    ③ LLM_ENABLED_IN_CONFIG（三重条件）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/LowLevelMemTrackerDefines.h` | 无（纯宏） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `Misc/Build.h` | `UE_BUILD_SHIPPING` / `UE_BUILD_TEST` / `WITH_ENGINE` |

**注意**：`Misc/Build.h` 本身也是纯宏头（C 兼容），所以整条链都能被 C 编译单元包含。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\33_LowLevelMemTrackerDefines.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `MemoryBase.h`（约 2.5KB）。