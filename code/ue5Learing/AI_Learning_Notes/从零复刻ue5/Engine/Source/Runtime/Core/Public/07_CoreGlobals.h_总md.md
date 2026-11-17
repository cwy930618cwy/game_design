# 07 — `CoreGlobals.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（**854 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`
> **✅ 本节可独立编译**——只 include Core 内部头 + 标准库 `<atomic>`，无工程内类依赖。

---

## 一、这个文件解决什么问题（为什么）

引擎里到处都在用 `GIsEditor`、`GIsServer`、`GLog`、`GConfig`、`GFrameCounter` 这些**全局变量**。它们没有类、没有命名空间，直接以 `G` 开头散落在全局作用域里。`CoreGlobals.h` 就是这些全局量的**总登记处**：所有 `extern` 声明集中在这里，真正的定义在 `CoreGlobals.cpp`。

**说个场景**：你在游戏代码里写 `if (GIsServer) { ... }`，编译器怎么知道 `GIsServer` 是什么类型、在哪个 DLL 里？答案就是它 include 了 `CoreGlobals.h`，从这里的 `extern CORE_API bool GIsServer;` 拿到声明，链接时再去 Core 模块里找定义。

**它还管三件事**：
1. **日志分类声明**（`LogHAL` / `LogCore` / `LogTemp` … 共 24 个）
2. **线程标签系统**（`ETaskTag` + `FTaskTagScope`，`IsInGameThread()` 就靠它）
3. **commandlet 判断**（`IsRunningCommandlet()` / `IsRunningCookCommandlet()` …）

> 🔑 **一句话**：`CoreGlobals.h` = **引擎全局量的「户口本」**，854 行里 400 多个 `extern` 声明，谁要用谁查这本。

---

## 二、内部结构全景

```
CoreGlobals.h（854 行）
├─ 第 1~15 行     版权头 + #pragma once + 11 个 include（含 <atomic>）
├─ 第 17~27 行    11 个前置声明（Error / FConfigCacheIni / FExec / FText / UClass …）
├─ 第 29~65 行    ① 日志分类声明（24 个 DECLARE_LOG_CATEGORY_EXTERN + 平台辅助宏）
├─ 第 68~72 行    日志单例 + 启动耗时点函数
├─ 第 74~81 行    ② struct FScopedBootTiming（启动耗时作用域）
├─ 第 83~88 行    ③ struct FEngineTrackedActivityScope（Insights 活动作用域）
├─ 第 91~92 行    ④ SCOPED_BOOT_TIMING / UE_SCOPED_ENGINE_ACTIVITY 宏
├─ 第 95~106 行   ⑤ 核心全局量（GLog / GConfig / GUndo / GError / GWarn / GErrorHist）
├─ 第 108~129 行  ⑥ struct FCoreTexts（True/False/Yes/No/None 本地化文本）
├─ 第 131~142 行  ⑦ 旧版全局文本 GTrue/GFalse/GYes/GNo/GNone（已废弃）
├─ 第 144~194 行  ⑧ 蓝图/对象/嵌入相关开关（GIsGameAgnosticExe … GUELibraryOverrideSettings）
├─ 第 196~205 行  ⑨ commandlet 私有标志（WITH_ENGINE 内）
├─ 第 207~237 行  ⑩ GIsEditor 三态定义（编辑器 / 代码分析 / 运行时）
├─ 第 239~325 行  ⑪ commandlet 判断函数（IsRunningCommandlet 等 6 个）
├─ 第 327~342 行  ⑫ UE::GetMultiprocessId / UE::Private::SetMultiprocessId
├─ 第 344~356 行  ⑬ FIsDuplicatingClassForReinstancing + 客户端/服务器标志
├─ 第 358~427 行  ⑭ 启动/退出/截图相关全局量
├─ 第 429~457 行  ⑮ 各种 ini 路径（GEngineIni / GGameIni / GDeviceProfilesIni …）
├─ 第 459~494 行  ⑯ 异步加载函数指针 + ELoaderType 枚举
├─ 第 496~538 行  ⑰ 编辑器加载包 / PIE 实例号（含废弃包装类）
├─ 第 540~654 行  ⑱ 帧号 / 线程 ID / 日志打印开关 / 统计开关
├─ 第 656~676 行  ⑲ enum class ETaskTag（线程标签位标志）
├─ 第 679~735 行  ⑳ class FTaskTagScope + FOptionalTaskTagScope（已废弃）
├─ 第 737~791 行  ㉑ 线程判断函数（IsInGameThread / IsInRenderingThread / IsInRHIThread …）
├─ 第 793~819 行  ㉒ FScopedLoadingState（加载状态作用域，Shipping 下空实现）
├─ 第 821~847 行  ㉓ 绘制事件开关 + 调试可视化指针 + Zen 流式加载
└─ 第 849~854 行  ㉔ IsThisNotNull（已废弃）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `CoreGlobals.h` | 无（只 include Core 内部头 + `<atomic>`） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `Containers/UnrealString.h` | `FString`（各种 ini 路径） |
| `CoreTypes.h` | `uint8` / `uint32` / `uint64` / `int32` / `TCHAR` |
| `HAL/PlatformTLS.h` | `thread_local` 支持 |
| `Logging/LogMacros.h` | `DECLARE_LOG_CATEGORY_EXTERN` |
| `Misc/Build.h` | `WITH_ENGINE` / `WITH_EDITOR` / `UE_BUILD_SHIPPING` |
| `Misc/EnumClassFlags.h` | `ENUM_CLASS_FLAGS`（给 `ETaskTag` 用） |
| `Misc/OutputDevice.h` | `FOutputDeviceConsole` / `FOutputDeviceRedirector` |
| `ProfilingDebugging/CpuProfilerTrace.h` | `TRACE_CPUPROFILER_EVENT_SCOPE_STR` |
| `Templates/Atomic.h` | `TSAN_ATOMIC` 宏 |
| `UObject/NameTypes.h` | `FName` / `FLazyName` |
| `<atomic>` | `std::atomic<bool>`（`GIsReinstancing`） |

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\13_CoreGlobals.h逐行注释版.md` + 还原文件 `ue_5.6\...\Public\CoreGlobals.h`，同一轮全部产出并脚本验收。

---

**下一步**：本节完结后，`Public\` 7 个散头全部完成 → 进 `Public\` 子目录（`Containers` / `Math` / `HAL` / `Misc` …）。