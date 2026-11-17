# 03 — `CoreMinimal.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreMinimal.h`（**183 行 / 148 个 include**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreMinimal.h`
> **✅ 本节可独立编译**——纯头文件，只做 include，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`CoreMinimal.h` 是**写普通 `.h` 时最常 include 的那个头**。它把「写一个 UE 类最少需要的东西」全打包了：基础类型 + 前置声明 + 148 个常用头（容器 / 数学 / 字符串 / 日志 / 委托…）。

**说个场景**：你新建一个 `MyActor.h`，第一行写 `#include "CoreMinimal.h"`，然后就能直接用 `int32` / `FString` / `TArray` / `FVector` / `UE_LOG` / `check()` —— 全都不用再单独 include。

> 🔑 **一句话**：`CoreMinimal.h` = **最小可用集**，183 行 = 基础类型 + 前置声明 + 148 个常用头。

---

## 二、内部结构全景

```
CoreMinimal.h（183 行 / 148 include）
├─ 第 1~3 行    版权头 + #pragma once
├─ 第 5~9 行    ① 底层 include
│    └─ CoreTypes.h（15 行）
├─ 第 11~17 行  ② 前置声明
│    ├─ CoreFwd.h（44 行）
│    ├─ UObject/UObjectHierarchyFwd.h（56 行）
│    └─ Containers/ContainersFwd.h（77 行）
└─ 第 19~183 行 ③ 常用头（148 个）
     ├─ Math              50  ← UnrealMathUtility / Vector / Matrix / Quat / Transform
     ├─ Templates         31  ← UnrealTemplate / SharedPointer / UniquePtr / Function
     ├─ Misc              17  ← AssertionMacros / CString / DateTime / Optional
     ├─ Containers        11  ← Array / Map / Set / UnrealString
     ├─ HAL               11  ← PlatformCrt / PlatformMisc / UnrealMemory
     ├─ Delegates          6  ← Delegate / DelegateBase
     ├─ UObject            6  ← NameTypes / UnrealNames / ObjectVersion
     ├─ Internationalization 5 ← Text / Internationalization
     ├─ Logging            3  ← LogMacros / LogCategory / LogVerbosity
     ├─ GenericPlatform    2  ← GenericPlatformStricmp / GenericPlatformString
     ├─ Algo / Traits / Serialization 各 1
     └─ 散头 3 个（CoreTypes / CoreFwd / CoreGlobals）
```

**8 个兼容块**（`UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_X`）：

| 行 | 版本 | 内容 |
|---|---|---|
| 55~57 | 5_4 | `Misc/Compression.h` |
| 74~76 | 5_4 | `HAL/CriticalSection.h` |
| 81~83 | 5_4 | `Math/Color.h` |
| 97~99 | 5_4 | `Math/ColorList.h` |
| 135~138 | 5_4 | `Math/Plane.h` + `Math/Sphere.h` |
| 166~170 | 5_4 | `Math/InterpCurvePoint.h` + `InterpCurve.h` + `CurveEdInterface.h` |
| 173~176 | 5_4 | `Math/Float16Color.h` + `Vector2DHalf.h` |
| 181~183 | 5_5 | `Templates/IsTriviallyDestructible.h` |

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `CoreMinimal.h` | 无（只 include Core 内部头） | ✅ 能 | 否 |

**3 个前置 include**（实查行数）：

| 文件 | 行数 | 提供什么 |
|---|---|---|
| `CoreTypes.h` | 15 | 基础类型（平台宏 + `int32` 等） |
| `CoreFwd.h` | 44 | 34 个前置声明 |
| `UObject/UObjectHierarchyFwd.h` | 56 | UObject 层级前置声明 |
| `Containers/ContainersFwd.h` | 77 | 容器前置声明 |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整代码（`03_CoreMinimal.h_完整代码.md`）→ 停 → 教 `CoreSharedPCH.h`（263 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `CoreMinimal.h` 的完整 183 行代码（每行都有详细注释）。