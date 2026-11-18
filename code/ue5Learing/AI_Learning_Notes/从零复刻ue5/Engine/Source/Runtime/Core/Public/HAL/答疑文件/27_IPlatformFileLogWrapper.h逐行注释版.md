# 27 — `HAL/IPlatformFileLogWrapper.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`（**513 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `IFileHandle` / `FScopeLock`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

游戏加载慢，你想知道「到底哪个文件读得最久」。`IPlatformFileLogWrapper.h` 就是**文件系统的「行车记录仪」**：它包住底层 `IPlatformFile`，把**每一次**文件操作都记进日志，还带上**耗时**。

**它包含 4 个类 + 1 个宏**：

| 类型 | 作用 |
|---|---|
| `FILE_LOG(...)` | **带防递归保护**的日志宏 |
| `FLoggedFileHandle` | 带日志的文件句柄（包住 `IFileHandle`） |
| `FLoggedPlatformFile` | 带日志的平台文件包装器（包住 `IPlatformFile`） |
| `FLogVisitor` | 带日志的目录访问者（包住 `FDirectoryVisitor`） |
| `FLogStatVisitor` | 带日志的 stat 访问者（包住 `FDirectoryStatVisitor`） |

**核心设计 ①：防递归的 `FILE_LOG` 宏**（第 32~38 行）

```cpp
#define FILE_LOG(CategoryName, Verbosity, Format, ...) \
    if (!bSuppressFileLog) \
    { \
        bSuppressFileLog = true; \      // ← 先置标志
        UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__); \
        bSuppressFileLog = false; \     // ← 再恢复
    }
```

**为什么需要**：`UE_LOG` 内部可能写日志文件 → 触发 `FLoggedPlatformFile::OpenWrite` → 又打日志 → **无限递归**。`bSuppressFileLog` 就是那个「刹车」。

**核心设计 ②：每个方法都是「三段式」**

```cpp
FILE_LOG(...);                              // ① 记「开始」
double StartTime = FPlatformTime::Seconds(); // ② 记开始时间
bool Result = LowerLevel->Xxx(...);          // ③ 调底层
double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;  // ④ 算耗时
FILE_LOG(... Result, ThisTime);              // ⑤ 记「结束 + 耗时」
return Result;
```

**核心设计 ③：`SCOPED_NAMED_EVENT_FSTRING`**（第 191 行等）

```cpp
FString DataStr = FString::Printf(TEXT("FileExists %s"), Filename);
SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
```

这会在 Unreal Insights / PIX 里打一个**命名事件**，让你在时间轴上直接看到「这次 FileExists 花了多久」。

**核心设计 ④：打开句柄计数**（第 476~493 行，仅非 Shipping）

```cpp
TMap<FString, int32> OpenHandles;   // 每个文件打开了几次
FCriticalSection LogFileCritical;   // 保护它的锁
```

`OnHandleOpen` / `OnHandleClosed` 维护计数，`HandleDumpCommand` 是控制台命令，能打印「当前还有哪些文件没关」—— **查文件句柄泄漏的利器**。

**说个场景**：

```
控制台敲：LogFile.Dump
输出：
  OpenHandles:
    ../../../MyProject/Content/Map.umap : 2
    ../../../MyProject/Config/DefaultEngine.ini : 1
```

> 🔑 **一句话**：`IPlatformFileLogWrapper.h` = **文件系统的「行车记录仪」**，每次操作都记日志 + 耗时 + 命名事件，还能查句柄泄漏。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 14 个 include + 4 个前置声明（第 1~23 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入 TMap：OpenHandles 用（记录每个文件打开了几次）。
#include "Containers/Map.h"
// 引入 FString：文件名类型。
#include "Containers/UnrealString.h"
// 引入基础类型层：uint8 / int32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入通用平台文件接口：IPlatformFile / IFileHandle / FFileStatData。
#include "GenericPlatform/GenericPlatformFile.h"
// 引入临界区：FCriticalSection（保护 OpenHandles）。
#include "HAL/CriticalSection.h"
// 引入平台杂项：FPlatformProperties。
#include "HAL/PlatformMisc.h"
// 引入平台时间：FPlatformTime::Seconds（计时用）。
#include "HAL/PlatformTime.h"
// 引入日志分类：DECLARE_LOG_CATEGORY_EXTERN。
#include "Logging/LogCategory.h"
// 引入日志宏：UE_LOG。
#include "Logging/LogMacros.h"
// 引入颜色：FColor::Emerald（命名事件用）。
#include "Math/Color.h"
// 引入 FDateTime：文件时间戳类型。
#include "Misc/DateTime.h"
// 引入作用域锁：FScopeLock。
#include "Misc/ScopeLock.h"
// 引入 TUniquePtr：独占智能指针。
#include "Templates/UniquePtr.h"
// 引入追踪通道：SCOPED_NAMED_EVENT_FSTRING。
#include "Trace/Detail/Channel.h"

// 前置声明 FLoggedPlatformFile。
class FLoggedPlatformFile;
// 前置声明 FOutputDevice。
class FOutputDevice;
// 前置声明 IAsyncReadFileHandle。
class IAsyncReadFileHandle;
// 前置声明 IMappedFileHandle。
class IMappedFileHandle;
```

**本段新元素**

- `Containers/Map.h` —— `TMap<FString, int32>`（OpenHandles）
- `Containers/UnrealString.h` —— `FString`
- `CoreTypes.h` —— `uint8` / `int32` / `int64` / `TCHAR`
- `GenericPlatform/GenericPlatformFile.h` —— `IPlatformFile` / `IFileHandle` / `FFileStatData`
- `HAL/CriticalSection.h` —— `FCriticalSection`
- `HAL/PlatformMisc.h` —— `FPlatformProperties`
- `HAL/PlatformTime.h` —— `FPlatformTime::Seconds`
- `Logging/LogCategory.h` —— `DECLARE_LOG_CATEGORY_EXTERN`
- `Logging/LogMacros.h` —— `UE_LOG`
- `Math/Color.h` —— `FColor::Emerald`
- `Misc/DateTime.h` —— `FDateTime`
- `Misc/ScopeLock.h` —— `FScopeLock`
- `Templates/UniquePtr.h` —— `TUniquePtr<IFileHandle>`
- `Trace/Detail/Channel.h` —— `SCOPED_NAMED_EVENT_FSTRING`
- 4 个前置声明 —— `FLoggedPlatformFile` / `FOutputDevice` / `IAsyncReadFileHandle` / `IMappedFileHandle`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~18 行 | 14 个 include |
| 第 20~23 行 | 4 个前置声明 |

> 类比：像开工前领 14 样工具：容器、字符串、类型、平台文件、锁、平台信息、时间、日志、颜色、时间戳、作用域锁、智能指针、追踪通道。

### 二、日志分类 + `FILE_LOG` 防递归宏（第 25~40 行）

```cpp
// 包装底层文件系统，把每次调用都记进日志。
 

// LogPlatformFile：文件系统日志分类。
DECLARE_LOG_CATEGORY_EXTERN(LogPlatformFile, Log, All);

// bSuppressFileLog：是否抑制文件日志（防止日志本身触发文件操作导致递归）。
extern bool bSuppressFileLog;

// FILE_LOG：带防递归保护的文件日志宏。
#define FILE_LOG(CategoryName, Verbosity, Format, ...) \
	if (!bSuppressFileLog) \
	{ \
		bSuppressFileLog = true; \
		UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__); \
		bSuppressFileLog = false; \
	}

// 前置声明 FLoggedPlatformFile（第二次，供 FLoggedFileHandle 引用）。
class FLoggedPlatformFile;
```

**本段新元素**

- `DECLARE_LOG_CATEGORY_EXTERN(LogPlatformFile, Log, All)` —— 文件系统日志分类
- `extern bool bSuppressFileLog` —— 是否抑制文件日志（防递归）
- `FILE_LOG(CategoryName, Verbosity, Format, ...)` —— 带防递归保护的日志宏
- `if (!bSuppressFileLog)` —— 只在没抑制时才打
- `bSuppressFileLog = true` —— 先置标志（防止 UE_LOG 内部的文件操作再次触发）
- `##__VA_ARGS__` —— 可变参数透传（GCC 扩展，空参数时去掉前面的逗号）
- `bSuppressFileLog = false` —— 恢复标志

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 25~28 行 | 日志分类声明 |
| 第 30 行 | `bSuppressFileLog` 全局标志 |
| 第 32~38 行 | `FILE_LOG` 宏（5 行） |
| 第 40 行 | 前置声明 `FLoggedPlatformFile` |

> 类比：像录音时的「防啸叫」：喇叭一响就自动静音，避免自己录自己。

### 三、`FLoggedFileHandle` 声明 + 成员（第 42~52 行）

```cpp
// FLoggedFileHandle：带日志的文件句柄，包住底层句柄。
class FLoggedFileHandle : public IFileHandle
{
// FileHandle：底层文件句柄（独占指针）。
	TUniquePtr<IFileHandle>	FileHandle;
// Filename：文件名（日志用）。
	FString					Filename;
// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING
// PlatformFile：所属的平台文件包装器（用于统计打开句柄数）。
	FLoggedPlatformFile& PlatformFile;
// 条件编译结束。
#endif
// 公有区。
public:

// 构造函数：保存底层句柄、文件名、所属包装器（实现在 .cpp）。
	CORE_API FLoggedFileHandle(IFileHandle* InFileHandle, const TCHAR* InFilename, FLoggedPlatformFile& InOwner);
// 析构函数：关闭底层句柄并通知包装器（实现在 .cpp）。
	CORE_API virtual ~FLoggedFileHandle();
```

**本段新元素**

- `class FLoggedFileHandle : public IFileHandle` —— 带日志的文件句柄
- `TUniquePtr<IFileHandle> FileHandle` —— 底层句柄（独占指针）
- `FString Filename` —— 文件名（日志用）
- `FLoggedPlatformFile& PlatformFile` —— 所属包装器（仅非 Shipping，用于统计句柄数）
- `CORE_API FLoggedFileHandle(...)` —— 构造函数（实现在 .cpp）
- `CORE_API virtual ~FLoggedFileHandle()` —— 析构函数（实现在 .cpp）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 42~48 行 | 类声明 + 3 个成员 |
| 第 49 行 | 公有区 |
| 第 51 行 | 构造函数声明 |
| 第 52 行 | 析构函数声明 |

> 类比：像给水管装水表：进水管（FileHandle）、门牌号（Filename）、抄表员（PlatformFile）。

### 四、`FLoggedFileHandle` 的 10 个方法（三段式）（第 54~142 行）

```cpp
// Tell：返回当前读写位置。
	virtual int64		Tell() override
// 函数体开始。
	{
// 打日志：Tell 文件名。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Tell %s"), *Filename);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Tell。
		int64 Result = FileHandle->Tell();
// 计算耗时（秒转毫秒）。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：Tell 返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Tell return %lld [%fms]"), Result, ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// Seek：跳转到指定位置。
	virtual bool		Seek(int64 NewPosition) override
// 函数体开始。
	{
// 打日志：Seek 文件名 + 目标位置。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Seek %s %lld"), *Filename, NewPosition);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Seek。
		bool Result = FileHandle->Seek(NewPosition);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：Seek 返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Seek return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// SeekFromEnd：从文件末尾往前跳。
	virtual bool		SeekFromEnd(int64 NewPositionRelativeToEnd) override
// 函数体开始。
	{
// 打日志：SeekFromEnd 文件名 + 相对位置。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("SeekFromEnd %s %lld"), *Filename, NewPositionRelativeToEnd);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 SeekFromEnd。
		bool Result = FileHandle->SeekFromEnd(NewPositionRelativeToEnd);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("SeekFromEnd return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// Read：读数据。
	virtual bool		Read(uint8* Destination, int64 BytesToRead) override
// 函数体开始。
	{
// 打日志：Read 文件名 + 字节数。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Read %s %lld"), *Filename, BytesToRead);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Read。
		bool Result = FileHandle->Read(Destination, BytesToRead);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Read return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// ReadAt：从指定偏移读。
	virtual bool		ReadAt(uint8* Destination, int64 BytesToRead, int64 Offset) override
// 函数体开始。
	{
// 打日志：ReadAt 文件名 + 字节数 + 偏移。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("ReadAt %s %lld %lld"), *Filename, BytesToRead, Offset);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 ReadAt。
		bool Result = FileHandle->ReadAt(Destination, BytesToRead, Offset);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("ReadAt return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// Write：写数据。
	virtual bool		Write(const uint8* Source, int64 BytesToWrite) override
// 函数体开始。
	{
// 打日志：Write 文件名 + 字节数。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Write %s %lld"), *Filename, BytesToWrite);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Write。
		bool Result = FileHandle->Write(Source, BytesToWrite);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, VeryVerbose, TEXT("Write return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// Size：返回文件大小。
	virtual int64		Size() override
// 函数体开始。
	{
// 打日志：Size 文件名（注意这里是 Verbose 级，不是 VeryVerbose）。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Size %s"), *Filename);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Size。
		int64 Result = FileHandle->Size();
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Size return %lld [%fms]"), Result, ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// Flush：刷新到磁盘。
	virtual bool		Flush(const bool bFullFlush = false) override
// 函数体开始。
	{
// 打日志：Flush 文件名 + full/partial。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Flush %s %s"), *Filename, (bFullFlush ? TEXT("full") : TEXT("partial")));
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Flush。
		bool bResult = FileHandle->Flush(bFullFlush);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Flush return %d [%fms]"), int32(bResult), ThisTime);
// 返回结果。
		return bResult;
// 函数体结束。
	}
// Truncate：截断文件。
	virtual bool		Truncate(int64 NewSize) override
// 函数体开始。
	{
// 打日志：Truncate 文件名 + 新大小。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Truncate %s %lld"), *Filename, NewSize);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 Truncate。
		bool bResult = FileHandle->Truncate(NewSize);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("Truncate return %d [%fms]"), int32(bResult), ThisTime);
// 返回结果。
		return bResult;
// 函数体结束。
	}
// ShrinkBuffers：收缩内部缓冲。
	virtual void ShrinkBuffers() override
// 函数体开始。
	{
// 打日志：ShrinkBuffers 文件名。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("ShrinkBuffers %s"), *Filename);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 ShrinkBuffers。
		FileHandle->ShrinkBuffers();
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：耗时（无返回值）。
		FILE_LOG(LogPlatformFile, Verbose, TEXT("ShrinkBuffers return [%fms]"), ThisTime);
// 函数体结束。
	}
```

**本段新元素**

- `Tell()` —— 返回当前读写位置
- `Seek(NewPosition)` —— 跳转到指定位置
- `SeekFromEnd(NewPositionRelativeToEnd)` —— 从末尾往前跳
- `Read(Destination, BytesToRead)` —— 读数据
- `ReadAt(Destination, BytesToRead, Offset)` —— 从指定偏移读
- `Write(Source, BytesToWrite)` —— 写数据
- `Size()` —— 返回文件大小（注意日志级别是 `Verbose` 不是 `VeryVerbose`）
- `Flush(bFullFlush)` —— 刷新到磁盘（日志带 `full` / `partial`）
- `Truncate(NewSize)` —— 截断文件
- `ShrinkBuffers()` —— 收缩内部缓冲（无返回值）
- **三段式**：`FILE_LOG` 开始 → `FPlatformTime::Seconds()` → 调底层 → 算耗时 → `FILE_LOG` 结束

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 54~62 行 | `Tell` |
| 第 63~71 行 | `Seek` |
| 第 72~80 行 | `SeekFromEnd` |
| 第 81~89 行 | `Read` |
| 第 90~98 行 | `ReadAt` |
| 第 99~107 行 | `Write` |
| 第 108~116 行 | `Size` |
| 第 117~125 行 | `Flush` |
| 第 126~134 行 | `Truncate` |
| 第 135~142 行 | `ShrinkBuffers` |

> 类比：像给每个动作都装秒表：开始按一下、结束按一下，中间干活。

### 五、`FLoggedPlatformFile` 声明 + 成员 + 基础方法（第 145~186 行）

```cpp
// FLoggedPlatformFile：带日志的平台文件包装器。
class FLoggedPlatformFile : public IPlatformFile
{
// LowerLevel：被包装的下一层平台文件。
	IPlatformFile* LowerLevel;

// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING
// LogFileCritical：保护 OpenHandles 的临界区。
	FCriticalSection LogFileCritical;
// OpenHandles：记录每个文件当前打开了几次。
	TMap<FString, int32> OpenHandles;
// 条件编译结束。
#endif

// 公有区。
public:
// GetTypeName：返回类型名。
	static const TCHAR* GetTypeName()
// 函数体开始。
	{
// 返回 LogFile。
		return TEXT("LogFile");
// 函数体结束。
	}

// 默认构造函数：LowerLevel 置空。
	FLoggedPlatformFile()
// 初始化列表：LowerLevel 为空。
		: LowerLevel(nullptr)
// 函数体开始。
	{
// 函数体结束。
	}

// 说明：为了让基类的重载可见（否则会被隐藏）。
	//~ 为了让基类的重载可见（否则会被隐藏）
// 引入基类的 IterateDirectory。
	using IPlatformFile::IterateDirectory;
// 引入基类的 IterateDirectoryRecursively。
	using IPlatformFile::IterateDirectoryRecursively;
// 引入基类的 IterateDirectoryStat。
	using IPlatformFile::IterateDirectoryStat;
// 引入基类的 IterateDirectoryStatRecursively。
	using IPlatformFile::IterateDirectoryStatRecursively;

// ShouldBeUsed：判断是否启用本包装器（实现在 .cpp）。
	CORE_API virtual bool ShouldBeUsed(IPlatformFile* Inner, const TCHAR* CmdLine) const override;

// Initialize：初始化包装器（实现在 .cpp）。
	CORE_API virtual bool Initialize(IPlatformFile* Inner, const TCHAR* CommandLineParam) override;

// GetLowerLevel：取下一层平台文件。
	IPlatformFile* GetLowerLevel() override
// 函数体开始。
	{
// 返回 LowerLevel。
		return LowerLevel;
// 函数体结束。
	}
// SetLowerLevel：设置下一层平台文件。
	virtual void SetLowerLevel(IPlatformFile* NewLowerLevel) override
// 函数体开始。
	{
// 保存。
		LowerLevel = NewLowerLevel;
// 函数体结束。
	}
// GetName：返回本包装器的名字。
	virtual const TCHAR* GetName() const override
// 函数体开始。
	{
// 返回类型名。
		return FLoggedPlatformFile::GetTypeName();
// 函数体结束。
	}
```

**本段新元素**

- `class FLoggedPlatformFile : public IPlatformFile` —— 带日志的平台文件包装器
- `IPlatformFile* LowerLevel` —— 被包装的下一层
- `FCriticalSection LogFileCritical` —— 保护 `OpenHandles` 的锁（仅非 Shipping）
- `TMap<FString, int32> OpenHandles` —— 每个文件打开了几次（仅非 Shipping）
- `static GetTypeName()` —— 返回 `"LogFile"`
- `FCriticalSection LogFileCritical` —— 保护 `OpenHandles` 的锁（仅非 Shipping）
- `using IPlatformFile::IterateDirectory;` 等 4 个 —— 让基类重载可见
- `ShouldBeUsed(...)` / `Initialize(...)` —— 实现在 .cpp
- `GetLowerLevel()` / `SetLowerLevel(...)` / `GetName()` —— 3 个访问器

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 145~152 行 | 类声明 + 3 个成员 |
| 第 154~158 行 | 公有区 + `GetTypeName` |
| 第 160~163 行 | 默认构造 |
| 第 165~169 行 | 4 个 `using` |
| 第 171~173 行 | `ShouldBeUsed` / `Initialize` |
| 第 175~186 行 | 3 个访问器 |

> 类比：像给整栋楼装监控：每户（文件）进出都记录，还有一本总账（OpenHandles）。

### 六、15 个文件操作（每个都是三段式 + 命名事件）（第 188~363 行）

```cpp
// FileExists：查询文件是否存在。
	virtual bool		FileExists(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("FileExists %s"), Filename);
// 打一个命名事件（性能分析器里可见）。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->FileExists(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("FileExists return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// FileSize：取文件大小。
	virtual int64		FileSize(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("FileSize %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		int64 Result = LowerLevel->FileSize(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("FileSize return %lld [%fms]"), Result, ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// DeleteFile：删除文件。
	virtual bool		DeleteFile(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("DeleteFile %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->DeleteFile(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("DeleteFile return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// IsReadOnly：查询是否只读。
	virtual bool		IsReadOnly(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("IsReadOnly %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->IsReadOnly(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("IsReadOnly return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// MoveFile：移动/重命名文件。
	virtual bool		MoveFile(const TCHAR* To, const TCHAR* From) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("MoveFile %s %s"), To, From);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->MoveFile(To, From);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("MoveFile return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// SetReadOnly：设置只读属性。
	virtual bool		SetReadOnly(const TCHAR* Filename, bool bNewReadOnlyValue) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("SetReadOnly %s %d"), Filename, int32(bNewReadOnlyValue));
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->SetReadOnly(Filename, bNewReadOnlyValue);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("SetReadOnly return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// GetTimeStamp：取文件修改时间。
	virtual FDateTime	GetTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("GetTimeStamp %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		FDateTime Result = LowerLevel->GetTimeStamp(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：Ticks + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("GetTimeStamp return %llx [%fms]"), Result.GetTicks(), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// SetTimeStamp：设置文件修改时间。
	virtual void		SetTimeStamp(const TCHAR* Filename, FDateTime DateTime) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("SetTimeStamp %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		LowerLevel->SetTimeStamp(Filename, DateTime);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：耗时（无返回值）。
		FILE_LOG(LogPlatformFile, Log, TEXT("SetTimeStamp [%fms]"), ThisTime);
// 函数体结束。
	}
// GetAccessTimeStamp：取最后访问时间。
	virtual FDateTime	GetAccessTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("GetAccessTimeStamp %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		FDateTime Result = LowerLevel->GetAccessTimeStamp(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：Ticks + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("GetAccessTimeStamp return %llx [%fms]"), Result.GetTicks(), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// GetFilenameOnDisk：取磁盘上真实大小写的路径。
	virtual FString	GetFilenameOnDisk(const TCHAR* Filename) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("GetFilenameOnDisk %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		FString Result = LowerLevel->GetFilenameOnDisk(Filename);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：结果字符串 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("GetFilenameOnDisk return %s [%fms]"), *Result, ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// OpenRead：打开文件读。
	virtual IFileHandle*	OpenRead(const TCHAR* Filename, bool bAllowWrite) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("OpenRead %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 OpenRead。
		IFileHandle* Result = LowerLevel->OpenRead(Filename, bAllowWrite);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：句柄地址 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("OpenRead return %llx [%fms]"), uint64(Result), ThisTime);
// 成功就包一层 FLoggedFileHandle，失败直接返回空。
		return Result ? (new FLoggedFileHandle(Result, Filename, *this)) : Result;
// 函数体结束。
	}
// OpenWrite：打开文件写。
	virtual IFileHandle*	OpenWrite(const TCHAR* Filename, bool bAppend = false, bool bAllowRead = false) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("OpenWrite %s %d %d"), Filename, int32(bAppend), int32(bAllowRead));
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 OpenWrite。
		IFileHandle* Result = LowerLevel->OpenWrite(Filename, bAppend, bAllowRead);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：句柄地址 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("OpenWrite return %llx [%fms]"), uint64(Result), ThisTime);
// 成功就包一层 FLoggedFileHandle，失败直接返回空。
		return Result ? (new FLoggedFileHandle(Result, Filename, *this)) : Result;
// 函数体结束。
	}

// DirectoryExists：查询目录是否存在。
	virtual bool		DirectoryExists(const TCHAR* Directory) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("DirectoryExists %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->DirectoryExists(Directory);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("DirectoryExists return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// CreateDirectory：创建目录。
	virtual bool		CreateDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("CreateDirectory %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->CreateDirectory(Directory);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("CreateDirectory return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// DeleteDirectory：删除目录。
	virtual bool		DeleteDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("DeleteDirectory %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->DeleteDirectory(Directory);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("DeleteDirectory return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// GetStatData：取文件/目录元数据。
	virtual FFileStatData GetStatData(const TCHAR* FilenameOrDirectory) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("GetStatData %s"), FilenameOrDirectory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		FFileStatData Result = LowerLevel->GetStatData(FilenameOrDirectory);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：是否有效 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("GetStatData return %d [%fms]"), int32(Result.bIsValid), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
```

**本段新元素**

- `SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald)` —— 打命名事件（Insights / PIX 可见）
- `FString::Printf(TEXT("Xxx %s"), Filename)` —— 拼事件名
- `FileExists` / `FileSize` / `DeleteFile` / `IsReadOnly` / `MoveFile` / `SetReadOnly` —— 6 个文件操作
- `GetTimeStamp` / `SetTimeStamp` / `GetAccessTimeStamp` —— 3 个时间操作
- `GetFilenameOnDisk` —— 1 个路径操作
- `OpenRead` / `OpenWrite` —— 2 个打开操作（**包一层 `FLoggedFileHandle`**）
- `DirectoryExists` / `CreateDirectory` / `DeleteDirectory` —— 3 个目录操作
- `GetStatData` —— 1 个元数据操作
- `Result ? (new FLoggedFileHandle(Result, Filename, *this)) : Result` —— 成功才包装

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 188~198 行 | `FileExists` |
| 第 199~209 行 | `FileSize` |
| 第 210~220 行 | `DeleteFile` |
| 第 221~231 行 | `IsReadOnly` |
| 第 232~242 行 | `MoveFile` |
| 第 243~253 行 | `SetReadOnly` |
| 第 254~264 行 | `GetTimeStamp` |
| 第 265~274 行 | `SetTimeStamp` |
| 第 275~285 行 | `GetAccessTimeStamp` |
| 第 286~296 行 | `GetFilenameOnDisk` |
| 第 297~307 行 | `OpenRead`（包一层） |
| 第 308~318 行 | `OpenWrite`（包一层） |
| 第 320~330 行 | `DirectoryExists` |
| 第 331~341 行 | `CreateDirectory` |
| 第 342~352 行 | `DeleteDirectory` |
| 第 353~363 行 | `GetStatData` |

> 类比：像给每个窗口都装摄像头：谁来办事、办多久，全录下来。

### 七、`FLogVisitor` + 2 个 `IterateDirectory`（第 365~407 行）

```cpp
// FLogVisitor：带日志的目录访问者，包住用户的访问者。
	class FLogVisitor : public IPlatformFile::FDirectoryVisitor
	{
// 公有区。
	public:
// Visitor：用户的访问者引用。
		FDirectoryVisitor&	Visitor;
// 构造函数：保存用户访问者。
		FLogVisitor(FDirectoryVisitor& InVisitor)
// 初始化列表：保存引用。
			: Visitor(InVisitor)
// 函数体开始。
		{
// 函数体结束。
		}
// Visit：访问一个文件或目录。
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
// 函数体开始。
		{
// 打日志：路径 + 是否目录。
			FILE_LOG(LogPlatformFile, Verbose, TEXT("Visit %s %d"), FilenameOrDirectory, int32(bIsDirectory));
// 记录开始时间。
			double StartTime = FPlatformTime::Seconds();
// 调用户访问者（CallShouldVisitAndVisit 会先问 ShouldVisit 再 Visit）。
			bool Result = Visitor.CallShouldVisitAndVisit(FilenameOrDirectory, bIsDirectory);
// 计算耗时。
			double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
			FILE_LOG(LogPlatformFile, Verbose, TEXT("Visit return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
			return Result;
// 函数体结束。
		}
// 类结束。
	};

// IterateDirectory：遍历单层目录。
	virtual bool		IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("IterateDirectory %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 包一层 FLogVisitor。
		FLogVisitor LogVisitor(Visitor);
// 调底层遍历（传包装后的访问者）。
		bool Result = LowerLevel->IterateDirectory(Directory, LogVisitor);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("IterateDirectory return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// IterateDirectoryRecursively：递归遍历目录。
	virtual bool		IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("IterateDirectoryRecursively %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 包一层 FLogVisitor。
		FLogVisitor LogVisitor(Visitor);
// 调底层递归遍历。
		bool Result = LowerLevel->IterateDirectoryRecursively(Directory, LogVisitor);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("IterateDirectoryRecursively return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
```

**本段新元素**

- `class FLogVisitor : public IPlatformFile::FDirectoryVisitor` —— 带日志的目录访问者
- `FDirectoryVisitor& Visitor` —— 用户的访问者引用
- `Visit(FilenameOrDirectory, bIsDirectory)` —— 访问一个文件或目录
- `Visitor.CallShouldVisitAndVisit(...)` —— 先问 `ShouldVisit` 再 `Visit`
- `IterateDirectory(Directory, Visitor)` —— 遍历单层目录（包一层 `FLogVisitor`）
- `IterateDirectoryRecursively(Directory, Visitor)` —— 递归遍历

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 365~382 行 | `FLogVisitor` 内部类 |
| 第 384~395 行 | `IterateDirectory` |
| 第 396~407 行 | `IterateDirectoryRecursively` |

> 类比：像给门卫配个记录本：每个进出的人（文件）都记一笔。

### 八、`FLogStatVisitor` + 2 个 `IterateDirectoryStat`（第 409~451 行）

```cpp
// FLogStatVisitor：带日志的 stat 访问者。
	class FLogStatVisitor : public IPlatformFile::FDirectoryStatVisitor
	{
// 公有区。
	public:
// Visitor：用户的 stat 访问者引用。
		FDirectoryStatVisitor&	Visitor;
// 构造函数：保存用户访问者。
		FLogStatVisitor(FDirectoryStatVisitor& InVisitor)
// 初始化列表：保存引用。
			: Visitor(InVisitor)
// 函数体开始。
		{
// 函数体结束。
		}
// Visit：访问一个文件或目录（带 stat 数据）。
		virtual bool Visit(const TCHAR* FilenameOrDirectory, const FFileStatData& StatData) override
// 函数体开始。
		{
// 打日志：路径 + 是否目录。
			FILE_LOG(LogPlatformFile, Verbose, TEXT("Visit %s %d"), FilenameOrDirectory, int32(StatData.bIsDirectory));
// 记录开始时间。
			double StartTime = FPlatformTime::Seconds();
// 调用户访问者。
			bool Result = Visitor.CallShouldVisitAndVisit(FilenameOrDirectory, StatData);
// 计算耗时。
			double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
			FILE_LOG(LogPlatformFile, Verbose, TEXT("Visit return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
			return Result;
// 函数体结束。
		}
// 类结束。
	};

// IterateDirectoryStat：遍历单层目录（带 stat）。
	virtual bool		IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("IterateDirectoryStat %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 包一层 FLogStatVisitor。
		FLogStatVisitor LogVisitor(Visitor);
// 调底层遍历。
		bool Result = LowerLevel->IterateDirectoryStat(Directory, LogVisitor);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("IterateDirectoryStat return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// IterateDirectoryStatRecursively：递归遍历（带 stat）。
	virtual bool		IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("IterateDirectoryStatRecursively %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 包一层 FLogStatVisitor。
		FLogStatVisitor LogVisitor(Visitor);
// 调底层递归遍历。
		bool Result = LowerLevel->IterateDirectoryStatRecursively(Directory, LogVisitor);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("IterateDirectoryStatRecursively return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
```

**本段新元素**

- `class FLogStatVisitor : public IPlatformFile::FDirectoryStatVisitor` —— 带日志的 stat 访问者
- `FDirectoryStatVisitor& Visitor` —— 用户的 stat 访问者引用
- `Visit(FilenameOrDirectory, StatData)` —— 访问一个文件或目录（带 stat 数据）
- `IterateDirectoryStat(Directory, Visitor)` —— 遍历单层（带 stat）
- `IterateDirectoryStatRecursively(Directory, Visitor)` —— 递归遍历（带 stat）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 409~426 行 | `FLogStatVisitor` 内部类 |
| 第 428~439 行 | `IterateDirectoryStat` |
| 第 440~451 行 | `IterateDirectoryStatRecursively` |

> 类比：像门卫不仅记名字，还记身高体重（stat 数据）。

### 九、`DeleteDirectoryRecursively` + `CopyFile`（第 453~474 行）

```cpp
// DeleteDirectoryRecursively：递归删除目录。
	virtual bool		DeleteDirectoryRecursively(const TCHAR* Directory) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("DeleteDirectoryRecursively %s"), Directory);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->DeleteDirectoryRecursively(Directory);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("DeleteDirectoryRecursively return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
// CopyFile：复制文件。
	virtual bool		CopyFile(const TCHAR* To, const TCHAR* From, EPlatformFileRead ReadFlags = EPlatformFileRead::None, EPlatformFileWrite WriteFlags = EPlatformFileWrite::None) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("CopyFile %s %s"), To, From);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层。
		bool Result = LowerLevel->CopyFile(To, From, ReadFlags, WriteFlags);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：返回值 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("CopyFile return %d [%fms]"), int32(Result), ThisTime);
// 返回结果。
		return Result;
// 函数体结束。
	}
```

**本段新元素**

- `DeleteDirectoryRecursively(Directory)` —— 递归删除目录
- `CopyFile(To, From, ReadFlags, WriteFlags)` —— 复制文件

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 453~463 行 | `DeleteDirectoryRecursively` |
| 第 464~474 行 | `CopyFile` |

> 类比：同上，只是换了个动作。

### 十、句柄计数（仅非 Shipping）（第 476~493 行）

```cpp
// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING
// OnHandleOpen：记录一次文件打开。
	void OnHandleOpen(const FString& Filename)
// 函数体开始。
	{
// 加锁保护 OpenHandles。
		FScopeLock LogFileLock(&LogFileCritical);
// 找到或新建该文件的计数。
		int32& NumOpenHandles = OpenHandles.FindOrAdd(Filename);
// 计数 +1。
		NumOpenHandles++;
// 函数体结束。
	}
// OnHandleClosed：记录一次文件关闭。
	void OnHandleClosed(const FString& Filename)
// 函数体开始。
	{
// 加锁保护 OpenHandles。
		FScopeLock LogFileLock(&LogFileCritical);
// 取该文件的计数（必须存在）。
		int32& NumOpenHandles = OpenHandles.FindChecked(Filename);
// 减 1 后如果归零。
		if (--NumOpenHandles == 0)
// 条件体开始。
		{
// 从表里移除。
			OpenHandles.Remove(Filename);
// 条件体结束。
		}
// 函数体结束。
	}
// HandleDumpCommand：控制台命令，打印当前打开的文件（实现在 .cpp）。
	CORE_API void HandleDumpCommand(const TCHAR* Cmd, FOutputDevice& Ar);
// 条件编译结束。
#endif
```

**本段新元素**

- `#if !UE_BUILD_SHIPPING` —— 只在非 Shipping 构建里编译
- `OnHandleOpen(Filename)` —— 记录一次文件打开
- `FScopeLock LogFileLock(&LogFileCritical)` —— 加锁保护 `OpenHandles`
- `OpenHandles.FindOrAdd(Filename)` —— 找到或新建计数
- `NumOpenHandles++` —— 计数 +1
- `OnHandleClosed(Filename)` —— 记录一次文件关闭
- `OpenHandles.FindChecked(Filename)` —— 取计数（必须存在）
- `if (--NumOpenHandles == 0)` —— 减 1 后归零
- `OpenHandles.Remove(Filename)` —— 从表里移除
- `HandleDumpCommand(Cmd, Ar)` —— 控制台命令，打印当前打开的文件（实现在 .cpp）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 476 行 | `#if !UE_BUILD_SHIPPING` |
| 第 477~482 行 | `OnHandleOpen` |
| 第 483~491 行 | `OnHandleClosed` |
| 第 492 行 | `HandleDumpCommand` |
| 第 493 行 | `#endif` |

> 类比：像图书馆的借阅登记：借的时候 +1，还的时候 -1，归零就划掉。

### 十一、`OpenAsyncRead` + `OpenMappedEx` + `SetAsyncMinimumPriority`（第 494~513 行）

```cpp
// OpenAsyncRead：打开异步读。
	virtual IAsyncReadFileHandle* OpenAsyncRead(const TCHAR* Filename, bool bAllowWrite = false) override
// 函数体开始。
	{
// 拼出命名事件字符串。
		FString DataStr = FString::Printf(TEXT("OpenAsyncRead %s"), Filename);
// 打命名事件。
		SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);
// 打日志。
		FILE_LOG(LogPlatformFile, Log, TEXT("%s"), *DataStr);
// 记录开始时间。
		double StartTime = FPlatformTime::Seconds();
// 调底层 OpenAsyncRead。
		IAsyncReadFileHandle* Result = LowerLevel->OpenAsyncRead(Filename, bAllowWrite);
// 计算耗时。
		double ThisTime = (FPlatformTime::Seconds() - StartTime) / 1000.0;
// 打日志：句柄地址 + 耗时。
		FILE_LOG(LogPlatformFile, Log, TEXT("OpenAsyncRead return %llx [%fms]"), uint64(Result), ThisTime);
// 待办：异步文件句柄还没有包装日志。
		//@todo 异步文件句柄还没有包装日志（暂时）
// 直接返回（不包装）。
		return Result;
// 函数体结束。
	}
// OpenMappedEx：打开映射文件（实现在 .cpp）。
	virtual FOpenMappedResult OpenMappedEx(const TCHAR* Filename, EOpenReadFlags OpenOptions = EOpenReadFlags::None, int64 MaximumSize = 0) override;

// SetAsyncMinimumPriority：设置异步 IO 最低优先级。
	virtual void SetAsyncMinimumPriority(EAsyncIOPriorityAndFlags Priority) override
// 函数体开始。
	{
// 转调底层。
		LowerLevel->SetAsyncMinimumPriority(Priority);
// 函数体结束。
	}

// 类结束。
};
```

**本段新元素**

- `OpenAsyncRead(Filename, bAllowWrite)` —— 打开异步读（**不包装**，直接返回）
- `//@todo no wrapped logging for async file handles (yet)` —— 引擎待办注释
- `OpenMappedEx(Filename, OpenOptions, MaximumSize)` —— 打开映射文件（实现在 .cpp）
- `SetAsyncMinimumPriority(MinPriority)` —— 设置异步 IO 最低优先级（纯转调）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 494~505 行 | `OpenAsyncRead` |
| 第 506 行 | `OpenMappedEx` |
| 第 508~511 行 | `SetAsyncMinimumPriority` |
| 第 513 行 | 类结束 |

> 类比：像异步快递：还没装监控（待办），先直接放行。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 14 个 include + 4 个前置声明 | 1~23 | 版权头 + 14 个 include + 4 个前置声明 |
| 日志分类 + `FILE_LOG` 防递归宏 | 25~40 | 日志分类 + `FILE_LOG` 防递归宏 |
| `FLoggedFileHandle` 声明 + 成员 | 42~52 | `FLoggedFileHandle` 声明 + 成员 |
| `FLoggedFileHandle` 的 10 个方法（三段式） | 54~142 | `FLoggedFileHandle` 的 10 个方法（三段式） |
| `FLoggedPlatformFile` 声明 + 成员 + 基础方法 | 145~186 | `FLoggedPlatformFile` 声明 + 成员 + 基础方法 |
| 15 个文件操作（每个都是三段式 + 命名事件） | 188~363 | 15 个文件操作（每个都是三段式 + 命名事件） |
| `FLogVisitor` + 2 个 `IterateDirectory` | 365~407 | `FLogVisitor` + 2 个 `IterateDirectory` |
| `FLogStatVisitor` + 2 个 `IterateDirectoryStat` | 409~451 | `FLogStatVisitor` + 2 个 `IterateDirectoryStat` |
| `DeleteDirectoryRecursively` + `CopyFile` | 453~474 | `DeleteDirectoryRecursively` + `CopyFile` |
| 句柄计数（仅非 Shipping） | 476~493 | 句柄计数（仅非 Shipping） |
| `OpenAsyncRead` + `OpenMappedEx` + `SetAsyncMinimumPriority` | 494~513 | `OpenAsyncRead` + `OpenMappedEx` + `SetAsyncMinimumPriority` |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#define X(...) \` | 多行宏（反斜杠续行） | `#define FILE_LOG(...) \` |
| `##__VA_ARGS__` | 可变参数透传（GCC 扩展） | `UE_LOG(Cat, Verb, Format, ##__VA_ARGS__)` |
| `extern bool X;` | 声明外部定义的全局量 | `extern bool bSuppressFileLog;` |
| `TUniquePtr<X>` | 独占智能指针 | `TUniquePtr<IFileHandle> FileHandle;` |
| `TMap<K, V>` | 哈希表 | `TMap<FString, int32> OpenHandles;` |
| `FScopeLock Lock(&Critical)` | 作用域锁（RAII） | `FScopeLock LogFileLock(&LogFileCritical);` |
| `Map.FindOrAdd(Key)` | 找到或新建 | `OpenHandles.FindOrAdd(Filename)` |
| `Map.FindChecked(Key)` | 找到（不存在则断言） | `OpenHandles.FindChecked(Filename)` |
| `SCOPED_NAMED_EVENT_FSTRING(Str, Color)` | 打命名事件 | `SCOPED_NAMED_EVENT_FSTRING(DataStr, FColor::Emerald);` |
| `X ? A : B` | 三元运算符 | `Result ? (new FLoggedFileHandle(...)) : Result` |
| `class X : public Y::Z` | 继承内部类 | `class FLogVisitor : public IPlatformFile::FDirectoryVisitor` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 513 行 | 992 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 479 行 | 479 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |
| `//@todo` 待办 | 保留原文格式 | 保留 `//@todo` + 中文说明 | 待办标记，保留可搜索性 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`（992 行）
2. 脚本验收：`codeOurs=479 / codeEngine=479 / diff=0`
3. 确认后下一个：`HAL\IPlatformFileManagedStorageWrapper.h`（31770 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`（513 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`（992 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\14_IPlatformFileLogWrapper.h_总md.md`