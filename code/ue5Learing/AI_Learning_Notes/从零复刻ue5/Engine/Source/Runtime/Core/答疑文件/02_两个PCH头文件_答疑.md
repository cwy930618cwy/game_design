# 02 — 两个 PCH 头文件答疑（`PrivatePCHHeaderFile` / `SharedPCHHeaderFile`）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs` 第 13、15 行
> 对应真实文件：`Core\Private\CorePrivatePCH.h`（**11 行**）、`Core\Public\CoreSharedPCH.h`（**263 行 / 252 个 include**）
> **✅ 本节可独立编译**——两个都是纯头文件，不产生编译单元。

---

## 一、这两行在干什么（为什么）

`Core.Build.cs` 第 13、15 行：

```csharp
PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";
SharedPCHHeaderFile  = "Public/CoreSharedPCH.h";
```

这两行是**告诉 UBT「预编译头（PCH）用哪个文件」**。PCH = Precompiled Header，把一大堆常用头文件**提前编译成二进制快照**，之后每个 `.cpp` 直接吃快照，不用再一行行解析那 252 个 `#include`。

**说个场景**：Core 模块有 685 个 `.cpp`，每个都要 `#include "CoreMinimal.h"` 等一大堆头。不预编译 → 685 次重复解析同样的头，编译要几小时；预编译 → 解析一次存成快照，685 个 `.cpp` 各吃一次快照，几分钟搞定。

> 🔑 **一句话**：`SharedPCHHeaderFile` = **给全引擎共用的「大面团」**；`PrivatePCHHeaderFile` = **Core 自己额外加的「小面团」**（在大面团基础上再揉进 5 个头）。

---

## 二、两个文件是什么（实读）

### 2.1 `Public/CoreSharedPCH.h`（263 行 / 252 个 include）

```
CoreSharedPCH.h
├─ 第 1~3 行：版权头 + #pragma once
├─ 第 5 行：// From Core:（注释，说明下面全是 Core 自己的头）
├─ 第 6~255 行：252 个 #include，按字母序排
│    ├─ Math              55 个  ← 向量 / 矩阵 / 四元数 / 曲线
│    ├─ Misc              49 个  ← 断言 / 路径 / 配置 / 时间
│    ├─ Templates         37 个  ← 类型萃取 / 智能指针 / Function
│    ├─ HAL               23 个  ← 平台层
│    ├─ Containers        22 个  ← TArray / TMap / TSet / FString
│    ├─ GenericPlatform   12 个
│    ├─ Serialization      9 个
│    ├─ UObject            8 个
│    ├─ Internationalization 7 个
│    ├─ Delegates          6 个
│    ├─ Async / Logging    各 4 个
│    ├─ Modules / ProfilingDebugging 各 3 个
│    ├─ Stats / Features   各 2 个
│    ├─ Algo / Traits      各 1 个
│    └─ 4 个散头：CoreFwd.h / CoreGlobals.h / CoreMinimal.h / CoreTypes.h
├─ 第 256~258 行：#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
│                   └─ Templates/IsTriviallyDestructible.h
└─ 第 260~263 行：#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
                    ├─ Math/BasicMathExpressionEvaluator.h
                    └─ Misc/ExpressionParserTypes.h
```

> 最后两个 `#if` 块是**兼容旧 include 顺序**的补丁：老代码依赖这些头被间接带进来，5.5 / 5.6 把它们从主列表移走了，但为了不破坏老代码，用宏开关兜着。

### 2.2 `Private/CorePrivatePCH.h`（11 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once

#include "CoreSharedPCH.h"

#include "HAL/PlatformAffinity.h"
#include "HAL/RunnableThread.h"
#include "Internationalization/Culture.h"
#include "Misc/AutomationTest.h"
#include "Templates/ValueOrError.h"
```

| 行 | 干什么 |
|---|---|
| 第 5 行 `#include "CoreSharedPCH.h"` | **先吃共享大面团**（252 个头全进来） |
| 第 7~11 行 5 个头 | **再额外加 5 个**：平台亲和性 / 可运行线程 / 文化本地化 / 自动化测试 / ValueOrError |

> 🔑 关键：`CorePrivatePCH.h` **不是独立的一套**，它是「共享 PCH + 5 个额外头」。所以 Core 的 `.cpp` 吃私有 PCH 时，等于同时拿到了共享 PCH 的全部内容。

---

## 三、UBT 怎么用这两个字段（实查 `UEBuildModuleCPP.cs`）

| 位置 | 代码 | 作用 |
|---|---|---|
| `ModuleRules.cs:793` | `public string? PrivatePCHHeaderFile { get; set; }` | 字段定义 |
| `ModuleRules.cs:799` | `public string? SharedPCHHeaderFile { get; set; }` | 字段定义 |
| `UEBuildModuleCPP.cs:1131~1134` | `if (Rules.PrivatePCHHeaderFile != null) PrivatePchFileItem = ...` | 把私有 PCH 头登记为 FileItem |
| `UEBuildModuleCPP.cs:1137~1140` | `if (Rules.SharedPCHHeaderFile != null) SharedPchFileItem = ...` | 把共享 PCH 头登记为 FileItem |
| `UEBuildModuleCPP.cs:1151` | `if (HeaderFileItem == PrivatePchFileItem \|\| HeaderFileItem == SharedPchFileItem) continue;` | **IWYU 时跳过 PCH 头**（PCH 头不参与 include 清理） |
| `UEBuildModuleCPP.cs:1366~1382` | `CreateSharedPCHTemplate(...)` | 用 `SharedPCHHeaderFile` 建共享 PCH 模板 |
| `UEBuildModuleCPP.cs:1956~1958` | `if (Rules.PrivatePCHHeaderFile != null && (PCHUsage == NoSharedPCHs \|\| UseExplicitOrSharedPCHs)) CreatePrivatePCH(...)` | 用 `PrivatePCHHeaderFile` 建私有 PCH |
| `UEBuildModuleCPP.cs:296` | `if (PCHUsage == NoSharedPCHs && PrivatePCHHeaderFile == null)` | 警告：选了 NoSharedPCHs 却没给私有 PCH |
| `UEBuildModuleCPP.cs:305~307` | `logger.LogWarning("{Module} module must specify an explicit precompiled header for PCHUsage {PCHUsage} (eg. PrivatePCHHeaderFile = \"Private/{Module}PrivatePCH.h\")")` | 警告原文：**必须指定显式 PCH** |

> 注意第 305~307 行的警告文案：`Private/{Module}PrivatePCH.h` —— 这就是 UE 的**命名约定**，`Core` 模块 → `CorePrivatePCH.h`，`Engine` 模块 → `EnginePrivatePCH.h`。

---

## 四、两者关系（树形图）

```
Core.Build.cs
├─ 第 13 行 PrivatePCHHeaderFile ──> Private/CorePrivatePCH.h（11 行）
│                                      ├─ #include "CoreSharedPCH.h"   ← 先吃共享
│                                      └─ 5 个额外头（PlatformAffinity / RunnableThread /
│                                                    Culture / AutomationTest / ValueOrError）
└─ 第 15 行 SharedPCHHeaderFile  ──> Public/CoreSharedPCH.h（263 行 / 252 include）
                                       ├─ Math 55 / Misc 49 / Templates 37 / HAL 23 / Containers 22 ...
                                       └─ 2 个 #if 兼容块（5.5 / 5.6 旧 include 顺序）

UBT 消费方
├─ UEBuildModuleCPP.cs:1366 CreateSharedPCHTemplate()  ← 用 SharedPCHHeaderFile
├─ UEBuildModuleCPP.cs:1956 CreatePrivatePCH()         ← 用 PrivatePCHHeaderFile
└─ UEBuildModuleCPP.cs:1151 IWYU 时跳过这两个头
```

---

## 五、为什么 Core 两个都要设（关键）

`ModuleRules.PCHUsageMode` 有 5 档（`ModuleRules.cs:193~219`），Core 用的是默认推导结果 `UseExplicitOrSharedPCHs`：

| 情况 | UBT 行为 |
|---|---|
| 设了 `PrivatePCHHeaderFile` | 用**私有 PCH**（`CorePrivatePCH.h`），不吃共享 |
| 没设 `PrivatePCHHeaderFile` | 用**共享 PCH**（`CoreSharedPCH.h`） |

> 但 Core **两个都设了**——因为 `CorePrivatePCH.h` 第 5 行自己 `#include "CoreSharedPCH.h"`，所以「私有 PCH」实际上 = 共享 PCH + 5 个头。这样 Core 的 `.cpp` 既能拿到全引擎通用的 252 个头，又能拿到自己额外需要的 5 个。

**为什么别的模块不设 `SharedPCHHeaderFile`？** 因为共享 PCH 只该由「被大量模块包含的头文件」提供（`ModuleRules.cs:796~798` 注释原话：*This should only be set for header files that are included by a significant number of other C++ modules*）。Core 是几乎所有模块的依赖，所以它提供共享 PCH；普通游戏模块只设私有 PCH。

---

## 六、速查表

| 项 | `PrivatePCHHeaderFile` | `SharedPCHHeaderFile` |
|---|---|---|
| Core 里的值 | `"Private/CorePrivatePCH.h"` | `"Public/CoreSharedPCH.h"` |
| 真实文件行数 | 11 行 | 263 行 |
| include 数量 | 6 个（1 个共享 + 5 个额外） | 252 个 |
| 谁用 | 只有本模块的 `.cpp` | 全引擎所有依赖 Core 的模块 |
| 命名约定 | `Private/{模块名}PrivatePCH.h` | `Public/{模块名}SharedPCH.h` |
| 不设会怎样 | 退回用共享 PCH | 不提供共享 PCH，别的模块自己生成 |

**口诀**：**共享 PCH 是「全引擎共用的大面团」，私有 PCH 是「本模块在大面团上再揉 5 把的小面团」；Core 两个都设，因为它是几乎所有模块的底座。**

---

## 待确认

1. 这两个文件看懂了吗？**不用抄**（答疑文件，主线仍是 `Core.Build.cs` → `Core_APL.xml`）。
2. 重点记：`CorePrivatePCH.h` 第 5 行 `#include "CoreSharedPCH.h"` —— 私有 PCH 是共享 PCH 的超集。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 13、15 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Private\CorePrivatePCH.h`（11 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`（263 行 / 252 include）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行，第 193~219、793、799 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildModuleCPP.cs`（第 296、305、1131、1137、1151、1366、1956 行）