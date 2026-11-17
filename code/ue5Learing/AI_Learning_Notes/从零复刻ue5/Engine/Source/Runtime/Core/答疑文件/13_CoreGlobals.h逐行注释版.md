# 13 — `CoreGlobals.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（**854 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`
> **✅ 本节可独立编译**——只 include Core 内部头 + 标准库 `<atomic>`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

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

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + include + 前置声明（第 1~27 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。
// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入 FString：本文件大量全局变量是 FString 类型（各种 ini 路径）。
#include "Containers/UnrealString.h"
// 引入基础类型层：uint8 / uint32 / uint64 / int32 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入平台 TLS 接口：FTaskTagScope 用 thread_local 记录当前线程标签。
#include "HAL/PlatformTLS.h"
// 引入日志宏：DECLARE_LOG_CATEGORY_EXTERN 在这里声明全局日志分类。
#include "Logging/LogMacros.h"
// 引入编译开关宏：WITH_ENGINE / WITH_EDITOR / UE_BUILD_SHIPPING 等。
#include "Misc/Build.h"
// 引入 ENUM_CLASS_FLAGS 宏：给 ETaskTag 生成位运算。
#include "Misc/EnumClassFlags.h"
// 引入输出设备接口：GError / GWarn / GLogConsole 的类型来源。
#include "Misc/OutputDevice.h"
// 引入 CPU 性能分析追踪宏：SCOPED_BOOT_TIMING 里用到 TRACE_CPUPROFILER_EVENT_SCOPE_STR。
#include "ProfilingDebugging/CpuProfilerTrace.h"
// 引入 TAtomic：TSAN_ATOMIC 宏展开后用到。
#include "Templates/Atomic.h"
// 引入 FName / FLazyName：GCurrentTraceName / GLongCorePackageName 的类型。
#include "UObject/NameTypes.h"

// 引入标准库 atomic：GIsReinstancing 用 std::atomic<bool>。
#include <atomic>

// 前置声明 Error：错误对象类型，本文件只用到指针。
class Error;
// 前置声明 FChunkedFixedUObjectArray：调试可视化用的对象数组。
class FChunkedFixedUObjectArray;
// 前置声明 FConfigCacheIni：GConfig 的类型。
class FConfigCacheIni;
// 前置声明 FExec：GDebugToolExec 的类型。
class FExec;
// 前置声明 FFixedUObjectArray：旧版对象数组。
class FFixedUObjectArray;
// 前置声明 FOutputDeviceConsole：GLogConsole 的类型。
class FOutputDeviceConsole;
// 前置声明 FOutputDeviceRedirector：GetGlobalLogSingleton 的返回类型。
class FOutputDeviceRedirector;
// 前置声明 FRunnableThread：GRenderingThread 的类型。
class FRunnableThread;
// 前置声明 FText：FCoreTexts 里全是 FText 引用。
class FText;
// 前置声明 ITransaction：GUndo 的类型。
class ITransaction;
// 前置声明 UClass：PRIVATE_GRunningCommandletClass 的类型。
class UClass;
```

**本段新元素**

- `#pragma once` —— 头文件保护
- `Containers/UnrealString.h` —— `FString`（各种 ini 路径）
- `HAL/PlatformTLS.h` —— `thread_local` 支持
- `Logging/LogMacros.h` —— `DECLARE_LOG_CATEGORY_EXTERN`
- `Misc/Build.h` —— `WITH_ENGINE` / `WITH_EDITOR` / `UE_BUILD_SHIPPING`
- `ProfilingDebugging/CpuProfilerTrace.h` —— `TRACE_CPUPROFILER_EVENT_SCOPE_STR`
- `Templates/Atomic.h` —— `TSAN_ATOMIC` 宏
- `UObject/NameTypes.h` —— `FName` / `FLazyName`
- `<atomic>` —— `std::atomic<bool>`
- `class Error;` 等 11 个 —— 前置声明，本文件只用到指针

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 2 行 | 头文件保护 |
| 第 4~13 行 | 10 个 Core 内部 include |
| 第 15 行 | 标准库 `<atomic>` |
| 第 17~27 行 | 11 个前置声明 |

> 类比：像出门前列清单：要用的工具先点名，具体长什么样后面再说。

### 二、日志分类声明（24 个）（第 29~65 行）

```cpp
// LogHAL —— HAL 平台抽象层日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogHAL, Log, All);
// LogSerialization —— 序列化日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogSerialization, Log, All);
// LogUnrealMath —— 数学库日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealMath, Log, All);
// LogUnrealMatrix —— 矩阵运算日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealMatrix, Log, All);
// LogContentComparisonCommandlet —— 内容比对命令行工具日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogContentComparisonCommandlet, Log, All);
// LogNetPackageMap —— 网络包映射日志（默认 Warning 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNetPackageMap, Warning, All);
// LogNetSerialization —— 网络序列化日志（默认 Warning 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogNetSerialization, Warning, All);
// LogMemory —— 内存分配日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogMemory, Log, All);
// LogProfilingDebugging —— 性能分析调试日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogProfilingDebugging, Log, All);
// LogCore —— Core 模块自身日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log, All);
// LogOutputDevice —— 输出设备日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogOutputDevice, Log, All);

// LogSHA —— SHA 哈希日志（默认 Warning 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogSHA, Warning, All);
// LogStats —— 统计系统日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogStats, Log, All);
// LogStreaming —— 流式加载日志（默认 Display 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogStreaming, Display, All);
// LogInit —— 引擎初始化日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogInit, Log, All);
// LogExit —— 引擎退出日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogExit, Log, All);
// LogExec —— 控制台命令执行日志（默认 Warning 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogExec, Warning, All);
// LogScript —— 脚本执行日志（默认 Warning 级）。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogScript, Warning, All);
// LogLocalization —— 本地化日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLocalization, Log, All);
// LogLongPackageNames —— 长包名日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLongPackageNames, Log, All);
// LogProcess —— 进程相关日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogProcess, Log, All);
// LogLoad —— 加载流程日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogLoad, Log, All);
// LogVirtualization —— 虚拟化（VA）日志。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogVirtualization, Log, All);

// 临时日志分类，一般不要把用到它的代码提交进仓库
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogTemp, Log, All);

// 平台专属日志分类，在这里声明是为了方便任何地方直接使用
// 需要再套一层宏，才能把「宏」当参数传给「宏」
// 定义辅助宏：把 A 原样转给 DECLARE_LOG_CATEGORY_EXTERN，让平台宏能当分类名。
#define DECLARE_LOG_CATEGORY_EXTERN_HELPER(A,B,C) DECLARE_LOG_CATEGORY_EXTERN(A,B,C)
// 如果平台定义了 PLATFORM_GLOBAL_LOG_CATEGORY（平台全局日志分类名）。
#ifdef PLATFORM_GLOBAL_LOG_CATEGORY
// 用辅助宏声明该平台全局日志分类。
	CORE_API DECLARE_LOG_CATEGORY_EXTERN_HELPER(PLATFORM_GLOBAL_LOG_CATEGORY, Log, All);
// 条件编译结束。
#endif
// 如果平台定义了 PLATFORM_GLOBAL_LOG_CATEGORY_ALT（备用平台日志分类名）。
#ifdef PLATFORM_GLOBAL_LOG_CATEGORY_ALT
// 用辅助宏声明该备用分类。
	CORE_API DECLARE_LOG_CATEGORY_EXTERN_HELPER(PLATFORM_GLOBAL_LOG_CATEGORY_ALT, Log, All);
// 条件编译结束。
#endif
```

**本段新元素**

- `DECLARE_LOG_CATEGORY_EXTERN(名字, 默认级别, 编译期最高级别)` —— 声明一个全局日志分类
- `Log` / `Warning` / `Display` —— 默认详细级别
- `All` —— 编译期允许的最高级别
- `DECLARE_LOG_CATEGORY_EXTERN_HELPER(A,B,C)` —— 辅助宏，让「宏」能当参数传给「宏」
- `PLATFORM_GLOBAL_LOG_CATEGORY` —— 平台自定义的全局日志分类名

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 29~39 行 | 11 个基础日志分类 |
| 第 41~52 行 | 13 个系统日志分类 |
| 第 55 行 | `LogTemp` 临时调试分类 |
| 第 59 行 | 辅助宏定义 |
| 第 60~65 行 | 平台专属分类（条件编译） |

> 类比：像给每个部门装一个喇叭：谁要喊话就用自己那个，喊多大声（级别）也定好了。

### 三、日志单例 + 启动耗时点（第 68~72 行）

```cpp
// 取全局日志重定向器单例（GLog 宏就是调它）。
CORE_API FOutputDeviceRedirector* GetGlobalLogSingleton();

// 打一个启动耗时点（Message 是阶段名）。
CORE_API void BootTimingPoint(const ANSICHAR *Message);

// 把所有启动耗时点汇总输出。
CORE_API void DumpBootTiming();
```

**本段新元素**

- `GetGlobalLogSingleton()` —— 取全局日志重定向器单例
- `BootTimingPoint(Message)` —— 打一个启动耗时点
- `DumpBootTiming()` —— 汇总输出所有耗时点

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 68 行 | 取日志单例 |
| 第 70 行 | 打耗时点 |
| 第 72 行 | 输出耗时汇总 |

> 类比：像跑步时按秒表：每到一个检查点按一下，最后一起看成绩。

### 四、`FScopedBootTiming`（启动耗时作用域）（第 74~81 行）

```cpp
// 启动耗时作用域：构造时记开始时间，析构时自动上报耗时。
struct FScopedBootTiming
{
// 阶段名。
	FString Message;
// 开始时间（秒）。
	double StartTime;
// 构造函数：只传阶段名。
	CORE_API FScopedBootTiming(const ANSICHAR *InMessage);
// 构造函数：阶段名 + 后缀（同名阶段可区分）。
	CORE_API FScopedBootTiming(const ANSICHAR *InMessage, FName Suffix);
// 析构函数：自动上报耗时。
	CORE_API ~FScopedBootTiming();
};
```

**本段新元素**

- `FScopedBootTiming` —— 构造时记开始时间，析构时自动上报耗时
- `FString Message` —— 阶段名
- `double StartTime` —— 开始时间（秒）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 76~77 行 | 两个成员 |
| 第 78~79 行 | 两个构造函数（带/不带后缀） |
| 第 80 行 | 析构函数自动上报 |

> 类比：像自动秒表：进作用域开始计时，出作用域自动停表并记录。

### 五、`FEngineTrackedActivityScope`（Insights 活动作用域）（第 83~88 行）

```cpp
// 引擎活动追踪作用域：把一段代码标记成一个可被 Insights 追踪的活动。
struct FEngineTrackedActivityScope
{
// 构造函数：printf 风格格式化字符串 + 可变参数。
	CORE_API FEngineTrackedActivityScope(const TCHAR* Fmt, ...);
// 构造函数：直接传 FString。
	CORE_API FEngineTrackedActivityScope(const FString& Str);
// 析构函数：结束活动。
	CORE_API ~FEngineTrackedActivityScope();
};
```

**本段新元素**

- `FEngineTrackedActivityScope` —— 把一段代码标记成 Unreal Insights 里可见的活动
- `const TCHAR* Fmt, ...` —— printf 风格格式化
- `const FString& Str` —— 直接传字符串

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 85 行 | printf 风格构造 |
| 第 86 行 | FString 构造 |
| 第 87 行 | 析构结束活动 |

> 类比：像给一段代码贴时间轴标签，事后在 Insights 里能看到它占多宽。

### 六、两个作用域宏（第 91~92 行）

```cpp
// 宏 SCOPED_BOOT_TIMING(x)：同时打 CPU 追踪事件 + 建启动耗时作用域，变量名自动生成。
#define SCOPED_BOOT_TIMING(x) TRACE_CPUPROFILER_EVENT_SCOPE_STR(x); FScopedBootTiming ANONYMOUS_VARIABLE(BootTiming_)(x);
// 宏 UE_SCOPED_ENGINE_ACTIVITY(Fmt, ...)：建引擎活动作用域，变量名自动生成。
#define UE_SCOPED_ENGINE_ACTIVITY(Fmt, ...) FEngineTrackedActivityScope ANONYMOUS_VARIABLE(EngineActivity_)(Fmt, ## __VA_ARGS__);
```

**本段新元素**

- `SCOPED_BOOT_TIMING(x)` —— 同时打 CPU 追踪事件 + 建启动耗时作用域
- `UE_SCOPED_ENGINE_ACTIVITY(Fmt, ...)` —— 建引擎活动作用域
- `ANONYMOUS_VARIABLE(前缀)` —— 生成唯一变量名，避免同一作用域重名
- `## __VA_ARGS__` —— 可变参数透传（GCC 扩展，空参数时去掉前面的逗号）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 91 行 | 启动耗时宏 |
| 第 92 行 | 引擎活动宏 |

> 类比：像自动命名：你只管写宏，变量名它自己起，不会撞车。

### 七、核心全局量（第 95~106 行）

```cpp
// 宏 GLog：取全局日志重定向器，等价于 GetGlobalLogSingleton()。
#define GLog GetGlobalLogSingleton()
// GConfig —— 全局配置缓存（读写 ini 用）。
extern CORE_API FConfigCacheIni* GConfig;
// GUndo —— 全局撤销事务对象。
extern CORE_API ITransaction* GUndo;
// GLogConsole —— 全局控制台输出设备。
extern CORE_API FOutputDeviceConsole* GLogConsole;
// GError —— 全局错误输出设备（弹错误框用）。
CORE_API extern class FOutputDeviceError*			GError;
// GWarn —— 全局警告/反馈上下文（弹对话框用）。
CORE_API extern class FFeedbackContext*				GWarn;


// GErrorHist —— 错误历史缓冲区（16384 个 TCHAR）。
extern CORE_API TCHAR GErrorHist[16384];

// #crashReport: 2014-08-19 合并成一个，重构。
// GErrorExceptionDescription —— 异常描述缓冲区（4096 个 TCHAR）。
extern CORE_API TCHAR GErrorExceptionDescription[4096];
```

**本段新元素**

- `GLog` —— 宏，等价于 `GetGlobalLogSingleton()`
- `GConfig` —— 全局配置缓存（读写 ini）
- `GUndo` —— 全局撤销事务
- `GLogConsole` —— 全局控制台输出设备
- `GError` —— 全局错误输出设备（弹错误框）
- `GWarn` —— 全局警告/反馈上下文（弹对话框）
- `GErrorHist[16384]` —— 错误历史缓冲区
- `GErrorExceptionDescription[4096]` —— 异常描述缓冲区

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 95 行 | `GLog` 宏 |
| 第 96~100 行 | 5 个核心全局指针 |
| 第 103 行 | 错误历史缓冲 |
| 第 106 行 | 异常描述缓冲 |

> 类比：像值班室的五部电话：配置、撤销、控制台、报错、警告，各管一摊。

### 八、`FCoreTexts`（本地化文本）（第 108~129 行）

```cpp
// FCoreTexts —— 引擎内置的本地化文本（True/False/Yes/No/None）。
struct FCoreTexts
{
// True 的本地化文本。
	const FText& True;
// False 的本地化文本。
	const FText& False;
// Yes 的本地化文本。
	const FText& Yes;
// No 的本地化文本。
	const FText& No;
// None 的本地化文本。
	const FText& None;

// 取全局唯一实例。
	static CORE_API const FCoreTexts& Get();

	/** 让已有引用失效。调用后不要再使用 FCoreTexts。 */
// 销毁全局实例。
	static CORE_API void TearDown();

	// 禁止拷贝
// 删除拷贝构造。
	FCoreTexts(const FCoreTexts&) = delete;
// 删除拷贝赋值。
	FCoreTexts& operator=(const FCoreTexts&) = delete;

// 构造函数：5 个文本全部传入。
	FCoreTexts(const FText& InTrue, const FText& InFalse, const FText& InYes, const FText& InNo, const FText& InNone)
// 初始化列表：逐个绑定引用。
		: True(InTrue), False(InFalse), Yes(InYes), No(InNo), None(InNone)
	{
	}
};
```

**本段新元素**

- `FCoreTexts` —— 引擎内置的 True/False/Yes/No/None 本地化文本
- `const FText&` —— 引用成员，指向全局实例里的文本
- `static const FCoreTexts& Get()` —— 取全局唯一实例
- `static void TearDown()` —— 销毁实例（之后引用全失效）
- `= delete` —— 删除拷贝构造/赋值，禁止拷贝

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 110~114 行 | 5 个 FText 引用 |
| 第 116 行 | 取全局实例 |
| 第 119 行 | 销毁实例 |
| 第 122~123 行 | 禁拷贝 |
| 第 125~128 行 | 构造函数 |

> 类比：像一本多语言小词典：True/False/Yes/No/None 五个词，按当前语言给。

### 九、旧版全局文本（已废弃）（第 131~142 行）

```cpp
// 如果没定义 DISABLE_LEGACY_CORE_TEXTS，或它被定义为 0（即仍启用旧全局文本）。
#if !defined(DISABLE_LEGACY_CORE_TEXTS) || DISABLE_LEGACY_CORE_TEXTS == 0
// 标记 GTrue 已废弃（4.23 起），改用 FCoreTexts::Get().True。
UE_DEPRECATED(4.23, "GTrue has been deprecated in favor of FCoreTexts::Get().True.")
// GTrue —— 旧版全局 True 文本（已废弃）。
extern CORE_API const FText GTrue;
// 标记 GFalse 已废弃（4.23 起）。
UE_DEPRECATED(4.23, "GFalse has been deprecated in favor of FCoreTexts::Get().False.")
// GFalse —— 旧版全局 False 文本（已废弃）。
extern CORE_API const FText GFalse;
// 标记 GYes 已废弃（4.23 起）。
UE_DEPRECATED(4.23, "GYes has been deprecated in favor of FCoreTexts::Get().Yes.")
// GYes —— 旧版全局 Yes 文本（已废弃）。
extern CORE_API const FText GYes;
// 标记 GNo 已废弃（4.23 起）。
UE_DEPRECATED(4.23, "GNo has been deprecated in favor of FCoreTexts::Get().No.")
// GNo —— 旧版全局 No 文本（已废弃）。
extern CORE_API const FText GNo;
// 标记 GNone 已废弃（4.23 起）。
UE_DEPRECATED(4.23, "GNone has been deprecated in favor of FCoreTexts::Get().None.")
// GNone —— 旧版全局 None 文本（已废弃）。
extern CORE_API const FText GNone;
// 条件编译结束。
#endif
```

**本段新元素**

- `UE_DEPRECATED(版本, "提示")` —— 标记已废弃，编译时给警告
- `DISABLE_LEGACY_CORE_TEXTS` —— 关掉旧全局文本的开关
- `GTrue` / `GFalse` / `GYes` / `GNo` / `GNone` —— 5 个旧版全局文本

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 131 行 | 条件编译 |
| 第 132~141 行 | 5 个废弃变量 + 5 个废弃标记 |
| 第 142 行 | 条件编译结束 |

> 类比：像老式电话亭：还在，但门口贴了「请用手机」的告示。

### 十、蓝图/对象/嵌入相关开关（第 144~194 行）

```cpp
/** 为 true 时，这个可执行文件能运行所有游戏（游戏以 DLL 形式加载）。 */
// GIsGameAgnosticExe —— 是否「游戏无关」的可执行文件。
extern CORE_API bool GIsGameAgnosticExe;

/** 从游戏里保存时，这个开关允许加载「仅编辑器」属性。 */
// GForceLoadEditorOnly —— 强制加载仅编辑器属性。
extern CORE_API bool GForceLoadEditorOnly;

/** 禁止加载不在脚本文件里的对象；脚本编译期间用。 */
// GVerifyObjectReferencesOnly —— 只校验对象引用。
extern CORE_API bool GVerifyObjectReferencesOnly;

/** 构造对象时，在主机平台走快速路径…… */
// GFastPathUniqueNameGeneration —— 唯一名生成走快速路径。
extern CORE_API bool GFastPathUniqueNameGeneration;

/** 允许 AActor 在编辑器里从特定入口执行脚本，例如跑构造脚本时。 */
// GAllowActorScriptExecutionInEditor —— 允许编辑器内执行 Actor 脚本。
extern CORE_API bool GAllowActorScriptExecutionInEditor;

/** 给 CDO 里新实例化的组件强制使用模板名。 */
// GCompilingBlueprint —— 是否正在编译蓝图。
extern CORE_API bool GCompilingBlueprint;

/** 蓝图编译后是否正在做垃圾回收。 */
// GIsGCingAfterBlueprintCompile —— 蓝图编译后正在 GC。
extern CORE_API bool GIsGCingAfterBlueprintCompile;

/** 是否正在重建蓝图实例；cook 后的构建里永远不该为 true。 */
// GIsReconstructingBlueprintInstances —— 正在重建蓝图实例。
extern CORE_API bool GIsReconstructingBlueprintInstances;

/** Actor 和对象是否正在被重新实例化。 */
// GIsReinstancing —— 正在重新实例化（原子布尔，多线程可见）。
extern CORE_API std::atomic<bool> GIsReinstancing;

/** 用于冲刷资源流式加载的辅助函数。 */
// GFlushStreamingFunc —— 冲刷流式加载的函数指针。
extern CORE_API void(*GFlushStreamingFunc)(void);

/** UE-as-a-library（把引擎当库嵌入）功能用的设置。 */
// FUELibraryOverrideSettings —— 嵌入模式下的覆盖设置。
struct FUELibraryOverrideSettings
{
	/** 是否通过 UELibrary 初始化。若为 false，其他字段的值都不应被采信。
	    （原文第二行：none of the other field values should be acknowledged.） */
// bIsEmbedded —— 是否嵌入模式，默认 false。
	bool bIsEmbedded = false;

	/** 把引擎嵌入到哪个窗口句柄 */
// WindowHandle —— 宿主窗口句柄，默认空。
	void* WindowHandle = nullptr;

	/** 覆盖后的嵌入视口宽度 */
// WindowWidth —— 视口宽度，默认 0。
	int32 WindowWidth = 0;

	/** 覆盖后的嵌入视口高度 */
// WindowHeight —— 视口高度，默认 0。
	int32 WindowHeight = 0;
};

/** 把 UE 当库使用时的设置 */
// GUELibraryOverrideSettings —— 全局嵌入设置实例。
extern CORE_API FUELibraryOverrideSettings GUELibraryOverrideSettings;

// GIsRunningUnattendedScript —— 是否在跑无人值守脚本。
extern CORE_API bool GIsRunningUnattendedScript;
```

**本段新元素**

- `GIsGameAgnosticExe` —— 是否「游戏无关」的可执行文件
- `GForceLoadEditorOnly` —— 强制加载仅编辑器属性
- `GVerifyObjectReferencesOnly` —— 只校验对象引用
- `GFastPathUniqueNameGeneration` —— 唯一名生成走快速路径
- `GAllowActorScriptExecutionInEditor` —— 允许编辑器内执行 Actor 脚本
- `GCompilingBlueprint` —— 是否正在编译蓝图
- `GIsGCingAfterBlueprintCompile` —— 蓝图编译后正在 GC
- `GIsReconstructingBlueprintInstances` —— 正在重建蓝图实例
- `std::atomic<bool> GIsReinstancing` —— 正在重新实例化（原子）
- `GFlushStreamingFunc` —— 冲刷流式加载的函数指针
- `FUELibraryOverrideSettings` —— UE-as-a-library 嵌入设置
- `GUELibraryOverrideSettings` —— 全局嵌入设置实例
- `GIsRunningUnattendedScript` —— 是否在跑无人值守脚本

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 145~172 行 | 9 个蓝图/对象开关 + 1 个函数指针 |
| 第 175~189 行 | `FUELibraryOverrideSettings` 结构（4 字段） |
| 第 192 行 | 全局嵌入设置 |
| 第 194 行 | 无人值守脚本标志 |

> 类比：像一排电闸：编译蓝图、重建实例、重新实例化，各有一个开关。

### 十一、commandlet 私有标志（第 196~205 行）

```cpp
// 如果编了引擎（WITH_ENGINE）。
#if WITH_ENGINE
// PRIVATE_GIsRunningCommandlet —— 是否在跑命令行工具（commandlet）。
extern CORE_API bool PRIVATE_GIsRunningCommandlet;
// PRIVATE_GRunningCommandletClass —— 正在跑的 commandlet 的 UClass。
extern CORE_API UClass* PRIVATE_GRunningCommandletClass;

/** 为 true 时，即使跑 commandlet 也初始化 RHI 并搭渲染场景。 */
// PRIVATE_GAllowCommandletRendering —— commandlet 下允许渲染。
extern CORE_API bool PRIVATE_GAllowCommandletRendering;

/** 为 true 时，即使跑 commandlet 也初始化音频。 */
// PRIVATE_GAllowCommandletAudio —— commandlet 下允许音频。
extern CORE_API bool PRIVATE_GAllowCommandletAudio;
// 条件编译结束。
#endif
```

**本段新元素**

- `WITH_ENGINE` —— 是否编了引擎
- `PRIVATE_GIsRunningCommandlet` —— 是否在跑 commandlet
- `PRIVATE_GRunningCommandletClass` —— 正在跑的 commandlet 的 UClass
- `PRIVATE_GAllowCommandletRendering` —— commandlet 下允许渲染
- `PRIVATE_GAllowCommandletAudio` —— commandlet 下允许音频
- `PRIVATE_` 前缀 —— 表示「内部用，外部请走 IsRunningCommandlet()」

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 197~198 行 | 2 个基础标志 |
| 第 201 行 | 渲染开关 |
| 第 204 行 | 音频开关 |

> 类比：像后台开关箱：标着 PRIVATE，请从前面板（函数）操作。

### 十二、`GIsEditor` 三态定义（第 207~237 行）

```cpp
// 如果编了「仅编辑器数据」（WITH_EDITORONLY_DATA）。
#if WITH_EDITORONLY_DATA

/**
*	是否在编辑器里。
*	注意：PIE（Play In Editor）时它仍为 true；那种情况应改用 GWorld->HasBegunPlay。
*/
// GIsEditor —— 是否在编辑器里。
extern CORE_API bool GIsEditor;
// GIsImportingT3D —— 是否正在导入 T3D。
extern CORE_API bool GIsImportingT3D;
// GIsTransacting —— 是否正在事务（撤销/重做）中。
extern CORE_API bool GIsTransacting;

/** 表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。
UI 泵消息循环自然返回前，游戏线程做不了任何事。
*/
// GIntraFrameDebuggingGameThread —— 是否处于帧内调试（游戏线程被暂停）。
extern CORE_API bool			GIntraFrameDebuggingGameThread;
/** 是否是第一次进入 UI 消息泵循环。 */
// GFirstFrameIntraFrameDebugging —— 是否首帧帧内调试。
extern CORE_API bool			GFirstFrameIntraFrameDebugging;

// 否则如果是在做代码分析（USING_CODE_ANALYSIS）。
#elif USING_CODE_ANALYSIS

// 代码分析期间定义成变量，避免大量「常量 && 表达式」警告
// GIsEditor —— 代码分析期用变量形式。
extern CORE_API bool GIsEditor;
// GIntraFrameDebuggingGameThread —— 代码分析期用变量形式。
extern CORE_API bool GIntraFrameDebuggingGameThread;
// GFirstFrameIntraFrameDebugging —— 代码分析期用变量形式。
extern CORE_API bool GFirstFrameIntraFrameDebugging;

// 否则（普通运行时）。
#else

// GIsEditor 直接定义成常量 false。
#define GIsEditor								false
// GIntraFrameDebuggingGameThread 直接定义成常量 false。
#define GIntraFrameDebuggingGameThread			false
// GFirstFrameIntraFrameDebugging 直接定义成常量 false。
#define GFirstFrameIntraFrameDebugging			false

#endif // WITH_EDITORONLY_DATA
```

**本段新元素**

- `WITH_EDITORONLY_DATA` —— 是否编了仅编辑器数据
- `USING_CODE_ANALYSIS` —— 是否在做静态代码分析
- `GIsEditor` —— 是否在编辑器里（三种定义方式）
- `GIntraFrameDebuggingGameThread` —— 游戏线程是否被暂停做帧内调试
- `GFirstFrameIntraFrameDebugging` —— 是否首帧帧内调试
- `#define GIsEditor false` —— 运行时直接定义成常量，零开销

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 213~222 行 | 编辑器构建：真实变量 |
| 第 227~229 行 | 代码分析：变量形式（避免警告） |
| 第 233~235 行 | 运行时：宏定义成 false |

> 类比：像同一个开关三种装法：编辑器里是真开关，分析时是假开关，运行时干脆焊死。

### 十三、commandlet 判断函数（6 个）（第 239~325 行）

```cpp
// 如果编了编辑器（WITH_EDITOR）。
#if WITH_EDITOR
// PRIVATE_GIsRunningCookCommandlet —— 是否在跑 cook commandlet。
extern CORE_API bool PRIVATE_GIsRunningCookCommandlet;
// PRIVATE_GIsRunningDLCCookCommandlet —— 是否在跑 DLC cook commandlet。
extern CORE_API bool PRIVATE_GIsRunningDLCCookCommandlet;
// 条件编译结束。
#endif

/**
* 检查这个可执行文件是否在跑 commandlet（编辑器式环境下的自定义命令行处理）。
*/
// IsRunningCommandlet —— 是否在跑 commandlet。
FORCEINLINE bool IsRunningCommandlet()
{
// 如果编了引擎。
#if WITH_ENGINE
// 返回引擎版的实际标志。
	return PRIVATE_GIsRunningCommandlet;
// 否则。
#else
// 没有引擎就永远 false。
	return false;
// 条件编译结束。
#endif
}

/**
* 检查这个可执行文件是否在跑 cook commandlet。
*/
// IsRunningCookCommandlet —— 是否在跑 cook commandlet。
FORCEINLINE bool IsRunningCookCommandlet()
{
// 如果编了编辑器。
#if WITH_EDITOR
// 返回编辑器版的实际标志。
	return PRIVATE_GIsRunningCookCommandlet;
// 否则。
#else
// 没有编辑器就永远 false。
	return false;
// 条件编译结束。
#endif
}

/**
* 检查这个可执行文件是否在跑 DLC cook commandlet。
*/
// IsRunningDLCCookCommandlet —— 是否在跑 DLC cook commandlet。
FORCEINLINE bool IsRunningDLCCookCommandlet()
{
// 如果编了编辑器。
#if WITH_EDITOR
// 返回编辑器版的实际标志。
	return PRIVATE_GIsRunningDLCCookCommandlet;
// 否则。
#else
// 没有编辑器就永远 false。
	return false;
// 条件编译结束。
#endif
}

/**
 * 返回当前正在跑的 commandlet 的 UClass。
 */
// GetRunningCommandletClass —— 取当前 commandlet 的 UClass。
FORCEINLINE UClass* GetRunningCommandletClass()
{
// 如果编了引擎。
#if WITH_ENGINE
// 返回引擎版的实际指针。
	return PRIVATE_GRunningCommandletClass;
// 否则。
#else
// 没有引擎就返回空指针。
	return nullptr;
// 条件编译结束。
#endif
}

/**
 * 返回命令行参数 -Run=<CommandletName> 指定的 commandlet 名字。
 * 参数形式：-Run=<CommandletName>。
 * 
 * 给需要在引擎启动早期、commandlet 还没初始化时就知道名字的代码用。
 * （原文：early on during engine startup, before the commandlet has been initialized.）
 * 能用 GetRunningCommandletClass()->GetName() 就优先用它。
 * （原文：If possible, code should prefer calling ... instead.）
 * 
 * @return 最终会运行的 commandlet 名字。
 * 没有 commandlet 则返回空字符串。
 */
// GetCommandletNameFromCmdline —— 从命令行取 commandlet 名字。
CORE_API FString GetCommandletNameFromCmdline();

/**
 * 检查跑 commandlet 时是否也要初始化 RHI 并搭渲染场景。
 */
// IsAllowCommandletRendering —— commandlet 下是否允许渲染。
FORCEINLINE bool IsAllowCommandletRendering()
{
// 如果编了引擎。
#if WITH_ENGINE
// 返回引擎版的实际标志。
	return PRIVATE_GAllowCommandletRendering;
// 否则。
#else
// 没有引擎就永远 false。
	return false;
// 条件编译结束。
#endif
}

// IsAllowCommandletAudio —— commandlet 下是否允许音频。
FORCEINLINE bool IsAllowCommandletAudio()
{
// 如果编了引擎。
#if WITH_ENGINE
// 返回引擎版的实际标志。
	return PRIVATE_GAllowCommandletAudio;
// 否则。
#else
// 没有引擎就永远 false。
	return false;
// 条件编译结束。
#endif
}
```

**本段新元素**

- `FORCEINLINE` —— 强制内联，零调用开销
- `IsRunningCommandlet()` —— 是否在跑 commandlet
- `IsRunningCookCommandlet()` —— 是否在跑 cook commandlet
- `IsRunningDLCCookCommandlet()` —— 是否在跑 DLC cook commandlet
- `GetRunningCommandletClass()` —— 取当前 commandlet 的 UClass
- `GetCommandletNameFromCmdline()` —— 从命令行取 commandlet 名字
- `IsAllowCommandletRendering()` —— commandlet 下是否允许渲染
- `IsAllowCommandletAudio()` —— commandlet 下是否允许音频

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 247~254 行 | `IsRunningCommandlet` |
| 第 259~266 行 | `IsRunningCookCommandlet` |
| 第 271~278 行 | `IsRunningDLCCookCommandlet` |
| 第 283~290 行 | `GetRunningCommandletClass` |
| 第 304 行 | `GetCommandletNameFromCmdline` |
| 第 309~325 行 | `IsAllowCommandletRendering` / `IsAllowCommandletAudio` |

> 类比：像前台查询台：六个窗口，问「现在是不是在跑 XX」都能答。

### 十四、多进程 ID（第 327~342 行）

```cpp
// 命名空间 UE：引擎新版公共 API 都放这里。
namespace UE
{
/**
 * 多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0。
 * 单进程运行时 GetMultiprocessId 返回 0。
 * 基于命令行判断，典型场景是多进程 cook 的 cookworker。
 * 需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。
 * （原文：Systems that need to write a separate file per running process should use this id as part of their file descriptor.）
 */
// GetMultiprocessId —— 取当前进程的多进程编号。
CORE_API int32 GetMultiprocessId();
// 命名空间结束。
}

// 命名空间 UE::Private：引擎内部实现细节，外部不要直接调。
namespace UE::Private
{
// SetMultiprocessId —— 设置当前进程的多进程编号（内部用）。
CORE_API void SetMultiprocessId(int32 MultiprocessId);
// 命名空间结束。
}
```

**本段新元素**

- `namespace UE` —— 引擎新版公共 API 命名空间
- `UE::GetMultiprocessId()` —— 取当前进程的多进程编号
- `namespace UE::Private` —— 内部实现细节
- `UE::Private::SetMultiprocessId()` —— 设置编号（内部用）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 336 行 | 公开 getter |
| 第 341 行 | 内部 setter |

> 类比：像给每个工人发工牌号：导演是 0 号，工人是 1、2、3…，单干时也是 0 号。

### 十五、重新实例化包装类 + 客户端/服务器标志（第 344~356 行）

```cpp
// FIsDuplicatingClassForReinstancing —— 可隐式转 bool 的包装类，用来标记「正在为重新实例化复制类」。
class FIsDuplicatingClassForReinstancing
{
// 公有区。
public:
// 赋值运算符：接受 bool，返回自身引用，支持链式赋值。
	CORE_API FIsDuplicatingClassForReinstancing& operator= (bool bOther);
// 类型转换运算符：隐式转成 bool。
	CORE_API operator bool() const;
// 类结束。
};

// GEdSelectionLock —— 编辑器选择是否被锁定。
extern CORE_API bool GEdSelectionLock;
// GIsClient —— 当前是否是客户端。
extern CORE_API bool GIsClient;
// GIsServer —— 当前是否是服务器。
extern CORE_API bool GIsServer;
// GIsCriticalError —— 是否发生了致命错误（TSAN 原子布尔）。
extern CORE_API TSAN_ATOMIC(bool) GIsCriticalError;
// GIsRunning —— 引擎是否在运行（TSAN 原子布尔）。
extern CORE_API TSAN_ATOMIC(bool) GIsRunning;
// GIsDuplicatingClassForReinstancing —— 是否正在为重新实例化复制类。
extern CORE_API FIsDuplicatingClassForReinstancing GIsDuplicatingClassForReinstancing;
```

**本段新元素**

- `FIsDuplicatingClassForReinstancing` —— 可隐式转 bool 的包装类
- `operator=` —— 赋值运算符，接受 bool
- `operator bool()` —— 类型转换运算符
- `GEdSelectionLock` —— 编辑器选择是否锁定
- `GIsClient` / `GIsServer` —— 客户端 / 服务器标志
- `TSAN_ATOMIC(bool)` —— ThreadSanitizer 友好的原子布尔

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 347~348 行 | 两个运算符重载 |
| 第 351~353 行 | 3 个普通 bool |
| 第 354~356 行 | 3 个原子/包装类型 |

> 类比：像带锁的开关：包装类让你能像用 bool 一样用它，但内部可以加逻辑。

### 十六、启动/退出/截图相关（第 358~427 行）

```cpp
/**
* 下面这些在引擎首次启动时设置。
*/

/**
* 指定引擎是否作为构建机进程启动。
*/
// GIsBuildMachine —— 是否是构建机进程。
extern CORE_API bool GIsBuildMachine;

/**
* 决定是否输出日志文本。为 true 时不输出任何日志。
*/
// GIsSilent —— 静默模式（不输出日志）。
extern CORE_API bool GIsSilent;

// GIsSlowTask —— 是否有慢任务在进行（会弹进度条）。
extern CORE_API bool GIsSlowTask;
// GSlowTaskOccurred —— 是否发生过慢任务。
extern CORE_API bool GSlowTaskOccurred;
// GIsGuarded —— 是否处于异常保护块内。
extern CORE_API bool GIsGuarded;

/**
* 设为 true 后，只允许在引擎 tick 开始时设置 RequestingExit，
* 可消除设置 RequestExit 时的未定义行为风险。
*
* 这个开关需要在所有平台/用法上验证过，才能改成默认值。
*/
// 如果没定义 UE_SET_REQUEST_EXIT_ON_TICK_ONLY。
#ifndef UE_SET_REQUEST_EXIT_ON_TICK_ONLY
// 默认定义为 0（关闭）。
	#define UE_SET_REQUEST_EXIT_ON_TICK_ONLY 0
// 条件编译结束。
#endif

/**
 * 引擎启动时所有初始模块完成各自加载阶段后返回 true；
 * 对应事件 FCoreDelegates::OnAllModuleLoadingPhasesComplete。
 */
// IsEngineStartupModuleLoadingComplete —— 启动模块加载是否全部完成。
extern CORE_API bool IsEngineStartupModuleLoadingComplete();
// SetEngineStartupModuleLoadingComplete —— 标记启动模块加载完成。
extern CORE_API void SetEngineStartupModuleLoadingComplete();

// 标记 GIsRequestingExit 已废弃（4.24 起），改用 IsEngineExitRequested() / RequestEngineExit()。
UE_DEPRECATED(4.24, "Please use IsEngineExitRequested()/RequestEngineExit(const FString&)")
// GIsRequestingExit —— 是否已请求退出引擎（已废弃）。
extern CORE_API bool GIsRequestingExit;

/**
 * 检查是否收到退出请求，收到就把 GIsRequestingExit 置位。
 */
// BeginExitIfRequested —— 有退出请求就开始退出流程。
extern CORE_API void BeginExitIfRequested();

// IsEngineExitRequested —— 是否已请求退出引擎。
FORCEINLINE bool IsEngineExitRequested()
{
// 临时关闭废弃警告（下面要读已废弃的 GIsRequestingExit）。
PRAGMA_DISABLE_DEPRECATION_WARNINGS
// 返回退出请求标志。
	return GIsRequestingExit;
// 恢复废弃警告。
PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

// 请求引擎在能安全退出时尽快退出
// ReasonString 不是可选的，要写清为什么请求退出
// RequestEngineExit —— 请求退出（C 字符串版）。
CORE_API void RequestEngineExit(const TCHAR* ReasonString);
// RequestEngineExit —— 请求退出（FString 版）。
CORE_API void RequestEngineExit(const FString& ReasonString);

/**
*	全局开关，控制是否显示屏幕上的警告/消息。
*	控制台命令 DISABLEALLSCREENMESSAGES 关闭。
*	控制台命令 ENABLEALLSCREENMESSAGES 打开。
*	控制台命令 TOGGLEALLSCREENMESSAGES 切换。
*/
// GAreScreenMessagesEnabled —— 屏幕消息是否启用。
extern CORE_API bool GAreScreenMessagesEnabled;
// GScreenMessagesRestoreState —— 恢复用的原状态。
extern CORE_API bool GScreenMessagesRestoreState;

/* 是否正在截屏/录屏 */
// GIsDumpingMovie —— 是否在导出序列帧（0 表示不导出）。
extern CORE_API int32 GIsDumpingMovie;
// GIsHighResScreenshot —— 是否在做高分辨率截图。
extern CORE_API bool GIsHighResScreenshot;
// GScreenshotResolutionX —— 截图宽度。
extern CORE_API uint32 GScreenshotResolutionX;
// GScreenshotResolutionY —— 截图高度。
extern CORE_API uint32 GScreenshotResolutionY;
// GMakeCacheIDIndex —— 缓存 ID 生成计数器。
extern CORE_API uint64 GMakeCacheIDIndex;
```

**本段新元素**

- `GIsBuildMachine` —— 是否构建机进程
- `GIsSilent` —— 静默模式（不输出日志）
- `GIsSlowTask` / `GSlowTaskOccurred` / `GIsGuarded` —— 慢任务/异常保护
- `UE_SET_REQUEST_EXIT_ON_TICK_ONLY` —— 只在 tick 开始允许请求退出
- `IsEngineStartupModuleLoadingComplete()` —— 启动模块加载是否完成
- `GIsRequestingExit` —— 是否已请求退出（已废弃）
- `IsEngineExitRequested()` —— 是否已请求退出（新 API）
- `RequestEngineExit(ReasonString)` —— 请求退出
- `GAreScreenMessagesEnabled` —— 屏幕消息是否启用
- `GIsDumpingMovie` / `GIsHighResScreenshot` —— 导出序列帧 / 高清截图
- `GScreenshotResolutionX/Y` —— 截图分辨率
- `GMakeCacheIDIndex` —— 缓存 ID 计数器

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 365~374 行 | 启动状态标志 |
| 第 382~384 行 | 退出时机开关 |
| 第 390~391 行 | 模块加载完成 |
| 第 394~411 行 | 退出请求（新旧 API） |
| 第 419~427 行 | 屏幕消息 + 截图 |

> 类比：像飞机的仪表盘：启动、退出、截图，每个状态一个灯。

### 十七、各种 ini 路径（第 429~457 行）

```cpp
// GEngineIni —— 引擎主 ini 文件路径。
extern CORE_API FString GEngineIni;

/** 编辑器 ini 位置，按引擎版本存（所有项目共享），首次运行时在版本间迁移。 */
// GEditorLayoutIni —— 编辑器布局 ini。
extern CORE_API FString GEditorLayoutIni;
// GEditorKeyBindingsIni —— 编辑器快捷键 ini。
extern CORE_API FString GEditorKeyBindingsIni;
// GEditorSettingsIni —— 编辑器设置 ini。
extern CORE_API FString GEditorSettingsIni;

/** 编辑器「按项目」的 ini 文件，按项目存。 */
// GEditorIni —— 编辑器项目 ini。
extern CORE_API FString GEditorIni;
// GEditorPerProjectIni —— 编辑器每项目独立 ini。
extern CORE_API FString GEditorPerProjectIni;

// GCompatIni —— 兼容性 ini。
extern CORE_API FString GCompatIni;
// GLightmassIni —— Lightmass 光照烘焙 ini。
extern CORE_API FString GLightmassIni;
// GScalabilityIni —— 画质可伸缩性 ini。
extern CORE_API FString GScalabilityIni;
// GHardwareIni —— 硬件配置 ini。
extern CORE_API FString GHardwareIni;
// GInputIni —— 输入配置 ini。
extern CORE_API FString GInputIni;
// GGameIni —— 游戏配置 ini。
extern CORE_API FString GGameIni;
// GGameUserSettingsIni —— 玩家用户设置 ini。
extern CORE_API FString GGameUserSettingsIni;
// GRuntimeOptionsIni —— 运行时选项 ini。
extern CORE_API FString GRuntimeOptionsIni;
// GInstallBundleIni —— 安装包 ini。
extern CORE_API FString GInstallBundleIni;
// GDeviceProfilesIni —— 设备配置档 ini。
extern CORE_API FString GDeviceProfilesIni;
// GGameplayTagsIni —— GameplayTag 配置 ini。
extern CORE_API FString GGameplayTagsIni;

// GNearClippingPlane —— 近裁剪面距离。
extern CORE_API float GNearClippingPlane;
// GNearClippingPlane_RenderThread —— 渲染线程用的近裁剪面距离。
extern CORE_API float GNearClippingPlane_RenderThread;

// GExitPurge —— 退出时是否做彻底清理。
extern CORE_API bool GExitPurge;
// GInternalProjectName —— 内部项目名（最多 64 个 TCHAR）。
extern CORE_API TCHAR GInternalProjectName[64];
// GForeignEngineDir —— 外部引擎目录（跨引擎引用时用）。
extern CORE_API const TCHAR* GForeignEngineDir;
```

**本段新元素**

- `GEngineIni` —— 引擎主 ini
- `GEditorLayoutIni` / `GEditorKeyBindingsIni` / `GEditorSettingsIni` —— 编辑器布局/快捷键/设置（按引擎版本存）
- `GEditorIni` / `GEditorPerProjectIni` —— 编辑器项目 ini（按项目存）
- `GCompatIni` / `GLightmassIni` / `GScalabilityIni` / `GHardwareIni` —— 兼容/光照/画质/硬件
- `GInputIni` / `GGameIni` / `GGameUserSettingsIni` —— 输入/游戏/玩家设置
- `GRuntimeOptionsIni` / `GInstallBundleIni` / `GDeviceProfilesIni` / `GGameplayTagsIni` —— 运行时/安装包/设备档/标签
- `GNearClippingPlane` —— 近裁剪面
- `GExitPurge` —— 退出时彻底清理
- `GInternalProjectName[64]` —— 内部项目名
- `GForeignEngineDir` —— 外部引擎目录

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 429 行 | 引擎主 ini |
| 第 432~438 行 | 编辑器 ini（6 个） |
| 第 440~450 行 | 系统 ini（11 个） |
| 第 452~457 行 | 裁剪面 / 退出清理 / 项目名 / 外部引擎 |

> 类比：像一柜子文件夹：每个系统一个 ini，找配置先看名字。

### 十八、异步加载 + 加载器类型（第 459~494 行）

```cpp
/** 游戏调试工具的 exec 处理器，支持 editactor 这类命令 */
// GDebugToolExec —— 调试工具 exec 处理器。
extern CORE_API FExec* GDebugToolExec;

/** 当前是否在异步加载代码路径里 */
// IsAsyncLoading —— 函数指针，判断是否在异步加载。
extern CORE_API bool(*IsAsyncLoading)();

/** 暂停异步包加载。 */
// SuspendAsyncLoading —— 暂停异步加载的函数指针。
extern CORE_API void (*SuspendAsyncLoading)();

/** 恢复异步包加载。 */
// ResumeAsyncLoading —— 恢复异步加载的函数指针。
extern CORE_API void (*ResumeAsyncLoading)();

/** 暂停异步包加载（引擎原文如此，实际是查询是否暂停）。 */
// IsAsyncLoadingSuspended —— 查询异步加载是否暂停。
extern CORE_API bool (*IsAsyncLoadingSuspended)();

/** 异步加载是否使用独立线程 */
// IsAsyncLoadingMultithreaded —— 查询异步加载是否多线程。
extern CORE_API bool(*IsAsyncLoadingMultithreaded)();

// ELoaderType —— 当前使用的包加载器类型。
enum class ELoaderType : uint8
{
// NotInitialized —— 还没初始化。
	NotInitialized,
// LegacyLoader —— 旧版加载器。
	LegacyLoader,
// EditorPackageLoader —— 编辑器包加载器。
	EditorPackageLoader,
// ZenLoader —— Zen 加载器（新存储后端）。
	ZenLoader
};

// LexToString —— 把 ELoaderType 转成字符串。
CORE_API const TCHAR* LexToString(ELoaderType Type);

/** 返回当前激活的加载器类型，没有则返回默认值。 */
// GetLoaderType —— 函数指针，取当前加载器类型。
extern CORE_API ELoaderType(*GetLoaderType)();

/** 暂停「异步 IO 完成后触发的纹理更新」。 */
// SuspendTextureStreamingRenderTasks —— 暂停纹理流式渲染任务。
extern CORE_API void (*SuspendTextureStreamingRenderTasks)();

/** 恢复「异步 IO 完成后触发的纹理更新」。 */
// ResumeTextureStreamingRenderTasks —— 恢复纹理流式渲染任务。
extern CORE_API void (*ResumeTextureStreamingRenderTasks)();
```

**本段新元素**

- `GDebugToolExec` —— 调试工具 exec 处理器
- `IsAsyncLoading` —— 函数指针，判断是否在异步加载
- `SuspendAsyncLoading` / `ResumeAsyncLoading` —— 暂停/恢复异步加载
- `IsAsyncLoadingSuspended` —— 查询是否暂停
- `IsAsyncLoadingMultithreaded` —— 是否多线程异步加载
- `enum class ELoaderType : uint8` —— 加载器类型（4 个值）
- `LexToString(ELoaderType)` —— 枚举转字符串
- `GetLoaderType` —— 函数指针，取当前加载器
- `SuspendTextureStreamingRenderTasks` / `ResumeTextureStreamingRenderTasks` —— 暂停/恢复纹理流式任务

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 460 行 | 调试 exec |
| 第 463~475 行 | 5 个异步加载函数指针 |
| 第 477~483 行 | `ELoaderType` 枚举 |
| 第 485~488 行 | 枚举转字符串 + 取当前加载器 |
| 第 491~494 行 | 纹理流式任务开关 |

> 类比：像换轮胎：旧加载器、编辑器加载器、Zen 加载器，随时能换，换的时候先暂停。

### 十九、编辑器加载包 / PIE 实例号（第 496~538 行）

```cpp
// FIsEditorLoadingPackage —— 已废弃（5.5），可隐式转 bool 的包装类。
class UE_DEPRECATED(5.5, "This will be removed.") FIsEditorLoadingPackage
{
// 公有区。
public:
// 赋值运算符：接受 bool。
	CORE_API FIsEditorLoadingPackage& operator= (bool InValue);
// 类型转换运算符：隐式转 bool。
	CORE_API operator bool() const;
// 类结束。
};

// 临时关闭废弃警告（下面要用已废弃的类型）。
PRAGMA_DISABLE_DEPRECATION_WARNINGS
/** 编辑器当前是否正在加载包 */
// 标记 GIsEditorLoadingPackage 已废弃（5.5），改用 UE::GetIsEditorLoadingPackage() / SetIsEditorLoadingPackage()。
UE_DEPRECATED(5.5, "Use UE::GetIsEditorLoadingPackage() and UE::SetIsEditorLoadingPackage() instead.")
// GIsEditorLoadingPackage —— 编辑器是否正在加载包（已废弃）。
extern CORE_API FIsEditorLoadingPackage GIsEditorLoadingPackage;
// 恢复废弃警告。
PRAGMA_ENABLE_DEPRECATION_WARNINGS

// 命名空间 UE。
namespace UE
{
// GetIsEditorLoadingPackage —— 查询编辑器是否正在加载包。
	CORE_API bool GetIsEditorLoadingPackage();
// SetIsEditorLoadingPackage —— 设置该标志。
	CORE_API void SetIsEditorLoadingPackage(bool InValue);
// 命名空间结束。
}

/** cooker 当前是否正在加载包 */
// GIsCookerLoadingPackage —— cooker 是否正在加载包。
extern CORE_API bool GIsCookerLoadingPackage;

/** GWorld 是否指向 PIE（Play In Editor）世界 */
// GIsPlayInEditorWorld —— 是否在 PIE 世界里。
extern CORE_API bool GIsPlayInEditorWorld;

// FPlayInEditorID —— 已废弃（5.5），可隐式转 int32 的包装类。
class UE_DEPRECATED(5.5, "This will be removed.") FPlayInEditorID
{
// 公有区。
public:
// 赋值运算符：接受 int32。
	CORE_API FPlayInEditorID& operator= (int32 InOther);
// 类型转换运算符：隐式转 int32。
	CORE_API operator int32() const;
// 类结束。
};

// 临时关闭废弃警告。
PRAGMA_DISABLE_DEPRECATION_WARNINGS
/** 编辑器里表示当前正在处理的 PIE 世界上下文实例号 */
// 标记 GPlayInEditorID 已废弃（5.5），改用 UE::GetPlayInEditorID() / SetPlayInEditorID()。
UE_DEPRECATED(5.5, "Use UE::GetPlayInEditorID() and UE::SetPlayInEditorID() instead.")
// GPlayInEditorID —— 当前 PIE 实例号（已废弃）。
extern CORE_API FPlayInEditorID GPlayInEditorID;
// 恢复废弃警告。
PRAGMA_ENABLE_DEPRECATION_WARNINGS

// 命名空间 UE。
namespace UE
{
// GetPlayInEditorID —— 取当前 PIE 实例号。
	CORE_API int32 GetPlayInEditorID();
// SetPlayInEditorID —— 设置当前 PIE 实例号。
	CORE_API void  SetPlayInEditorID(int32 InPlayInEditorID);
// 命名空间结束。
}
```

**本段新元素**

- `FIsEditorLoadingPackage` —— 已废弃（5.5）的 bool 包装类
- `GIsEditorLoadingPackage` —— 编辑器是否正在加载包（已废弃）
- `UE::GetIsEditorLoadingPackage()` / `SetIsEditorLoadingPackage()` —— 新 API
- `GIsCookerLoadingPackage` —— cooker 是否正在加载包
- `GIsPlayInEditorWorld` —— GWorld 是否指向 PIE 世界
- `FPlayInEditorID` —— 已废弃（5.5）的 int32 包装类
- `GPlayInEditorID` —— 当前 PIE 实例号（已废弃）
- `UE::GetPlayInEditorID()` / `SetPlayInEditorID()` —— 新 API
- `GIsPIEUsingPlayerStart` —— PIE 是否用 PlayerStart（已废弃）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 496~501 行 | `FIsEditorLoadingPackage` 包装类 |
| 第 506 行 | 废弃的全局标志 |
| 第 511~512 行 | 新 API |
| 第 516~519 行 | cooker / PIE 世界标志 |
| 第 521~537 行 | `FPlayInEditorID` + 新旧 API |
| 第 542 行 | 废弃的 PlayerStart 标志 |

> 类比：像换门锁：旧钥匙（包装类）还能用但已贴封条，新钥匙（UE:: 函数）才是正路。

### 二十、帧号 / 线程 ID / 日志开关 / 统计（第 540~654 行）

```cpp
/** PIE 是否尝试从 PlayerStart 开始播放 */
// 标记 GIsPIEUsingPlayerStart 已废弃（4.25），该变量已不再被设置。
UE_DEPRECATED(4.25, "This variable is no longer set. Use !GEditor->GetPlayInEditorSessionInfo()->OriginalRequestParams.HasPlayWorldPlacement() instead.")
// GIsPIEUsingPlayerStart —— PIE 是否用 PlayerStart（已废弃）。
extern CORE_API bool GIsPIEUsingPlayerStart;

/** 运行时是否要求纹理尺寸为 2 的幂 */
// GPlatformNeedsPowerOfTwoTextures —— 平台是否要求 2 的幂纹理。
extern CORE_API bool GPlatformNeedsPowerOfTwoTextures;

/** FPlatformTime::Seconds() 首次初始化的时间点（非常早） */
// GStartTime —— 引擎启动时间（秒）。
extern CORE_API double GStartTime;

/** 引擎初始化时的系统时间。 */
// GSystemStartTime —— 系统启动时间字符串。
extern CORE_API FString GSystemStartTime;

/** 是否还在初始加载过程中。 */
// GIsInitialLoad —— 是否处于初始加载。
extern CORE_API bool GIsInitialLoad;

/* 是否使用事件驱动加载器 */
// GEventDrivenLoaderEnabled —— 事件驱动加载器是否启用。
extern CORE_API bool GEventDrivenLoaderEnabled;

/** 是否正在从 UClass 取 VTablePtr */
// GIsRetrievingVTablePtr —— 是否正在取 VTablePtr。
extern CORE_API bool GIsRetrievingVTablePtr;

/** 稳定递增的帧计数器。 */
// GFrameCounter —— 帧计数器（64 位）。
extern CORE_API uint64 GFrameCounter;

// GFrameCounterRenderThread —— 渲染线程侧的帧计数器。
extern CORE_API uint64 GFrameCounterRenderThread;

/** 上一次跑 GC 时的 GFrameCounter。 */
// GLastGCFrame —— 上次 GC 的帧号。
extern CORE_API uint64 GLastGCFrame;

/** 输入采样时间（单位：CPU 周期）。 */
// GInputTime —— 输入采样时间。
extern CORE_API uint64 GInputTime;

/** 每帧渲染场景前自增一次；分屏模式下所有视图共用一次自增（不是每视图一次）。 */
// GFrameNumber —— 帧号（32 位）。
extern CORE_API uint32 GFrameNumber;

/** 待重命名；渲染线程版帧号请用 View.ViewFamily->FrameNumber 或从 GFrameTime 传下去。 */
// GFrameNumberRenderThread —— 渲染线程侧帧号。
extern CORE_API uint32 GFrameNumberRenderThread;

/** 是否是游戏运行的第一个实例。 */
// 标记 GIsFirstInstance 已废弃（5.1），改用 FPlatformProcess::IsFirstInstance()。
UE_DEPRECATED(5.1, "Please use `FPlatformProcess::IsFirstInstance()`")
// GIsFirstInstance —— 是否首个实例（已废弃）。
extern CORE_API bool GIsFirstInstance;

/** 一帧被判定为「卡顿」的阈值（毫秒）。 */
// GHitchThresholdMS —— 卡顿阈值毫秒数。
extern CORE_API float GHitchThresholdMS;

/** 保存压缩数据时，把数据切成多大的块 */
// GSavingCompressionChunkSize —— 压缩分块大小。
extern CORE_API int32 GSavingCompressionChunkSize;

/** 主线程/游戏线程的线程 ID */
// GGameThreadId —— 游戏线程 ID。
extern CORE_API uint32 GGameThreadId;

/** 渲染线程的线程 ID（如果有） */
// 标记 GRenderThreadId 已废弃（4.26），改用 IsInActualRenderingThread()。
UE_DEPRECATED(4.26, "Please use `IsInActualRenderingThread()`")
// GRenderThreadId —— 渲染线程 ID（已废弃）。
extern CORE_API uint32 GRenderThreadId;

/** Slate 线程的线程 ID（如果有） */
// GSlateLoadingThreadId —— Slate 加载线程 ID。
extern CORE_API uint32 GSlateLoadingThreadId;

/** GGameThreadId 是否已经设置过 */
// GIsGameThreadIdInitialized —— 游戏线程 ID 是否已初始化。
extern CORE_API bool GIsGameThreadIdInitialized;

/** 决定做哪种 trace，NAME_None 表示不做。 */
// GCurrentTraceName —— 当前 trace 名。
extern CORE_API FLazyName GCurrentTraceName;

/** 日志里时间怎么打印。 */
// GPrintLogTimes —— 日志时间格式（TSAN 原子）。
extern CORE_API TSAN_ATOMIC(ELogTimes::Type) GPrintLogTimes;

/** 日志里分类怎么打印。 */
// GPrintLogCategory —— 是否打印日志分类（TSAN 原子）。
extern CORE_API TSAN_ATOMIC(bool) GPrintLogCategory;

/** 日志里详细级别怎么打印。 */
// GPrintLogVerbosity —— 是否打印详细级别（TSAN 原子）。
extern CORE_API TSAN_ATOMIC(bool) GPrintLogVerbosity;

// 如果启用了卡顿检测（USE_HITCH_DETECTION）。
#if USE_HITCH_DETECTION
/** 轻量统计和 FGameThreadHitchHeartBeat 用它，在 Shipping 包里打印卡顿时的调用栈。 */
// GHitchDetected —— 是否检测到卡顿（TSAN 原子）。
extern CORE_API TSAN_ATOMIC(bool) GHitchDetected;
// 条件编译结束。
#endif

/** 统计系统是否要为 PIX 这类工具发命名事件。 */
// GCycleStatsShouldEmitNamedEvents —— 是否发命名事件。
extern CORE_API int32 GCycleStatsShouldEmitNamedEvents;

/** 详细统计是否也生成外部性能分析器的命名事件。
* 线程睡眠/等待统计和超高频周期统计默认关闭。
* GCycleStatsShouldEmitNamedEvents 为 0 时本开关无效。
*/
// GShouldEmitVerboseNamedEvents —— 详细统计是否发命名事件。
extern CORE_API bool GShouldEmitVerboseNamedEvents;

/** 关闭一些会打断演示的警告和小功能 */
// GIsDemoMode —— 演示模式。
extern CORE_API bool GIsDemoMode;

/** Core 包的名字。 */
//@包名迁移期标记，迁移完成后删掉双重检查
// GLongCorePackageName —— Core 包长名。
extern CORE_API FLazyName GLongCorePackageName;
//@包名迁移期标记，迁移完成后删掉双重检查
// GLongCoreUObjectPackageName —— CoreUObject 包长名。
extern CORE_API FLazyName GLongCoreUObjectPackageName;

/** 当前是否在跑单元测试。 */
// GIsAutomationTesting —— 是否在自动化测试。
extern CORE_API bool GIsAutomationTesting;

/** 是否在主循环之外泵消息 */
// GPumpingMessagesOutsideOfMainLoop —— 主循环外泵消息。
extern CORE_API bool GPumpingMessagesOutsideOfMainLoop;

/** 是否正在泵消息 */
// GPumpingMessages —— 是否正在泵消息。
extern CORE_API bool GPumpingMessages;
 
/** 启用各种编辑器和 HMD 的 hack，让实验性 VR 编辑器功能可用（可能牺牲其他系统） */
// GEnableVREditorHacks —— 是否启用 VR 编辑器 hack。
extern CORE_API bool GEnableVREditorHacks;

// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING

/** 是否忽略已附加的调试器。 */
// GIgnoreDebugger —— 是否忽略调试器。
extern CORE_API bool GIgnoreDebugger;

// 条件编译结束（!UE_BUILD_SHIPPING）。
#endif // #if !UE_BUILD_SHIPPING
```

**本段新元素**

- `GPlatformNeedsPowerOfTwoTextures` —— 平台是否要求 2 的幂纹理
- `GStartTime` —— 引擎启动时间（秒）
- `GSystemStartTime` —— 系统启动时间字符串
- `GIsInitialLoad` —— 是否初始加载中
- `GEventDrivenLoaderEnabled` —— 事件驱动加载器是否启用
- `GIsRetrievingVTablePtr` —— 是否正在取 VTablePtr
- `GFrameCounter` / `GFrameCounterRenderThread` —— 帧计数器（64 位）
- `GLastGCFrame` —— 上次 GC 的帧号
- `GInputTime` —— 输入采样时间（CPU 周期）
- `GFrameNumber` / `GFrameNumberRenderThread` —— 帧号（32 位）
- `GIsFirstInstance` —— 是否首个实例（已废弃）
- `GHitchThresholdMS` —— 卡顿阈值（毫秒）
- `GSavingCompressionChunkSize` —— 压缩分块大小
- `GGameThreadId` / `GRenderThreadId` / `GSlateLoadingThreadId` —— 线程 ID
- `GIsGameThreadIdInitialized` —— 游戏线程 ID 是否已初始化
- `GCurrentTraceName` —— 当前 trace 名
- `GPrintLogTimes` / `GPrintLogCategory` / `GPrintLogVerbosity` —— 日志打印开关
- `GHitchDetected` —— 是否检测到卡顿
- `GCycleStatsShouldEmitNamedEvents` / `GShouldEmitVerboseNamedEvents` —— 统计命名事件
- `GIsDemoMode` —— 演示模式
- `GLongCorePackageName` / `GLongCoreUObjectPackageName` —— 包长名
- `GIsAutomationTesting` —— 是否自动化测试
- `GPumpingMessagesOutsideOfMainLoop` / `GPumpingMessages` —— 消息泵
- `GEnableVREditorHacks` —— VR 编辑器 hack
- `GIgnoreDebugger` —— 是否忽略调试器

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 545~560 行 | 平台/启动/加载标志 |
| 第 563~577 行 | 帧计数器与帧号 |
| 第 581~600 行 | 实例/卡顿/压缩/线程 ID |
| 第 603~612 行 | trace 名 + 日志打印开关 |
| 第 616~626 行 | 卡顿检测 + 统计命名事件 |
| 第 629~647 行 | 演示模式 / 包名 / 测试 / 消息泵 / VR hack |
| 第 652 行 | 忽略调试器（非 Shipping） |

> 类比：像仪表盘第二排：帧率、线程、日志、统计，全是运行时读数。

### 二十一、`ETaskTag`（线程标签位标志）（第 656~676 行）

```cpp
// ETaskTag —— 执行上下文（线程或任务）的标签，用来判断「我现在在哪个线程」。
enum class ETaskTag : int32
{
// ENone —— 无标签。
	ENone						= 0 << 0,
// EStaticInit —— 静态初始化阶段。
	EStaticInit					= 1 << 0,
// EGameThread —— 游戏线程。
	EGameThread					= 1 << 1,
// ESlateThread —— Slate UI 线程。
	ESlateThread				= 1 << 2,
// EAudioThread —— 音频线程（5.0 起已废弃，不再使用）。
	EAudioThread UE_DEPRECATED(5.0, "AudioThread was removed and ETaskTag::EAudioThread is not used anymore. Please remove it.") = 1 << 3,
// ERenderingThread —— 渲染线程。
	ERenderingThread			= 1 << 4,
// ERhiThread —— RHI 线程。
	ERhiThread					= 1 << 5,
// EAsyncLoadingThread —— 异步加载线程。
	EAsyncLoadingThread			= 1 << 6,
// EEventThread —— 事件线程。
	EEventThread				= 1 << 7,

// ENamedThreadBits —— 所有「具名线程」位的掩码。
	ENamedThreadBits			= (EEventThread << 1) - 1,
	EParallelThread				= 1 << 30, // 多个线程或任务参与时用（通常是并行 for）；会跳过具名线程标签的唯一性检查。
// EWorkerThread —— 工作线程（并行 + 独立位）。
	EWorkerThread				= 1 << 29 | EParallelThread,
// EParallelRenderingThread —— 并行渲染线程。
	EParallelRenderingThread	= ERenderingThread | EParallelThread,
// EParallelGameThread —— 并行游戏线程。
	EParallelGameThread			= EGameThread | EParallelThread,
// EParallelRhiThread —— 并行 RHI 线程。
	EParallelRhiThread			= ERhiThread | EParallelThread,
};

// 给 ETaskTag 生成位运算与判位工具。
ENUM_CLASS_FLAGS(ETaskTag);
```

**本段新元素**

- `enum class ETaskTag : int32` —— 执行上下文标签
- `ENone` / `EStaticInit` / `EGameThread` / `ESlateThread` —— 基础标签
- `EAudioThread` —— 已废弃（5.0）
- `ERenderingThread` / `ERhiThread` / `EAsyncLoadingThread` / `EEventThread` —— 其余线程
- `ENamedThreadBits` —— 所有具名线程位的掩码
- `EParallelThread = 1 << 30` —— 并行标记位
- `EWorkerThread` / `EParallelRenderingThread` / `EParallelGameThread` / `EParallelRhiThread` —— 并行组合
- `ENUM_CLASS_FLAGS(ETaskTag)` —— 生成位运算

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 658~666 行 | 8 个基础标签（含 1 个废弃） |
| 第 668 行 | 具名线程掩码 |
| 第 669~673 行 | 并行标记 + 4 个并行组合 |
| 第 676 行 | 生成位运算 |

> 类比：像给每个工人戴不同颜色的帽子：一眼看出他在哪个车间。

### 二十二、`FTaskTagScope`（标签作用域）（第 679~735 行）

```cpp
/**
 * 这个类用来给执行上下文（线程或任务）打标签，之后可在调用栈里查询状态。
 * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。
 *
 * @param InTag 要用的标签
 */
// FTaskTagScope —— 标签作用域，构造时打标签、析构时恢复。
class FTaskTagScope
{
// 友元：FRunnableThread 可访问私有成员。
	friend class FRunnableThread;
// 友元：FRenderingThread 可访问私有成员。
	friend class FRenderingThread;
// 每个线程各自一份「当前激活标签」（thread_local）。
	static thread_local ETaskTag ActiveTaskTag;
// 取静态线程 ID。
	static int32 GetStaticThreadId();
// 父标签（用于析构时恢复）。
	ETaskTag ParentTag;
// 本作用域的标签。
	ETaskTag Tag;
// 是否只在当前无标签时才打标签。
	bool TagOnlyIfNone;

// 公有区。
public:
	/**
	 * 清掉 EStaticInit 标签，让主 OS 线程没有激活标签。
	 * 这样从主线程调用 IsInGameThread() 这类函数才能正常工作。
	 */
// SetTagNone —— 清空当前线程标签。
	static void CORE_API SetTagNone();

	/**
	 * 恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数
	 * 在检查线程状态（如 IsInGameThread()）时能正常工作。
	 * （原文：state (from functions like IsInGameThread()).）
	 */
// SetTagStaticInit —— 恢复静态初始化标签。
	static void CORE_API SetTagStaticInit();

	/**
	* 交换标签，只在「线程上下文在不同线程间迁移」时用。
	*/
// SwapTag —— 交换当前标签并返回旧标签。
	static ETaskTag CORE_API SwapTag(ETaskTag Tag);

// 公有区。
public:
// 构造函数：进入作用域时打上 InTag。
	CORE_API FTaskTagScope(ETaskTag InTag);
// 析构函数：离开作用域时恢复父标签。
	CORE_API ~FTaskTagScope();

// GetCurrentTag —— 取当前线程的标签。
	static CORE_API ETaskTag GetCurrentTag();
// IsCurrentTag —— 当前标签是否等于 InTag。
	static CORE_API bool IsCurrentTag(ETaskTag InTag);
// IsRunningDuringStaticInit —— 是否处于静态初始化阶段。
	static CORE_API bool IsRunningDuringStaticInit();
// 类结束。
};

/**
 * 这个类也能给执行上下文打标签，但只在「还没打过标签」时才打。
 * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。
 *
 * @param InTag 要用的标签
 */
// FOptionalTaskTagScope —— 已废弃（5.6），因为 FTaskTagScope 现在默认就是「可选」行为。
class UE_DEPRECATED(5.6, "Use FTaskTagScope instead which is now always optional by default.") FOptionalTaskTagScope : public FTaskTagScope
{
// 公有区。
public:
// 构造函数：默认标签 ENone，直接转给父类。
	FOptionalTaskTagScope(ETaskTag InTag = ETaskTag::ENone) : FTaskTagScope(InTag)
	{
	}
// 类结束。
};
```

**本段新元素**

- `FTaskTagScope` —— 构造时打标签、析构时恢复
- `friend class FRunnableThread` / `FRenderingThread` —— 友元
- `static thread_local ETaskTag ActiveTaskTag` —— 每线程一份当前标签
- `ParentTag` / `Tag` / `TagOnlyIfNone` —— 父标签 / 本标签 / 是否只在无标签时打
- `SetTagNone()` —— 清空标签
- `SetTagStaticInit()` —— 恢复静态初始化标签
- `SwapTag(Tag)` —— 交换标签
- `GetCurrentTag()` / `IsCurrentTag()` / `IsRunningDuringStaticInit()` —— 查询
- `FOptionalTaskTagScope` —— 已废弃（5.6），因为 `FTaskTagScope` 现在默认就是可选行为

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 687~693 行 | 友元 + 4 个成员 |
| 第 700~712 行 | 3 个静态方法 |
| 第 715~720 行 | 构造/析构 + 3 个查询 |
| 第 729~735 行 | 废弃的 `FOptionalTaskTagScope` |

> 类比：像进车间换工牌：进门挂上，出门摘掉，中途还能查「我现在挂的是哪块」。

### 二十三、线程判断函数（第 737~791 行）

```cpp
/**
 * 确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。
 *
 * @param CtorSignature 当前正在跑的构造函数签名，
 *		用来拼出正确的错误信息。
 * （原文：construct proper error message.）
 */
// EnsureRetrievingVTablePtrDuringCtor —— 校验取 vtable 的时机。
CORE_API void EnsureRetrievingVTablePtrDuringCtor(const TCHAR* CtorSignature);

/** @return 是否从游戏线程调用。 */
// IsInGameThread —— 是否在游戏线程。
extern CORE_API bool IsInGameThread();

/** @return 是否从「并行 for 里的游戏线程」调用。 */
// IsInParallelGameThread —— 是否在并行游戏线程。
extern CORE_API bool IsInParallelGameThread();

// IsAudioThreadRunning —— 音频线程是否在运行。
extern CORE_API bool IsAudioThreadRunning();

/** @return 是否从音频线程调用（不是「调用了音频函数的线程」）。 */
// IsInAudioThread —— 是否在音频线程。
extern CORE_API bool IsInAudioThread();

/** @return 是否从 Slate 线程调用（不是「调用了 Slate 函数的线程」）。 */
// IsInSlateThread —— 是否在 Slate 线程。
extern CORE_API bool IsInSlateThread();

/** @return 是否从渲染线程调用；单线程渲染时任何线程都算。 */
// IsInRenderingThread —— 是否在渲染线程。
extern CORE_API bool IsInRenderingThread();

/** @return 是否从渲染线程调用，或从任何非游戏线程调用；单线程渲染时游戏线程也算。 */
// IsInParallelRenderingThread —— 是否在并行渲染线程。
extern CORE_API bool IsInParallelRenderingThread();

/** @return 是否从任一渲染线程调用；任何线程都可安全调用，不会断言。 */
// IsInAnyRenderingThread —— 是否在任一渲染线程。
extern CORE_API bool IsInAnyRenderingThread();

/** @return 是否从渲染线程调用。 */
// 与 IsInRenderingThread 不同，单线程渲染时它永远返回 false；只有真正独立的渲染线程才返回 true。主要用于检查
// IsInActualRenderingThread —— 是否在真正的渲染线程。
extern CORE_API bool IsInActualRenderingThread();

/** @return 异步加载线程启用时看是否在该线程；否则看是否在游戏线程跑异步加载代码。 */
// IsInAsyncLoadingThread —— 函数指针，判断是否在异步加载线程。
extern CORE_API bool (*IsInAsyncLoadingThread)();

/** 渲染用的线程对象 */
UE_DEPRECATED(4.26, "Please use `GIsThreadedRendering` or `IsInActualRenderingThread()`")
// GRenderingThread —— 渲染线程对象（已废弃）。
extern CORE_API FRunnableThread* GRenderingThread;

/** @return RHI 线程是否在运行 */
// IsRHIThreadRunning —— RHI 线程是否在运行。
extern CORE_API bool IsRHIThreadRunning();

/** @return 是否从 RHI 线程调用；单线程渲染时任何线程都算。 */
// IsInRHIThread —— 是否在 RHI 线程。
extern CORE_API bool IsInRHIThread();

/** @return 是否从「标记为 RHI 任务」的线程调用。 */
// IsInParallelRHIThread —— 是否在并行 RHI 线程。
extern CORE_API bool IsInParallelRHIThread();

// GIsRunningRHIInSeparateThread_InternalUseOnly —— RHI 是否跑在独立线程（内部用）。
extern CORE_API bool GIsRunningRHIInSeparateThread_InternalUseOnly;
// GIsRunningRHIInDedicatedThread_InternalUseOnly —— RHI 是否跑在专用线程（内部用）。
extern CORE_API bool GIsRunningRHIInDedicatedThread_InternalUseOnly;
// GIsRunningRHIInTaskThread_InternalUseOnly —— RHI 是否跑在任务线程（内部用）。
extern CORE_API bool GIsRunningRHIInTaskThread_InternalUseOnly;
```

**本段新元素**

- `EnsureRetrievingVTablePtrDuringCtor(CtorSignature)` —— 校验取 vtable 的时机
- `IsInGameThread()` —— 是否在游戏线程
- `IsInParallelGameThread()` —— 是否在并行游戏线程
- `IsAudioThreadRunning()` / `IsInAudioThread()` —— 音频线程
- `IsInSlateThread()` —— Slate 线程
- `IsInRenderingThread()` / `IsInParallelRenderingThread()` / `IsInAnyRenderingThread()` / `IsInActualRenderingThread()` —— 4 个渲染线程判断
- `IsInAsyncLoadingThread` —— 函数指针
- `GRenderingThread` —— 渲染线程对象（已废弃）
- `IsRHIThreadRunning()` / `IsInRHIThread()` / `IsInParallelRHIThread()` —— RHI 线程
- `GIsRunningRHIInSeparateThread_InternalUseOnly` 等 3 个 —— RHI 线程模式标志

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 744 行 | vtable 校验 |
| 第 747~758 行 | 游戏/音频/Slate 线程判断 |
| 第 761~774 行 | 4 个渲染线程判断 + 异步加载 |
| 第 778 行 | 废弃的渲染线程对象 |
| 第 781~791 行 | RHI 线程判断 + 3 个模式标志 |

> 类比：像问「我现在在哪个车间」：游戏、渲染、RHI、音频、Slate，各有各的门禁。

### 二十四、`FScopedLoadingState`（加载状态作用域）（第 793~819 行）

```cpp
/** 启动加载计时器 */
// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING
// NotifyLoadingStateChanged —— 通知加载状态变化（真实实现）。
CORE_API void NotifyLoadingStateChanged(bool bState, const TCHAR *Message);
// FScopedLoadingState —— 加载状态作用域，构造时通知「开始加载」，析构时通知「结束加载」。
struct FScopedLoadingState
{
// 加载阶段名。
	FString Message;
// 构造函数：记下阶段名并通知开始。
	FScopedLoadingState(const TCHAR* InMessage)
// 初始化列表：保存阶段名。
		: Message(InMessage)
	{
// 通知「进入加载状态」。
		NotifyLoadingStateChanged(true, *Message);
	}
// 析构函数：通知结束。
	~FScopedLoadingState()
	{
// 通知「退出加载状态」。
		NotifyLoadingStateChanged(false, *Message);
	}
};
// 否则（Shipping 构建）。
#else
// NotifyLoadingStateChanged —— Shipping 下空实现。
FORCEINLINE void NotifyLoadingStateChanged(bool bState, const TCHAR *Message)
{
}
// FScopedLoadingState —— Shipping 下的空壳版本。
struct FScopedLoadingState
{
// 构造函数：什么都不做。
	FORCEINLINE FScopedLoadingState(const TCHAR* InMessage)
	{
	}
};
// 条件编译结束。
#endif
```

**本段新元素**

- `NotifyLoadingStateChanged(bState, Message)` —— 通知加载状态变化
- `FScopedLoadingState` —— 构造时通知「开始加载」，析构时通知「结束加载」
- `#if !UE_BUILD_SHIPPING` —— 非 Shipping 才有真实实现
- Shipping 下 —— 空实现 + 空壳结构体（零开销）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 795~808 行 | 非 Shipping：真实实现 |
| 第 810~818 行 | Shipping：空实现 |

> 类比：像打卡机：正式员工打卡记录，临时工直接放行不记。

### 二十五、绘制事件 + 调试可视化 + Zen（第 821~847 行）

```cpp
// GetEmitDrawEvents —— 是否发出绘制事件（调试用）。
bool CORE_API GetEmitDrawEvents();
// SetEmitDrawEvents —— 设置是否发出绘制事件。
void CORE_API SetEmitDrawEvents(bool EmitDrawEvents);

/** 帮助在调试器里可视化弱指针的数组 */
// 前置声明 FChunkedFixedUObjectArray。
class FChunkedFixedUObjectArray;

// GCoreObjectArrayForDebugVisualizers —— 调试可视化用的对象数组指针。
extern CORE_API FChunkedFixedUObjectArray* GCoreObjectArrayForDebugVisualizers;

// 命名空间 UE::CoreUObject::Private：CoreUObject 内部实现细节。
namespace UE::CoreUObject::Private
{
	/** 帮助在调试器里可视化对象路径的数组 */
// 前置声明 FStoredObjectPathDebug。
	struct FStoredObjectPathDebug;
	
	/** 帮助在调试器里可视化对象句柄的数组 */
// 前置声明 FObjectHandlePackageDebugData。
	struct FObjectHandlePackageDebugData;
// 命名空间结束。
}
// 前置声明 UE::Core::FVisualizerDebuggingState。
namespace UE::Core { struct FVisualizerDebuggingState; }
// GCoreComplexObjectPathDebug —— 对象路径调试可视化数据。
extern CORE_API UE::CoreUObject::Private::FStoredObjectPathDebug* GCoreComplexObjectPathDebug;
// GCoreObjectHandlePackageDebugData —— 对象句柄调试可视化数据。
extern CORE_API UE::CoreUObject::Private::FObjectHandlePackageDebugData* GCoreObjectHandlePackageDebug;
// GCoreDebuggingState —— 调试可视化状态。
extern CORE_API UE::Core::FVisualizerDebuggingState* GCoreDebuggingState;

/** @return 是否在跑 cook-on-the-fly（边跑边 cook）。 */
// IsRunningCookOnTheFly —— 是否在跑 cook-on-the-fly。
bool CORE_API IsRunningCookOnTheFly();

// 命名空间 UE。
namespace UE {
// IsUsingZenPakFileStreaming —— 是否使用 Zen 的 pak 文件流式加载。
	bool CORE_API IsUsingZenPakFileStreaming();
// 命名空间结束。
}
```

**本段新元素**

- `GetEmitDrawEvents()` / `SetEmitDrawEvents()` —— 是否发出绘制事件
- `GCoreObjectArrayForDebugVisualizers` —— 调试可视化用的对象数组
- `UE::CoreUObject::Private::FStoredObjectPathDebug` —— 对象路径调试数据
- `UE::CoreUObject::Private::FObjectHandlePackageDebugData` —— 对象句柄调试数据
- `UE::Core::FVisualizerDebuggingState` —— 调试可视化状态
- `GCoreComplexObjectPathDebug` / `GCoreObjectHandlePackageDebug` / `GCoreDebuggingState` —— 3 个调试指针
- `IsRunningCookOnTheFly()` —— 是否在跑 cook-on-the-fly
- `UE::IsUsingZenPakFileStreaming()` —— 是否用 Zen 的 pak 流式加载

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 821~822 行 | 绘制事件开关 |
| 第 825~840 行 | 调试可视化（3 个前置声明 + 3 个指针） |
| 第 843 行 | cook-on-the-fly |
| 第 846 行 | Zen pak 流式加载 |

> 类比：像调试器里的放大镜：专门给 Visual Studio 看对象用的辅助数据。

### 二十六、`IsThisNotNull`（已废弃）（第 849~854 行）

```cpp
/* 用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。
 * 目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。
 * （原文：on undefined behavior.）
 */
// 标记 IsThisNotNull 已废弃（5.6）。
UE_DEPRECATED(5.6, "IsThisNotNull has been deprecated.")
// IsThisNotNull —— 检查 this 是否非空（已废弃）。
CORE_API FORCENOINLINE bool IsThisNotNull(const void* This, const ANSICHAR* Function);
```

**本段新元素**

- `IsThisNotNull(This, Function)` —— 检查 `this` 是否非空
- `FORCENOINLINE` —— 强制不内联（保证能断点）
- `UE_DEPRECATED(5.6, ...)` —— 已废弃

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 849~852 行 | 说明块注释 |
| 第 853 行 | 废弃标记 |
| 第 854 行 | 函数声明 |

> 类比：像老式保险丝：还在，但已经贴了「请换新方案」的标签。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + include + 前置声明 | 1~27 | 版权头 + include + 前置声明 |
| 日志分类声明（24 个） | 29~65 | 日志分类声明（24 个） |
| 日志单例 + 启动耗时点 | 68~72 | 日志单例 + 启动耗时点 |
| `FScopedBootTiming`（启动耗时作用域） | 74~81 | `FScopedBootTiming`（启动耗时作用域） |
| `FEngineTrackedActivityScope`（Insights 活动作用域） | 83~88 | `FEngineTrackedActivityScope`（Insights 活动作用域） |
| 两个作用域宏 | 91~92 | 两个作用域宏 |
| 核心全局量 | 95~106 | 核心全局量 |
| `FCoreTexts`（本地化文本） | 108~129 | `FCoreTexts`（本地化文本） |
| 旧版全局文本（已废弃） | 131~142 | 旧版全局文本（已废弃） |
| 蓝图/对象/嵌入相关开关 | 144~194 | 蓝图/对象/嵌入相关开关 |
| commandlet 私有标志 | 196~205 | commandlet 私有标志 |
| `GIsEditor` 三态定义 | 207~237 | `GIsEditor` 三态定义 |
| commandlet 判断函数（6 个） | 239~325 | commandlet 判断函数（6 个） |
| 多进程 ID | 327~342 | 多进程 ID |
| 重新实例化包装类 + 客户端/服务器标志 | 344~356 | 重新实例化包装类 + 客户端/服务器标志 |
| 启动/退出/截图相关 | 358~427 | 启动/退出/截图相关 |
| 各种 ini 路径 | 429~457 | 各种 ini 路径 |
| 异步加载 + 加载器类型 | 459~494 | 异步加载 + 加载器类型 |
| 编辑器加载包 / PIE 实例号 | 496~538 | 编辑器加载包 / PIE 实例号 |
| 帧号 / 线程 ID / 日志开关 / 统计 | 540~654 | 帧号 / 线程 ID / 日志开关 / 统计 |
| `ETaskTag`（线程标签位标志） | 656~676 | `ETaskTag`（线程标签位标志） |
| `FTaskTagScope`（标签作用域） | 679~735 | `FTaskTagScope`（标签作用域） |
| 线程判断函数 | 737~791 | 线程判断函数 |
| `FScopedLoadingState`（加载状态作用域） | 793~819 | `FScopedLoadingState`（加载状态作用域） |
| 绘制事件 + 调试可视化 + Zen | 821~847 | 绘制事件 + 调试可视化 + Zen |
| `IsThisNotNull`（已废弃） | 849~854 | `IsThisNotNull`（已废弃） |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `extern CORE_API T X;` | 声明一个在别的编译单元定义的全局量 | `extern CORE_API bool GIsServer;` |
| `DECLARE_LOG_CATEGORY_EXTERN(A,B,C)` | 声明日志分类 | `DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log, All);` |
| `enum class X : int32` | 强类型枚举 | `enum class ETaskTag : int32` |
| `1 << n` | 第 n 位置 1 | `EGameThread = 1 << 1` |
| `ENUM_CLASS_FLAGS(X)` | 生成位运算与判位工具 | `ENUM_CLASS_FLAGS(ETaskTag);` |
| `static thread_local T X;` | 每线程一份的静态变量 | `static thread_local ETaskTag ActiveTaskTag;` |
| `FORCEINLINE` | 强制内联 | `FORCEINLINE bool IsRunningCommandlet()` |
| `FORCENOINLINE` | 强制不内联 | `CORE_API FORCENOINLINE bool IsThisNotNull(...)` |
| `X() = delete` | 禁止调用 | `FCoreTexts(const FCoreTexts&) = delete;` |
| `UE_DEPRECATED(v, "msg")` | 标记废弃 | `UE_DEPRECATED(5.5, "This will be removed.")` |
| `operator bool() const` | 隐式类型转换 | `CORE_API operator bool() const;` |
| `## __VA_ARGS__` | 可变参数透传（GCC 扩展） | `Fmt, ## __VA_ARGS__` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 854 行 | 855 行 | 只多 1 行（文件末尾换行） |
| 非注释代码行 | 473 行 | 473 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| 行尾注释 | 英文 | 中文 | 1 处（`EParallelThread`） |
| 块注释行数 | 25 个块 | 25 个块，行数一一对应 | 逐行原位翻译，不增不减 |
| `#endif // 宏名` | 保留宏名 | 保留宏名 | 宏名是标识符，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（855 行）
2. 脚本验收：`codeOurs=473 / codeEngine=473 / diff=0`（剔除行尾注释后逐字一致）
3. 确认后 `Public\` 7 个散头全部完成，下一节进 `Public\` 子目录

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（854 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h`（855 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\07_CoreGlobals.h_总md.md`