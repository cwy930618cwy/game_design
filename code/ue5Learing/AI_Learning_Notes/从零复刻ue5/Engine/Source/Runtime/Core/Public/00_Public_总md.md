# 00 — `Core/Public/` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\`（**1196 文件** = 1154 `.h` + 21 `.inl` + 13 `.tps` + 8 `.isph`；**46 个子目录 + 7 个散头**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\`（同名同结构）
> **✅ 本节可独立编译**——纯头文件区，不产生编译单元。

---

## 一、这个目录解决什么问题（为什么）

`Core` 模块里，`Public\` 是**唯一对外开门的那间**：别的模块（`CoreUObject`、`Engine`、`Slate`…）能 `#include` 到的头文件，全在这里。`Private\` 是实现、`Internal\` 只给自己人、`Tests\` 只在跑测试时编。

**说个场景**：`Core` 是一台机柜，`Public\` 就是机柜**正面那排接口面板**——`TArray`、`FString`、`FVector`、`UE_LOG`、`check()` 全从这排面板插出去。你要用引擎任何基础能力，插的都是这排孔。

> 🔑 **一句话**：`Public\` = Core 的**对外接口面板**，1196 个文件、46 个抽屉 + 7 个门牌，一个都不能少、名字一个字不改。

---

## 二、内部结构全景

```
Core/Public/
├─ 7 个散头（门牌 + 总闸）
│    ├─ CoreTypes.h        15 行   ← 基础类型层，只拉 4 份定义头
│    ├─ CoreFwd.h          44 行   ← 前置声明 FName / FArchive / FText
│    ├─ CoreMinimal.h     183 行   ← 最小包含集，写普通 .h 最常用
│    ├─ CoreSharedPCH.h   263 行   ← 共享预编译头（Core.Build.cs 第 15 行引用）
│    ├─ Core.h            319 行   ← 模块总入口，几乎全量再导出
│    ├─ PixelFormat.h     527 行   ← EPixelFormat 等像素格式枚举
│    └─ CoreGlobals.h     854 行   ← 全局量 GLog / GError / GIsEditor / GIsServer
├─ 容器模板组（228 文件）
│    ├─ Containers  73   ← TArray / TMap / TSet / UnrealString
│    ├─ Templates   80   ← 类型萃取 / 智能指针 / Function
│    ├─ Algo        44   ← 排序 / 查找 / 遍历
│    ├─ Traits      12   ← IsContiguousContainer 等
│    ├─ Concepts     7   ← C++20 概念
│    └─ Delegates   12   ← 委托 / 多播
├─ 数学组
│    └─ Math        99   ← 向量 / 矩阵 / 四元数 / 随机数 / 插值曲线
├─ 字符串国际化组（52 文件）
│    ├─ String              15
│    └─ Internationalization 37
├─ 平台抽象组（307 文件）
│    ├─ HAL              93   ← 平台层总接口
│    ├─ GenericPlatform  31   ← 通用实现
│    ├─ Windows 35 / Mac 23 / Linux 20 / Unix 21 / IOS 25 / Android 28 / Apple 20 / Microsoft 14
├─ 工具组（341 文件）
│    ├─ Misc 181 / Serialization 56 / Async 38 / ProfilingDebugging 35
│    ├─ Stats 18 / Logging 12 / IO 12
│    └─ Hash 7 / Compression 7 / Memory 7
├─ UObject 接口组
│    └─ UObject     59   ← Core 侧给 UObject 用的基础声明
├─ 模块任务组
│    ├─ Modules      7   ← IModuleInterface / FModuleManager
│    └─ Tasks        4   ← UE::Tasks
└─ 专项组（52 文件）
     └─ Experimental 24 / Clang 5 / FramePro 5 / ColorManagement 3 / Audio 2 / Features 2 / MemPro 2 / MSVC 2 / Virtualization 2 / AutoRTFM 1 / FileCache 1 / IntelICX 1 / Sanitizer 1 / Tests 6
```

> 46 个子目录合计 1189 文件 + 7 个散头 = **1196**，与 `00_Core_总md.md` 实查总数一致。

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| 7 个散头 | 无（只互相 include + `HAL/Platform.h`） | ✅ 能 | 否 |
| 46 个子目录 | 无（全部只依赖 `Core` 内部 + 平台宏） | ✅ 能 | 否 |

**关键 include 链**（实查）：

| 文件 | 拉谁 |
|---|---|
| `Core.h` | `CoreMinimal.h` + 容器 + 数学 + 序列化 + 委托 + 国际化…（`IWYU pragma: begin_exports`） |
| `CoreMinimal.h` | `CoreTypes.h` + `CoreFwd.h` + `UObjectHierarchyFwd.h` + `ContainersFwd.h` + 常用头 |
| `CoreTypes.h` | `HAL/Platform.h`、`UMemoryDefines.h`、`CoreMiscDefines.h`、`CoreDefines.h` |
| `CoreFwd.h` | `CoreTypes.h` + `ContainersFwd.h` + `MathFwd.h` + `UObjectHierarchyFwd.h` |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 逐个文件教（每个文件 = 超详细注释 md + AI 生成还原文件）**。

### 4.1 已完成（`Core\` 顶层两个文件）

| 步骤 | 内容 | 状态 |
|---|---|---|
| 01 | `Core.Build.cs`（330 行） | ✅ 已还原并验收（669 行，代码 260/260 零差异） |
| 02 | `Core_APL.xml`（73 行） | ✅ 已还原并验收（130 行，代码 64/64 零差异） |

### 4.2 `Public\` 教程顺序（**按依赖链 + 由浅入深**，共 12 阶段）

> **排序原则**：① 先「门牌」（散头）→ ② 再「地基」（平台层 / 类型层）→ ③ 再「砖块」（容器 / 模板 / 数学）→ ④ 再「工具」→ ⑤ 最后「专项」。
> 理由：后面的文件大量 include 前面的文件，先学地基才看得懂上层。

| 阶段 | 内容 | 文件数 | 为什么排这里 |
|---|---|---|---|
| **P1** | **7 个散头**（`CoreTypes.h` 15 → `CoreFwd.h` 44 → `CoreMinimal.h` 183 → `CoreSharedPCH.h` 263 → `Core.h` 319 → `PixelFormat.h` 527 → `CoreGlobals.h` 854） | 7 | **门牌 + 总闸**：所有其他头都直接或间接被它们拉进来；最短 15 行、最长 854 行，由浅入深 |
| **P2** | **`HAL\`**（93）+ **`GenericPlatform\`**（31） | 124 | **地基**：`HAL/Platform.h` 是所有头的最底层依赖；`GenericPlatform` 是 HAL 的通用实现 |
| **P3** | **`Templates\`**（80）+ **`Traits\`**（12）+ **`Concepts\`**（7） | 99 | **类型工具**：容器 / 数学都靠它们（`TIsIntegral` / `TEnableIf` / `TUniquePtr`…） |
| **P4** | **`Containers\`**（73） | 73 | **砖块**：`TArray` / `TMap` / `TSet` / `FString`，最常用 |
| **P5** | **`Math\`**（99） | 99 | **砖块**：`FVector` / `FMatrix` / `FQuat` / `FRandomStream` |
| **P6** | **`String\`**（15）+ **`Internationalization\`**（37） | 52 | **字符串**：`FText` / 文化 / 本地化（依赖 P4 的 `FString`） |
| **P7** | **`Misc\`**（181） | 181 | **工具**：断言 / 路径 / 配置 / 时间 / GUID（最大的一块） |
| **P8** | **`Logging\`**（12）+ **`Stats\`**（18）+ **`ProfilingDebugging\`**（35） | 65 | **诊断**：`UE_LOG` / 统计 / 性能追踪 |
| **P9** | **`Async\`**（38）+ **`Tasks\`**（4）+ **`Modules\`**（7） | 49 | **并发与模块**：异步任务 / `UE::Tasks` / `IModuleInterface` |
| **P10** | **`Serialization\`**（56）+ **`IO\`**（12）+ **`Compression\`**（7）+ **`Hash\`**（7）+ **`Memory\`**（7） | 89 | **数据**：序列化 / 文件 IO / 压缩 / 哈希 / 内存 |
| **P11** | **`UObject\`**（59）+ **`Delegates\`**（12）+ **`Algo\`**（44） | 115 | **接口与算法**：Core 侧 UObject 声明 / 委托 / 排序查找 |
| **P12** | **各平台目录**（`Windows` 35 / `Android` 28 / `IOS` 25 / `Mac` 23 / `Unix` 21 / `Apple` 20 / `Linux` 20 / `Microsoft` 14） | 186 | **平台特化**：各平台对 HAL 的具体实现 |
| **P13** | **专项组**（`Experimental` 24 / `Tests` 6 / `Clang` 5 / `FramePro` 5 / `ColorManagement` 3 / `Audio` 2 / `Features` 2 / `MemPro` 2 / `MSVC` 2 / `Virtualization` 2 / `AutoRTFM` 1 / `FileCache` 1 / `IntelICX` 1 / `Sanitizer` 1） | 57 | **专项**：编译器适配 / 工具链 / 实验特性 |

**合计**：7 + 124 + 99 + 73 + 99 + 52 + 181 + 65 + 49 + 89 + 115 + 186 + 57 = **1196** ✅

### 4.3 每个阶段内部顺序

**阶段内按「文件名首字母」逐个教**（和引擎目录排序一致），每个文件：
1. 建**超详细注释 md**（`答疑文件\NN_<文件名>逐行注释版.md`）
2. **AI 直接生成还原文件**（`ue_5.6\...\Public\<同名路径>`）
3. 用户验收

---

## 五、为什么这么排（排序理由）

| 原则 | 说明 |
|---|---|
| **先门牌后房间** | 7 个散头是所有头的入口，先认清它们才知道后面每个头被谁拉进来 |
| **先地基后砖块** | `HAL` 是最底层（`Platform.h` 被所有头 include），不先学它，后面每个头第一行 `#include "HAL/Platform.h"` 都看不懂 |
| **先工具后使用** | `Templates` / `Traits` 是容器和数学的基础（`TArray` 内部用 `TIsTriviallyDestructible`），先学才看得懂容器实现 |
| **先常用后冷门** | `Containers` / `Math` / `Misc` 是日常最常用的，排在前面；`Experimental` / `Sanitizer` 等专项排最后 |
| **平台特化最后** | 各平台目录（`Windows` / `Android`…）是 HAL 的具体实现，必须先懂 HAL 接口才看得懂实现 |

---

## 六、速查表 · 口诀

| 三要素 | 值 |
|---|---|
| 读什么 | `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\`（1196 文件 / 46 子目录 + 7 散头） |
| 讲义建到哪 | `AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\` |
| 代码还原到哪 | `ue_5.6\Engine\Source\Runtime\Core\Public\` |

**口诀**：**先 7 个门牌（散头）→ 再地基（HAL + GenericPlatform）→ 再类型工具（Templates/Traits/Concepts）→ 再砖块（Containers/Math）→ 再字符串 → 再工具（Misc/Logging/Stats）→ 再并发 → 再数据 → 再接口算法 → 最后平台特化与专项。**

---

## 待确认

1. **这个 13 阶段顺序对吗？** 有要调整的告诉我（比如想先学 `Containers` 再学 `HAL`）。
2. 确认后我从 **P1 第 1 个文件 `CoreTypes.h`（15 行）** 开始，按新流程（超详细注释 md → 生成文件）。
3. 每个阶段开始前我会再建一份该阶段的子总 md。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\`（2026-11-17 实查：46 个子目录 + 7 个散头，1196 文件）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreTypes.h`（15 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreFwd.h`（44 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreMinimal.h`（183 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`（263 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\Core.h`（319 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h`（527 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（854 行）