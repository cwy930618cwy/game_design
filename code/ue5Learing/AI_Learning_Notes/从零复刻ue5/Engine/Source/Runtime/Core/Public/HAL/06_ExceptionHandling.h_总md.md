# 06 — `HAL/ExceptionHandling.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`（**50 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`
> **✅ 本节可独立编译**——只 include 2 个 Core 内部头 + 平台头。

---

## 一、这个文件解决什么问题（为什么）

游戏崩了，你想知道「崩在哪、为什么崩」。`ExceptionHandling.h` 就是**崩溃上报的接口清单**：它声明了一组 `ReportXxx()` 函数，各平台各自实现，崩溃时被调用，生成崩溃报告 / minidump / 调用栈。

**它声明了 3 类东西**：

| 类别 | 内容 |
|---|---|
| **全局开关** | `GAlwaysReportCrash`（调试器附加时也报）、`GUseCrashReportClient`（用哪个上报器）、`MiniDumpFilenameW`（转储文件名） |
| **平台专属 `ReportXxx()`** | `ReportCrash` / `ReportAssert` / `ReportGPUCrash` / `ReportEnsure` / `ReportStall` / `ReportHang` |
| **交互式 Ensure** | `ReportInteractiveEnsure` / `IsInteractiveEnsureMode` |

**说个场景**：你在代码里写 `check(ptr != nullptr)`，指针为空时触发断言。引擎内部会调 `ReportAssert(消息, 程序计数器)` → 生成崩溃报告 → 弹窗告诉你「第几行、什么条件失败」。

**平台差异**（这是本文件最值得注意的地方）：

| 平台 | `ReportCrash` 签名 | 有 `ReportStall`？ |
|---|---|---|
| Windows | `(Windows::LPEXCEPTION_POINTERS)` | ✅ |
| Mac | `(ucontext_t*, int32 Signal, __siginfo*)` | ❌ |
| Unix | **没有** `ReportCrash` | ✅ |

> 🔑 **一句话**：`ExceptionHandling.h` = **崩溃上报的「接口清单」**，各平台自己实现，签名还不一样。

---

## 二、内部结构全景

```
ExceptionHandling.h（50 行）
├─ 第 1~8 行      版权头 + #pragma once + 2 个 include + 1 个前置声明
├─ 第 10~16 行    ① 3 个全局量（GAlwaysReportCrash / GUseCrashReportClient / MiniDumpFilenameW）
├─ 第 18~30 行    ② PLATFORM_WINDOWS 分支（6 个 ReportXxx）
├─ 第 31~39 行    ③ PLATFORM_MAC 分支（5 个 ReportXxx，无 ReportStall）
├─ 第 40~45 行    ④ PLATFORM_UNIX 分支（5 个 ReportXxx，无 ReportCrash）
├─ 第 46 行       #endif
└─ 第 49~50 行    ⑤ 交互式 Ensure（2 个函数）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/ExceptionHandling.h` | 无 | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 何时引入 | 作用 |
|---|---|---|
| `Containers/ContainersFwd.h` | 总是 | 容器前置声明 |
| `CoreTypes.h` | 总是 | `int32` / `uint32` / `uint64` / `TCHAR` |
| `Windows/WindowsSystemIncludes.h` | Windows | `Windows::LPEXCEPTION_POINTERS` |
| `<excpt.h>` | Windows | `EXCEPTION_POINTERS` |
| `<signal.h>` | Mac | `ucontext_t` / `__siginfo` |

**注意**：`FProgramCounterSymbolInfo` 只前置声明（第 8 行），本文件没用到它的完整定义。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\19_ExceptionHandling.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `FeedbackContextAnsi.h`（1127 字节）。