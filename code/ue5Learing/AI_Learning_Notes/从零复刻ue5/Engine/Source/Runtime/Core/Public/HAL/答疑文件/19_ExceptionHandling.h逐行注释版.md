# 19 — `HAL/ExceptionHandling.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`（**50 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`
> **✅ 本节可独立编译**——只 include 2 个 Core 内部头 + 平台头。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

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

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + include + 前置声明（第 1~8 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入容器前置声明：TArray 等容器类型的前置声明。
#include "Containers/ContainersFwd.h"
// 引入基础类型层：int32 / uint32 / uint64 / TCHAR 都来自这里。
#include "CoreTypes.h"

// 前置声明 FProgramCounterSymbolInfo：程序计数器符号信息（崩溃栈解析用）。
struct FProgramCounterSymbolInfo;
```

**本段新元素**

- `Containers/ContainersFwd.h` —— 容器前置声明
- `CoreTypes.h` —— `int32` / `uint32` / `uint64` / `TCHAR`
- `struct FProgramCounterSymbolInfo;` —— 前置声明（本文件只用到指针）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~6 行 | 2 个 include |
| 第 8 行 | 前置声明 |

> 类比：像开会前先报名字：人还没到，名字先占位。

### 二、3 个全局量（第 10~16 行）

```cpp
/** 即使调试器已附加，是否仍要生成崩溃报告。 */
// GAlwaysReportCrash：调试器附加时也生成崩溃报告。
extern CORE_API bool GAlwaysReportCrash;

/** 是否使用 ClientReportClient 而不是 AutoReporter。 */
// GUseCrashReportClient：使用 CrashReportClient 上报崩溃。
extern CORE_API bool GUseCrashReportClient;

// MiniDumpFilenameW：崩溃转储文件名缓冲区（1024 个宽字符）。
extern CORE_API TCHAR MiniDumpFilenameW[1024];
```

**本段新元素**

- `extern CORE_API bool GAlwaysReportCrash` —— 调试器附加时也生成崩溃报告
- `extern CORE_API bool GUseCrashReportClient` —— 用 CrashReportClient 而非 AutoReporter
- `extern CORE_API TCHAR MiniDumpFilenameW[1024]` —— 转储文件名缓冲区（1024 宽字符）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 10~11 行 | `GAlwaysReportCrash` |
| 第 13~14 行 | `GUseCrashReportClient` |
| 第 16 行 | `MiniDumpFilenameW` |

> 类比：像事故记录本的三个开关：要不要记、记到哪、文件叫什么。

### 三、Windows 分支（6 个 ReportXxx）（第 18~30 行）

```cpp
// #CrashReport: 2014-09-11 计划挪到 PlatformExceptionHandling
// 如果是 Windows 平台。
#if PLATFORM_WINDOWS
// 引入 Windows 系统头：Windows::LPEXCEPTION_POINTERS 等类型。
#include "Windows/WindowsSystemIncludes.h"

// 引入 Windows 异常处理头：EXCEPTION_POINTERS 结构。
#include <excpt.h>

// #CrashReport: 2014-10-09 这些方法是 Windows 专属的，计划从这里移走。
// ReportCrash：上报崩溃（Windows 版，接收异常指针）。
extern CORE_API int32 ReportCrash( Windows::LPEXCEPTION_POINTERS ExceptionInfo );
// ReportAssert：上报断言失败。
extern CORE_API void ReportAssert(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportGPUCrash：上报 GPU 崩溃。
extern CORE_API void ReportGPUCrash(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportEnsure：上报 Ensure 失败。
extern CORE_API void ReportEnsure(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportStall：上报卡顿（带卡住的线程 ID）。
extern CORE_API void ReportStall(const TCHAR* ErrorMessage, uint32 HitchThreadId);
// ReportHang：上报挂起（带调用栈帧和挂起线程 ID）。
extern CORE_API void ReportHang(const TCHAR*, const uint64* StackFrames, int32 NumStackFrames, uint32 HungThreadId);
```

**本段新元素**

- `#if PLATFORM_WINDOWS` —— 平台条件编译
- `Windows/WindowsSystemIncludes.h` —— `Windows::LPEXCEPTION_POINTERS`
- `<excpt.h>` —— Windows 异常处理头
- `ReportCrash(Windows::LPEXCEPTION_POINTERS)` —— Windows 版崩溃上报
- `ReportAssert` / `ReportGPUCrash` / `ReportEnsure` —— 断言 / GPU 崩溃 / Ensure
- `ReportStall(ErrorMessage, HitchThreadId)` —— 卡顿上报（带线程 ID）
- `ReportHang(..., StackFrames, NumStackFrames, HungThreadId)` —— 挂起上报（带栈帧）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 18~22 行 | 待办注释 + 2 个平台头 |
| 第 24~25 行 | `ReportCrash`（Windows 专属签名） |
| 第 26~28 行 | `ReportAssert` / `ReportGPUCrash` / `ReportEnsure` |
| 第 29 行 | `ReportStall` |
| 第 30 行 | `ReportHang` |

> 类比：像 Windows 专属的急救箱：里面工具最全（6 件）。

### 四、Mac 分支（5 个 ReportXxx）（第 31~39 行）

```cpp
// 否则如果是 Mac 平台。
#elif PLATFORM_MAC
// #CrashReport: 2014-10-09 应该挪到另一个文件
// 引入 POSIX 信号头：ucontext_t / __siginfo 等类型。
#include <signal.h>

// ReportCrash：上报崩溃（Mac 版，接收上下文 + 信号 + 信号信息）。
extern CORE_API int32 ReportCrash(ucontext_t *Context, int32 Signal, struct __siginfo* Info);
// ReportAssert：上报断言失败。
extern CORE_API void ReportAssert(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportGPUCrash：上报 GPU 崩溃。
extern CORE_API void ReportGPUCrash(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportEnsure：上报 Ensure 失败。
extern CORE_API void ReportEnsure(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportHang：上报挂起。
extern CORE_API void ReportHang(const TCHAR*, const uint64* StackFrames, int32 NumStackFrames, uint32 HungThreadId);
```

**本段新元素**

- `#elif PLATFORM_MAC` —— Mac 平台
- `<signal.h>` —— POSIX 信号头（`ucontext_t` / `__siginfo`）
- `ReportCrash(ucontext_t*, int32 Signal, __siginfo*)` —— Mac 版签名（3 个参数）
- **没有 `ReportStall`** —— Mac 不支持卡顿上报

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 31~33 行 | 平台判断 + 信号头 |
| 第 35 行 | `ReportCrash`（Mac 专属签名） |
| 第 36~38 行 | `ReportAssert` / `ReportGPUCrash` / `ReportEnsure` |
| 第 39 行 | `ReportHang`（无 `ReportStall`） |

> 类比：像 Mac 的急救箱：工具少一件（没有卡顿上报）。

### 五、Unix 分支（5 个 ReportXxx）（第 40~46 行）

```cpp
// 否则如果是 Unix 平台（Linux 等）。
#elif PLATFORM_UNIX
// ReportAssert：上报断言失败。
extern CORE_API void ReportAssert(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportGPUCrash：上报 GPU 崩溃。
extern CORE_API void ReportGPUCrash(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportEnsure：上报 Ensure 失败。
extern CORE_API void ReportEnsure(const TCHAR* ErrorMessage, void* ProgramCounter);
// ReportStall：上报卡顿（带线程 ID）。
extern CORE_API void ReportStall(const TCHAR* ErrorMessage, uint32 ThreadId);
// ReportHang：上报挂起。
extern CORE_API void ReportHang(const TCHAR*, const uint64* StackFrames, int32 NumStackFrames, uint32 HungThreadId);
// 平台条件编译结束。
#endif
```

**本段新元素**

- `#elif PLATFORM_UNIX` —— Unix 平台（Linux 等）
- **没有 `ReportCrash`** —— Unix 不声明崩溃上报
- `ReportStall(ErrorMessage, ThreadId)` —— 参数名与 Windows 不同（`ThreadId` vs `HitchThreadId`）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 40 行 | 平台判断 |
| 第 41~43 行 | `ReportAssert` / `ReportGPUCrash` / `ReportEnsure` |
| 第 44 行 | `ReportStall`（参数名不同） |
| 第 45 行 | `ReportHang` |
| 第 46 行 | 条件编译结束 |

> 类比：像 Linux 的急救箱：没有「崩溃上报」这件，但有「卡顿上报」。

### 六、交互式 Ensure（第 49~50 行）

```cpp
// ReportInteractiveEnsure：上报交互式 Ensure（弹对话框让用户选）。
extern CORE_API void ReportInteractiveEnsure(const TCHAR* InMessage);
// IsInteractiveEnsureMode：当前是否处于交互式 Ensure 模式。
extern CORE_API bool IsInteractiveEnsureMode();
```

**本段新元素**

- `ReportInteractiveEnsure(InMessage)` —— 上报交互式 Ensure（弹对话框让用户选）
- `IsInteractiveEnsureMode()` —— 当前是否处于交互式 Ensure 模式

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 49 行 | `ReportInteractiveEnsure` |
| 第 50 行 | `IsInteractiveEnsureMode` |

> 类比：像弹窗问「继续还是退出」：交互式 Ensure 就是那个弹窗。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + include + 前置声明 | 1~8 | 版权头 + include + 前置声明 |
| 3 个全局量 | 10~16 | 3 个全局量 |
| Windows 分支（6 个 ReportXxx） | 18~30 | Windows 分支（6 个 ReportXxx） |
| Mac 分支（5 个 ReportXxx） | 31~39 | Mac 分支（5 个 ReportXxx） |
| Unix 分支（5 个 ReportXxx） | 40~46 | Unix 分支（5 个 ReportXxx） |
| 交互式 Ensure | 49~50 | 交互式 Ensure |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `extern CORE_API T X;` | 声明外部定义的全局量 | `extern CORE_API bool GAlwaysReportCrash;` |
| `extern CORE_API T X[N];` | 声明外部定义的全局数组 | `extern CORE_API TCHAR MiniDumpFilenameW[1024];` |
| `#if PLATFORM_X` / `#elif` / `#endif` | 平台条件编译 | `#if PLATFORM_WINDOWS` |
| `struct X;` | 前置声明 | `struct FProgramCounterSymbolInfo;` |
| `T* Ptr` | 指针参数 | `const uint64* StackFrames` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 50 行 | 87 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 32 行 | 32 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `#CrashReport:` 待办 | 保留原文格式 | 保留 `#CrashReport:` + 中文说明 | 待办标记，保留可搜索性 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`（87 行）
2. 脚本验收：`codeOurs=32 / codeEngine=32 / diff=0`
3. 确认后下一个：`HAL\FeedbackContextAnsi.h`（1127 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`（50 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ExceptionHandling.h`（87 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\06_ExceptionHandling.h_总md.md`