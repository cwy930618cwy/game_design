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

**先建本总 md（已建）→ 停，等确认 → 逐个文件教（每个文件 = 总 md + 完整代码 md 两份）**。

推进顺序（按你 2026-11-16 指令，先教 `Core\` 顶层两个文件）：

| 步骤 | 内容 | 状态 |
|---|---|---|
| 01 | `Core.Build.cs`（330 行） | 🟡 总 md 已建 |
| 02 | `Core_APL.xml`（73 行） | ⏳ 待教 |
| 03~09 | 7 个散头（`CoreTypes.h` 15 → `CoreFwd.h` 44 → `CoreMinimal.h` 183 → `CoreSharedPCH.h` 263 → `Core.h` 319 → `PixelFormat.h` 527 → `CoreGlobals.h` 854） | ⬜ 待教 |
| 10+ | 46 个子目录（`Containers` / `Math` / `HAL` / `Misc` …） | ⬜ 待教 |

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就按上表往下推。