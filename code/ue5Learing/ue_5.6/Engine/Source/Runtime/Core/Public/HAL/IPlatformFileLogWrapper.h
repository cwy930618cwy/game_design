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
// 类结束。
};

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
