// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入异步文件句柄：IAsyncReadFileHandle / IAsyncReadRequest。
#include "Async/AsyncFileHandle.h"
// 引入 TArray：动态数组。
#include "Containers/Array.h"
// 引入 TMap：哈希表。
#include "Containers/Map.h"
// 引入字符串转换：StringCast<ANSICHAR>。
#include "Containers/StringConv.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入基础类型层：uint8 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入通用平台文件接口：IPlatformFile / IFileHandle / FFileStatData。
#include "GenericPlatform/GenericPlatformFile.h"
// 引入临界区：FCriticalSection。
#include "HAL/CriticalSection.h"
// 引入平台 C 运行时。
#include "HAL/PlatformCrt.h"
// 引入平台杂项：FPlatformMisc。
#include "HAL/PlatformMisc.h"
// 引入平台属性：FPlatformProperties。
#include "HAL/PlatformProperties.h"
// 引入数据驱动平台信息注册表。
#include "Misc/DataDrivenPlatformInfoRegistry.h"
// 引入 FDateTime：文件时间戳类型。
#include "Misc/DateTime.h"
// 引入命令行解析：FParse::Param。
#include "Misc/Parse.h"
// 引入路径工具：FPaths。
#include "Misc/Paths.h"

// 前置声明 FPlatformFileOpenLog。
class FPlatformFileOpenLog;
// 前置声明 IAsyncReadFileHandle。
class IAsyncReadFileHandle;
// 前置声明 IMappedFileHandle。
class IMappedFileHandle;

// 如果不是 Shipping 构建（文件打开日志只在开发构建里启用）。
#if !UE_BUILD_SHIPPING

// FILE_OPEN_ORDER_LABEL：往打开日志里写一条标签的宏。
#define FILE_OPEN_ORDER_LABEL(Format, ...) \
// 转调 FPlatformFileOpenLog::AddLabel。
	FPlatformFileOpenLog::AddLabel( Format, ##__VA_ARGS__ );

// FLoggingAsyncReadFileHandle：带日志的异步读句柄，final 表示不可继承。
class FLoggingAsyncReadFileHandle final : public IAsyncReadFileHandle
{
// Owner：所属的平台文件包装器。
	FPlatformFileOpenLog* Owner;
// Filename：文件名。
	FString Filename;
// ActualRequest：真正的异步读句柄。
	IAsyncReadFileHandle* ActualRequest;
// 公有区。
public:
// 构造函数：保存所属包装器、文件名、真正的句柄。
	FLoggingAsyncReadFileHandle(FPlatformFileOpenLog* InOwner, const TCHAR* InFilename, IAsyncReadFileHandle* InActualRequest)
// 初始化列表：保存 Owner。
		: Owner(InOwner)
// 初始化列表：保存文件名。
		, Filename(InFilename)
// 初始化列表：保存真正的句柄。
		, ActualRequest(InActualRequest)
// 函数体开始。
	{
// 函数体结束。
	}
// 析构函数：删除真正的句柄。
	~FLoggingAsyncReadFileHandle()
// 函数体开始。
	{
// 删除真正的句柄。
		delete ActualRequest;
// 函数体结束。
	}
// SizeRequest：请求文件大小（直接转调）。
	virtual IAsyncReadRequest* SizeRequest(FAsyncFileCallBack* CompleteCallback = nullptr) override
// 函数体开始。
	{
// 转调真正的句柄。
		return ActualRequest->SizeRequest(CompleteCallback);
// 函数体结束。
	}
// ReadRequest：请求读数据（实现在 .cpp，会记录打开顺序）。
	virtual IAsyncReadRequest* ReadRequest(int64 Offset, int64 BytesToRead, EAsyncIOPriorityAndFlags PriorityAndFlags = AIOP_Normal, FAsyncFileCallBack* CompleteCallback = nullptr, uint8* UserSuppliedMemory = nullptr) override;
// 类结束。
};


// FPlatformFileOpenLog：记录文件打开顺序的平台文件包装器。
class FPlatformFileOpenLog : public IPlatformFile
{
// 受保护区。
protected:

// LowerLevel：被包装的下一层平台文件。
	IPlatformFile*			LowerLevel;
// CriticalSection：保护日志写入的临界区。
	FCriticalSection		CriticalSection;
// OpenOrder：打开顺序计数器。
	int64					OpenOrder;
// FilenameAccessMap：文件名 → 打开顺序号。
	TMap<FString, int64>	FilenameAccessMap;
// LogOutput：日志输出文件句柄列表（可同时写多个文件）。
	TArray<IFileHandle*>	LogOutput;
// bLogDuplicates：是否记录重复打开。
	bool					bLogDuplicates;

// 公有区。
public:

// 默认构造函数：LowerLevel 置空，计数器归零，不记录重复。
	FPlatformFileOpenLog()
// 初始化列表：LowerLevel 为空。
		: LowerLevel(nullptr)
// 初始化列表：打开顺序从 0 开始。
		, OpenOrder(0)
// 初始化列表：默认不记录重复。
		, bLogDuplicates(false)
// 函数体开始。
	{
// 函数体结束。
	}

// 虚析构函数（空）。
	virtual ~FPlatformFileOpenLog()
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

// ShouldBeUsed：判断是否启用本包装器。
	virtual bool ShouldBeUsed(IPlatformFile* Inner, const TCHAR* CmdLine) const override
// 函数体开始。
	{
// 命令行有 -FileOpenLog 或 -FilePackageOpenLog 就启用。
		bool bResult = FParse::Param(CmdLine, TEXT("FileOpenLog")) || FParse::Param(CmdLine, TEXT("FilePackageOpenLog"));
// 返回结果。
		return bResult;
// 函数体结束。
	}

// Initialize：初始化（实现在 .cpp）。
	virtual bool Initialize(IPlatformFile* Inner, const TCHAR* CommandLineParam) override;

// GetLowerLevel：取下一层平台文件。
	virtual IPlatformFile* GetLowerLevel() override
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
// GetTypeName：返回类型名。
	static const TCHAR* GetTypeName()
// 函数体开始。
	{
// 返回 FileOpenLog。
		return TEXT("FileOpenLog");
// 函数体结束。
	}
// GetName：返回本包装器的名字。
	virtual const TCHAR* GetName() const override
// 函数体开始。
	{
// 返回类型名。
		return GetTypeName();
// 函数体结束。
	}
// FileExists：转调底层。
	virtual bool		FileExists(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->FileExists(Filename);
// 函数体结束。
	}
// FileSize：转调底层。
	virtual int64		FileSize(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->FileSize(Filename);
// 函数体结束。
	}
// DeleteFile：转调底层。
	virtual bool		DeleteFile(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteFile(Filename);
// 函数体结束。
	}
// IsReadOnly：转调底层。
	virtual bool		IsReadOnly(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IsReadOnly(Filename);
// 函数体结束。
	}
// MoveFile：转调底层。
	virtual bool		MoveFile(const TCHAR* To, const TCHAR* From) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->MoveFile(To, From);
// 函数体结束。
	}
// SetReadOnly：转调底层。
	virtual bool		SetReadOnly(const TCHAR* Filename, bool bNewReadOnlyValue) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->SetReadOnly(Filename, bNewReadOnlyValue);
// 函数体结束。
	}
// GetTimeStamp：转调底层。
	virtual FDateTime	GetTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetTimeStamp(Filename);
// 函数体结束。
	}
// SetTimeStamp：转调底层。
	virtual void		SetTimeStamp(const TCHAR* Filename, FDateTime DateTime) override
// 函数体开始。
	{
// 转调。
		LowerLevel->SetTimeStamp(Filename, DateTime);
// 函数体结束。
	}
// GetAccessTimeStamp：转调底层。
	virtual FDateTime	GetAccessTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetAccessTimeStamp(Filename);
// 函数体结束。
	}
// GetFilenameOnDisk：转调底层。
	virtual FString	GetFilenameOnDisk(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetFilenameOnDisk(Filename);
// 函数体结束。
	}
// OpenRead：打开文件读（★ 记录打开顺序）。
	virtual IFileHandle*	OpenRead(const TCHAR* Filename, bool bAllowWrite) override
// 函数体开始。
	{
// 先让底层打开。
		IFileHandle* Result = LowerLevel->OpenRead(Filename, bAllowWrite);
// 如果打开成功。
		if (Result)
// 条件体开始。
		{
// 记录到打开日志。
			AddToOpenLog(Filename);
// 条件体结束。
		}
// 返回句柄。
		return Result;
// 函数体结束。
	}
// OpenWrite：打开文件写（不记录）。
	virtual IFileHandle*	OpenWrite(const TCHAR* Filename, bool bAppend = false, bool bAllowRead = false) override
// 函数体开始。
	{
// 直接转调底层。
		return LowerLevel->OpenWrite(Filename, bAppend, bAllowRead);
// 函数体结束。
	}
// DirectoryExists：转调底层。
	virtual bool		DirectoryExists(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DirectoryExists(Directory);
// 函数体结束。
	}
// CreateDirectory：转调底层。
	virtual bool		CreateDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CreateDirectory(Directory);
// 函数体结束。
	}
// DeleteDirectory：转调底层。
	virtual bool		DeleteDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteDirectory(Directory);
// 函数体结束。
	}
// GetStatData：转调底层。
	virtual FFileStatData GetStatData(const TCHAR* FilenameOrDirectory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetStatData(FilenameOrDirectory);
// 函数体结束。
	}
// IterateDirectory：转调底层。
	virtual bool		IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectory( Directory, Visitor );
// 函数体结束。
	}
// IterateDirectoryRecursively：转调底层。
	virtual bool		IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryRecursively( Directory, Visitor );
// 函数体结束。
	}
// IterateDirectoryStat：转调底层。
	virtual bool		IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryStat( Directory, Visitor );
// 函数体结束。
	}
// IterateDirectoryStatRecursively：转调底层。
	virtual bool		IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryStatRecursively( Directory, Visitor );
// 函数体结束。
	}
// DeleteDirectoryRecursively：转调底层。
	virtual bool		DeleteDirectoryRecursively(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteDirectoryRecursively( Directory );
// 函数体结束。
	}
// CopyFile：转调底层。
	virtual bool		CopyFile(const TCHAR* To, const TCHAR* From, EPlatformFileRead ReadFlags = EPlatformFileRead::None, EPlatformFileWrite WriteFlags = EPlatformFileWrite::None) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CopyFile( To, From, ReadFlags, WriteFlags);
// 函数体结束。
	}
// CreateDirectoryTree：转调底层。
	virtual bool		CreateDirectoryTree(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CreateDirectoryTree(Directory);
// 函数体结束。
	}
// CopyDirectoryTree：转调底层。
	virtual bool		CopyDirectoryTree(const TCHAR* DestinationDirectory, const TCHAR* Source, bool bOverwriteAllExisting) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CopyDirectoryTree(DestinationDirectory, Source, bOverwriteAllExisting);
// 函数体结束。
	}
// ConvertToAbsolutePathForExternalAppForRead：转调底层。
	virtual FString		ConvertToAbsolutePathForExternalAppForRead( const TCHAR* Filename ) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->ConvertToAbsolutePathForExternalAppForRead(Filename);
// 函数体结束。
	}
// ConvertToAbsolutePathForExternalAppForWrite：转调底层。
	virtual FString		ConvertToAbsolutePathForExternalAppForWrite( const TCHAR* Filename ) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->ConvertToAbsolutePathForExternalAppForWrite(Filename);
// 函数体结束。
	}
// SendMessageToServer：转调底层。
	virtual bool		SendMessageToServer(const TCHAR* Message, IFileServerMessageHandler* Handler) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->SendMessageToServer(Message, Handler);
// 函数体结束。
	}
// OpenAsyncRead：打开异步读（★ 包一层 FLoggingAsyncReadFileHandle）。
	virtual IAsyncReadFileHandle* OpenAsyncRead(const TCHAR* Filename, bool bAllowWrite = false) override
// 函数体开始。
	{
// 说明：这里不能记录「打开」，真正重要的是「开始读」的时刻。
		// 这里不能记录「打开」……真正重要的是「开始读」的时刻！
// 包一层带日志的异步读句柄。
		return new FLoggingAsyncReadFileHandle(this, Filename, LowerLevel->OpenAsyncRead(Filename, bAllowWrite));
// 函数体结束。
	}
// OpenMappedEx：打开映射文件（实现在 .cpp）。
	virtual FOpenMappedResult OpenMappedEx(const TCHAR* Filename, EOpenReadFlags OpenOptions = EOpenReadFlags::None, int64 MaximumSize = 0) override;

// AddToOpenLog：把文件加入打开日志。
	void AddToOpenLog(const TCHAR* Filename)
// 函数体开始。
	{
// 加锁。
		CriticalSection.Lock();
// 如果记录重复。
		if (bLogDuplicates)
// 条件体开始。
		{
// 拼出日志行（只有文件名）。
			FString Text = FString::Printf(TEXT("\"%s\"\n"), Filename);
// 遍历所有输出文件。
			for (auto File = LogOutput.CreateIterator(); File; ++File)
// 循环体开始。
			{
// 写入（转成 ANSI 编码）。
				(*File)->Write((uint8*)StringCast<ANSICHAR>(*Text).Get(), Text.Len());
// 循环体结束。
			}
// 条件体结束。
		}
// 否则如果该文件还没记录过。
		else if (FilenameAccessMap.Find(Filename) == nullptr)
// 条件体开始。
		{
// 记录文件名 → 顺序号。
			FilenameAccessMap.Emplace(Filename, ++OpenOrder);
// 拼出日志行（文件名 + 顺序号）。
			FString Text = FString::Printf(TEXT("\"%s\" %llu\n"), Filename, OpenOrder);
// 遍历所有输出文件。
			for (auto File = LogOutput.CreateIterator(); File; ++File)
// 循环体开始。
			{
// 写入。
				(*File)->Write((uint8*)StringCast<ANSICHAR>(*Text).Get(), Text.Len());
// 循环体结束。
			}
// 条件体结束。
		}
// 解锁。
		CriticalSection.Unlock();
// 函数体结束。
	}

// AddLabelToOpenLog：往打开日志里写一条标签。
	void AddLabelToOpenLog(const TCHAR* LabelStr)
// 函数体开始。
	{
// 加锁。
		CriticalSection.Lock();
// 遍历所有输出文件。
		for (auto File = LogOutput.CreateIterator(); File; ++File)
// 循环体开始。
		{
// 拼出标签行（# 开头）。
			FString Text = FString::Printf(TEXT("# %s\n"), LabelStr);
// 写入。
			(*File)->Write((uint8*)StringCast<ANSICHAR>(*Text).Get(), Text.Len());
// 循环体结束。
		}
// 解锁。
		CriticalSection.Unlock();
// 函数体结束。
	}


// AddPackageToOpenLog：把包加入打开日志（旧接口）。
	void AddPackageToOpenLog(const TCHAR* Filename)
// 函数体开始。
	{
// 待办：这个函数是否该废弃/移除？
		// 待办：这个函数是否该废弃/移除？
// 转调 AddToOpenLog。
		AddToOpenLog(Filename);
// 函数体结束。
	}

// AddLabel：静态模板方法，格式化后写标签。
	template <typename FmtType, typename... Types>
// 函数体开始。
	static void AddLabel(const FmtType& Fmt, Types... Args)
	{
// 静态断言：格式串必须是 TCHAR 数组。
		static_assert(TIsArrayOrRefOfTypeByPredicate<FmtType, TIsCharEncodingCompatibleWithTCHAR>::Value, "Formatting string must be a TCHAR array.");
// 静态断言：所有参数类型必须有效。
		static_assert((TIsValidVariadicFunctionArg<Types>::Value && ...), "Invalid argument(s) passed to FCsvProfiler::AddLabel");
// 转调 AddLabelInternal。
		AddLabelInternal((const TCHAR*)Fmt, Args...);
// 函数体结束。
	}

// 私有区。
private:
// AddLabelInternal：可变参数的内部实现（实现在 .cpp）。
	static CORE_API void VARARGS AddLabelInternal(const TCHAR* Fmt, ...);

// 类结束。
};
// 否则（Shipping 构建）。
#else // !UE_BUILD_SHIPPING

// FILE_OPEN_ORDER_LABEL：Shipping 下是空宏。
#define FILE_OPEN_ORDER_LABEL(Format, ...)

// 条件编译结束。
#endif

