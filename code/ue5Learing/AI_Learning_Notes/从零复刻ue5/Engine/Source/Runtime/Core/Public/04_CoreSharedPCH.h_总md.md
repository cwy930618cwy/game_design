# 04 — `CoreSharedPCH.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`（**263 行 / 252 个 include**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`
> **✅ 本节可独立编译**——纯头文件，只做 include，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`CoreSharedPCH.h` 是 **Core 模块对外提供的「共享预编译头」**。`Core.Build.cs` 第 15 行写着：

```csharp
SharedPCHHeaderFile = "Public/CoreSharedPCH.h";
```

**说个场景**：Core 有 685 个 `.cpp`，每个都要 include 一大堆头。不预编译 → 685 次重复解析同样的 252 个头，编译要几小时；预编译 → 解析一次存成二进制快照（`.pch` 文件），685 个 `.cpp` 各吃一次快照，几分钟搞定。

**和 `CoreMinimal.h` 的区别**：

| 项 | `CoreMinimal.h` | `CoreSharedPCH.h` |
|---|---|---|
| include 数 | 148 | **252** |
| 用途 | 写普通 `.h` 时 include | **预编译头**（编译加速） |
| 谁用 | 所有模块的头文件 | 只有 PCH 机制用 |
| 内容 | 「最小可用集」 | 「Core 全部常用头」 |

> 🔑 **一句话**：`CoreSharedPCH.h` = **Core 的「大面团」**，252 个头提前和好，685 个 `.cpp` 各揪一块用。

---

## 二、内部结构全景

```
CoreSharedPCH.h（263 行 / 252 include）
├─ 第 1~3 行    版权头 + #pragma once
├─ 第 5 行      分节注释「From Core:」
├─ 第 6~254 行  252 个 include（按字母序）
│    ├─ Math              55  ← Vector / Matrix / Quat / Transform / RandomStream / SHMath
│    ├─ Misc              49  ← App / Paths / ConfigCacheIni / FileHelper / MemStack
│    ├─ Templates         37  ← SharedPointer / UniquePtr / Function / ValueOrError
│    ├─ HAL               23  ← FileManager / IConsoleManager / RunnableThread / CriticalSection
│    ├─ Containers        22  ← Array / Map / Set / Queue / Ticker / StaticArray
│    ├─ GenericPlatform   12  ← GenericPlatformFile / Process / Time
│    ├─ Serialization      9  ← Archive / MemoryReader / MemoryWriter / BitReader
│    ├─ UObject            8  ← NameTypes / UnrealNames / ObjectVersion
│    ├─ Internationalization 7 ← Text / Internationalization / TextLocalizationManager
│    ├─ Delegates          6  ← Delegate / DelegateBase / MulticastDelegateBase
│    ├─ Async              4  ← AsyncWork / Future / ParallelFor / TaskGraphInterfaces
│    ├─ Logging            4  ← LogMacros / LogCategory / LogVerbosity / TokenizedMessage
│    ├─ Modules            3  ← ModuleInterface / ModuleManager / ModuleBoilerplate
│    ├─ ProfilingDebugging 3  ← Histogram / ProfilingHelpers / ResourceSize
│    ├─ Stats              2  ← Stats / StatsMisc
│    ├─ Features           2  ← IModularFeature / IModularFeatures
│    ├─ Algo / Traits      各 1
│    └─ 散头 4 个（CoreFwd / CoreGlobals / CoreMinimal / CoreTypes）
├─ 第 256~258 行 兼容块 5_5（IsTriviallyDestructible.h）
└─ 第 260~263 行 兼容块 5_6（BasicMathExpressionEvaluator.h + ExpressionParserTypes.h）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `CoreSharedPCH.h` | 无（只 include Core 内部头） | ✅ 能 | 否 |

**被 `Core.Build.cs` 引用**：第 15 行 `SharedPCHHeaderFile = "Public/CoreSharedPCH.h";`

**和 `CorePrivatePCH.h` 的关系**：

| 项 | `CoreSharedPCH.h` | `CorePrivatePCH.h`（11 行） |
|---|---|---|
| include 数 | 252 | 6（1 个共享 + 5 个额外） |
| 谁用 | 全引擎所有依赖 Core 的模块 | 只有 Core 自己的 `.cpp` |
| 关系 | — | 第 5 行 `#include "CoreSharedPCH.h"` |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整代码（`04_CoreSharedPCH.h_完整代码.md`）→ 停 → 教 `Core.h`（319 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `CoreSharedPCH.h` 的完整 263 行代码（每行都有详细注释）。