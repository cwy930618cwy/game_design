// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入容器前置声明：TArray 等容器类型的前置声明。
#include "Containers/ContainersFwd.h"
// 引入基础类型层：int32 / uint32 / uint64 / TCHAR 都来自这里。
#include "CoreTypes.h"

// 前置声明 FProgramCounterSymbolInfo：程序计数器符号信息（崩溃栈解析用）。
struct FProgramCounterSymbolInfo;

/** 即使调试器已附加，是否仍要生成崩溃报告。 */
// GAlwaysReportCrash：调试器附加时也生成崩溃报告。
extern CORE_API bool GAlwaysReportCrash;

/** 是否使用 ClientReportClient 而不是 AutoReporter。 */
// GUseCrashReportClient：使用 CrashReportClient 上报崩溃。
extern CORE_API bool GUseCrashReportClient;

// MiniDumpFilenameW：崩溃转储文件名缓冲区（1024 个宽字符）。
extern CORE_API TCHAR MiniDumpFilenameW[1024];

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


// ReportInteractiveEnsure：上报交互式 Ensure（弹对话框让用户选）。
extern CORE_API void ReportInteractiveEnsure(const TCHAR* InMessage);
// IsInteractiveEnsureMode：当前是否处于交互式 Ensure 模式。
extern CORE_API bool IsInteractiveEnsureMode();