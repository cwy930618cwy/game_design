# 02 — `CoreFwd.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreFwd.h`（**44 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreFwd.h`
> **✅ 本节可独立编译**——纯头文件，只做前置声明，不产生编译单元。

---

## 一、这个文件解决什么问题（为什么）

`CoreFwd.h` 是「**前置声明（forward declaration）**」的集合。它**不定义任何类**，只写 `class FName;` 这种「我先告诉你这个名字是个类，具体长什么样后面再说」。

**说个场景**：你写一个头文件，里面有个函数参数是 `FArchive&`。如果直接 `#include "Serialization/Archive.h"`，会把整个 Archive 头（几千行）拉进来，编译变慢。但如果只写 `class FArchive;`（前置声明），编译器知道「有这么个类」，就够用了——因为**引用和指针不需要完整定义**。

> 🔑 **一句话**：`CoreFwd.h` = **前置声明总表**，44 行只写「这些名字是类 / 结构体 / 枚举」，不写它们长什么样。

---

## 二、内部结构全景

```
CoreFwd.h（44 行）
├─ 第 1~3 行    版权头 + #pragma once
├─ 第 5~8 行    4 个 include
│    ├─ CoreTypes.h                        ← 基础类型（uint8 要用）
│    ├─ Containers/ContainersFwd.h         ← 容器前置声明（77 行）
│    ├─ Math/MathFwd.h                     ← 数学前置声明（143 行）
│    └─ UObject/UObjectHierarchyFwd.h      ← UObject 层级前置声明（56 行）
├─ 第 10~17 行  Basic types（7 个）
│    ├─ class FName / FExec / FArchive / FOutputDevice / FFeedbackContext
│    └─ struct FDateTime / FGuid
├─ 第 19 行     Math 分节注释（指向 MathFwd.h）
├─ 第 21~38 行  Misc（17 个）
│    ├─ struct FResourceSizeEx / FGenericMemoryStats / FManifestContext
│    ├─ class IConsoleVariable / FRunnableThread / FEvent / IPlatformFile / FMalloc
│    ├─ class IFileHandle / FAutomationTestBase / FSHAHash / FScriptArray
│    ├─ class FThreadSafeCounter / IConsoleObject / FConfigFile / FConfigSection
│    └─ enum class EModuleChangeReason
└─ 第 40~44 行  Text（4 个）
     ├─ class FText / FTextFilterString
     └─ enum class ETextFilterTextComparisonMode : uint8 / ETextFilterComparisonOperation : uint8
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `CoreFwd.h` | 无（只 include 4 个 Core 内部头） | ✅ 能 | 否 |

**4 个被包含文件**（实查行数）：

| 文件 | 行数 | 提供什么 |
|---|---|---|
| `CoreTypes.h` | 15 | 基础类型（`uint8` 等） |
| `Containers/ContainersFwd.h` | 77 | `FString` / `FAnsiString` / `FUtf8String` / 分配器 |
| `Math/MathFwd.h` | 143 | `FColor` / `FLinearColor` / `TVector<T>` / `TQuat<T>` / `TMatrix<T>` |
| `UObject/UObjectHierarchyFwd.h` | 56 | `UObject` / `UClass` / `UStruct` / `UFunction` / `FProperty` |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整代码（`02_CoreFwd.h_完整代码.md`）→ 停 → 教 `CoreMinimal.h`（183 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `CoreFwd.h` 的完整 44 行代码（每行都有详细注释）。