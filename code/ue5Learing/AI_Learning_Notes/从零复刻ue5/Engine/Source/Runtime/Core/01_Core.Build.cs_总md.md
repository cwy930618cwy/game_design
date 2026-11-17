# 01 — `Core.Build.cs` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs`
> **✅ 本节可独立编译**——C# 构建脚本，由 UBT 自己编译，不参与 C++ 编译。

---

## 一、这个文件解决什么问题（为什么）

上一课两张 `Target.cs` 回答「**这次编什么**」（编辑器 / 游戏）。这一份回答「**Core 这个模块怎么编**」——同一个 UBT 体系，只是基类从 `TargetRules` 换成了 `ModuleRules`。

**说个场景**：`Target.cs` 是总包工头说「今天盖楼还是盖厂房」；`Core.Build.cs` 是 Core 这间机房的施工细则——用什么预编译头、接哪几路模块、每个平台额外挂什么库和宏。

> 🔑 **一句话**：`Core.Build.cs` = Core 模块的**施工细则**，330 行全是往 UBT 的清单里**加名字**（模块名 / 库名 / 宏名 / 路径），一行 C++ 逻辑都没有。

---

## 二、内部结构全景

```
public class Core : ModuleRules
├─ 构造函数 Core(ReadOnlyTargetRules Target)
│    ├─ ① 骨架（1~15 行）
│    │     ├─ NumIncludedBytesPerUnityCPPOverride = 491520   ← Unity 合批上限
│    │     ├─ PrivatePCHHeaderFile  = "Private/CorePrivatePCH.h"
│    │     └─ SharedPCHHeaderFile   = "Public/CoreSharedPCH.h"
│    ├─ ② 依赖清单（17~65 行）
│    │     ├─ PrivateDependencyModuleNames   ← BuildSettings / AtomicQueue / AutoRTFM / BLAKE3 / OodleDataCompression / xxhash
│    │     ├─ PublicDependencyModuleNames    ← GuidelinesSupportLibrary / TraceLog / AtomicQueue
│    │     ├─ PrivateIncludePathModuleNames  ← DerivedDataCache / TargetPlatform / Json / RSA
│    │     ├─ PublicIncludePathModuleNames   ← AutoRTFM / ImageCore
│    │     └─ Android 块（18~31）→ 第 30 行注册 Core_APL.xml
│    ├─ ③ 平台分支（67~220 行）
│    │     ├─ Win64 → libpas
│    │     ├─ Windows → zlib / IntelTBB / IntelVTune / mimalloc / DBGHELP / PIX
│    │     ├─ Mac → IntelTBB / zlib / PLCrashReporter / 8 个 Framework / MultitouchSupport
│    │     ├─ IOS → zlib / 13 个 Framework / PLCrashReporter / iAD / VisibileForDll
│    │     ├─ Android → cxademangle / zlib / libunwind / Scudo 追踪
│    │     └─ Unix → zlib / jemalloc / dl / mimalloc
│    ├─ ④ 通用开关（222~311 行）
│    │     ├─ ICU / Desktop 崩溃上下文 / Windows 三个 profiler
│    │     ├─ IS_CLIENT_TARGET / UE_WITH_IRIS / UE_MERGED_MODULES
│    │     ├─ LTCG / PGO / MallocStomp / BinaryConfig
│    │     └─ CppCompileWarningSettings / IWYUSupport
│    └─ ⑤ 两个自定义成员（314~329 行）
│          ├─ protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
│          └─ protected virtual bool bWithMallocStomp  ← 只读属性
```

> 330 行里**没有一行 C++**：全是 `List<string>.Add(...)` 和 `if (平台/配置)` 判断。

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `Core.Build.cs` | 无（只依赖 UBT 程序集 `UnrealBuildTool`） | ✅ 能 | 否 |

**引用的三个引擎文件**（都在本模块内，不是外部依赖）：

| 被引用 | 引用位置 | 说明 |
|---|---|---|
| `Private/CorePrivatePCH.h` | 第 13 行 | 私有预编译头 |
| `Public/CoreSharedPCH.h` | 第 15 行 | 共享预编译头（`Public\` 里那个 263 行散头） |
| `Core_APL.xml` | 第 30 行（仅 Android） | 下一课要教的那个 73 行 xml |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 教完整 330 行代码（`01_Core.Build.cs_完整代码.md`）→ 停 → 教 `Core_APL.xml`（73 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就贴 `Core.Build.cs` 的完整 330 行代码。