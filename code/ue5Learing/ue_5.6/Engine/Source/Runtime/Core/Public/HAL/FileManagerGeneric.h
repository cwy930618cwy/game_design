// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入 TArray：文件列表容器。
#include "Containers/Array.h"
// 引入容器前置声明。
#include "Containers/ContainersFwd.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入基础类型层：uint8 / uint32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入通用平台文件接口：IPlatformFile / IFileHandle / FFileStatData。
#include "GenericPlatform/GenericPlatformFile.h"
// 引入文件管理器接口：IFileManager（本类的基类）。
#include "HAL/FileManager.h"
// 引入平台文件管理器：FPlatformFileManager（取底层 IPlatformFile）。
#include "HAL/PlatformFileManager.h"
// 引入 FDateTime：文件时间戳类型。
#include "Misc/DateTime.h"
// 引入 FArchive：序列化基类（读写都通过它）。
#include "Serialization/Archive.h"
// 引入 TUniquePtr：独占智能指针（管理 IFileHandle 生命周期）。
#include "Templates/UniquePtr.h"

// 如果平台没定义读缓冲大小。
#ifndef PLATFORM_FILE_READER_BUFFER_SIZE
// 默认 1024 字节。
 #define PLATFORM_FILE_READER_BUFFER_SIZE 1024
// 条件编译结束。
#endif

// 如果平台没定义写缓冲大小。
#ifndef PLATFORM_FILE_WRITER_BUFFER_SIZE
// 默认 4096 字节。
 #define PLATFORM_FILE_WRITER_BUFFER_SIZE 4096
// 条件编译结束。
#endif 

// 如果平台没定义调试文件写缓冲大小。
#ifndef PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE
// 默认 4096 字节。
 #define PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE 4096
// 条件编译结束。
#endif 

/**
 * 文件管理器的基类。
 *
 * 这个基类通过提供简单、未优化的实现来简化 IFileManager 的实现，
 * 这些实现可以从其他函数推导出来。
 * can be derived from other functions.
 */
// FFileManagerGeneric：文件管理器的通用基类，提供未优化的默认实现。
class FFileManagerGeneric
// 继承 IFileManager 接口。
	: public IFileManager
{
// 说明：不缓存 LowLevel，每次都取单例，保证永远是最新的。
	// instead of caching the LowLevel, we call the singleton each time to never be incorrect
// GetLowLevel：取底层平台文件接口（强制内联）。
	FORCEINLINE IPlatformFile& GetLowLevel() const
	{
// 从 FPlatformFileManager 单例取当前平台文件实现。
		return FPlatformFileManager::Get().GetPlatformFile();
	}

// 公有区。
public:

	/**
	 * 默认构造函数。
	 */
// 默认构造函数（空）。
	FFileManagerGeneric( ) { }

	/**
	 * 虚析构函数。
	 */
// 虚析构函数（空）。
	virtual ~FFileManagerGeneric( ) { }

// 公有区（第二段）。
public:

// 下面是 IFileManager 接口的实现。
	// IFileManager interface

// ProcessCommandLineOptions：解析命令行（实现在 .cpp）。
	CORE_API virtual void ProcessCommandLineOptions() override;

// SetSandboxEnabled：开关沙箱，直接转调底层。
	virtual void SetSandboxEnabled(bool bInEnabled) override
	{
// 转调底层 IPlatformFile 的同名方法。
		GetLowLevel().SetSandboxEnabled(bInEnabled);
	}

// IsSandboxEnabled：查询沙箱是否启用，直接转调底层。
	virtual bool IsSandboxEnabled() const override
	{
// 转调底层 IPlatformFile 的同名方法。
		return GetLowLevel().IsSandboxEnabled();
	}

// CreateFileReader：打开文件读，用默认读缓冲大小。
	FArchive* CreateFileReader( const TCHAR* Filename, uint32 ReadFlags=0 ) override
	{
// 转调内部实现，传入 PLATFORM_FILE_READER_BUFFER_SIZE。
		return CreateFileReaderInternal( Filename, ReadFlags, PLATFORM_FILE_READER_BUFFER_SIZE );
	}

// CreateFileWriter：打开文件写，用默认写缓冲大小。
	FArchive* CreateFileWriter( const TCHAR* Filename, uint32 WriteFlags=0 ) override
	{
// 转调内部实现，传入 PLATFORM_FILE_WRITER_BUFFER_SIZE。
		return CreateFileWriterInternal( Filename, WriteFlags, PLATFORM_FILE_WRITER_BUFFER_SIZE );
	}

// 如果允许调试文件。
#if ALLOW_DEBUG_FILES
// CreateDebugFileWriter：打开调试文件写，用调试缓冲大小。
	FArchive* CreateDebugFileWriter( const TCHAR* Filename, uint32 WriteFlags=0 ) override
	{
// 转调内部实现，传入 PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE。
		return CreateFileWriterInternal( Filename, WriteFlags, PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE );
	}
// 条件编译结束。
#endif

// Delete：删除文件（实现在 .cpp）。
	CORE_API bool Delete( const TCHAR* Filename, bool RequireExists=0, bool EvenReadOnly=0, bool Quiet=0 ) override;
// IsReadOnly：查询文件是否只读（实现在 .cpp）。
	CORE_API bool IsReadOnly( const TCHAR* Filename ) override;
// Move：移动/重命名文件（实现在 .cpp）。
	CORE_API bool Move( const TCHAR* Dest, const TCHAR* Src, bool Replace=1, bool EvenIfReadOnly=0, bool Attributes=0, bool bDoNotRetryOrError=0 ) override;
// FileExists：查询文件是否存在（实现在 .cpp）。
	CORE_API bool FileExists( const TCHAR* Filename ) override;
// DirectoryExists：查询目录是否存在（实现在 .cpp）。
	CORE_API bool DirectoryExists(const TCHAR* InDirectory) override;
// FindFiles：按通配符查找文件和/或目录（实现在 .cpp）。
	CORE_API void FindFiles( TArray<FString>& Result, const TCHAR* Filename, bool Files, bool Directories ) override;
// FindFilesRecursive：递归查找（实现在 .cpp）。
	CORE_API void FindFilesRecursive( TArray<FString>& FileNames, const TCHAR* StartDirectory, const TCHAR* Filename, bool Files, bool Directories, bool bClearFileNames=true) override;
// GetFileAgeSeconds：文件年龄（秒）（实现在 .cpp）。
	CORE_API double GetFileAgeSeconds( const TCHAR* Filename ) override;
// GetTimeStamp：取文件修改时间（实现在 .cpp）。
	CORE_API FDateTime GetTimeStamp( const TCHAR* Filename ) override;
// GetAccessTimeStamp：取文件最后访问时间（实现在 .cpp）。
	CORE_API FDateTime GetAccessTimeStamp( const TCHAR* Filename ) override;
// GetTimeStampPair：一次取两个文件的修改时间（注意：这里没有 override 关键字）。
	CORE_API void GetTimeStampPair(const TCHAR* PathA, const TCHAR* PathB, FDateTime& OutTimeStampA, FDateTime& OutTimeStampB);
// SetTimeStamp：设置文件修改时间（实现在 .cpp）。
	CORE_API bool SetTimeStamp( const TCHAR* Filename, FDateTime Timestamp ) override;
// GetFilenameOnDisk：取磁盘上真实大小写的路径（实现在 .cpp）。
	CORE_API virtual FString GetFilenameOnDisk(const TCHAR* Filename) override;

// Copy：复制文件（实现在 .cpp）。
	CORE_API virtual uint32	Copy( const TCHAR* Dest, const TCHAR* Src, bool Replace = 1, bool EvenIfReadOnly = 0, bool Attributes = 0, FCopyProgress* Progress = nullptr, EFileRead ReadFlags = FILEREAD_None, EFileWrite WriteFlags = FILEWRITE_None ) override;
// MakeDirectory：创建目录（实现在 .cpp）。
	CORE_API virtual bool	MakeDirectory( const TCHAR* Path, bool Tree=0 ) override;
// DeleteDirectory：删除目录（实现在 .cpp）。
	CORE_API virtual bool	DeleteDirectory( const TCHAR* Path, bool RequireExists=0, bool Tree=0 ) override;

// GetStatData：取文件/目录元数据（实现在 .cpp）。
	CORE_API virtual FFileStatData GetStatData(const TCHAR* FilenameOrDirectory) override;

	/**
	 * 查找指定目录下的所有文件，可选按扩展名过滤。
	 *
	 * @param Directory 要搜索的目录绝对路径，例如 "C:\UE4\Pictures"
	 *
	 * @param FileExtension 为 NULL 或空字符串时返回全部文件；
	 *			否则可以是 .EXT 或 EXT 形式，只返回该扩展名的文件。
	 *
	 * @return FoundFiles 匹配扩展名的文件；没指定扩展名时返回全部文件。
	 */
// FindFiles：按扩展名过滤查找（实现在 .cpp）。
	CORE_API virtual void FindFiles(TArray<FString>& FoundFiles, const TCHAR* Directory, const TCHAR* FileExtension = nullptr) override;

	/** 
	 * 对单层目录里的每个文件或目录调用一次访问者的 Visit 函数。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectory：遍历单层目录，虚接口访问者（实现在 .cpp）。
	CORE_API bool IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override;
// IterateDirectory：遍历单层目录，函数对象访问者（实现在 .cpp）。
	CORE_API bool IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitorFunc Visitor) override;

	/** 
	 * 对目录树里的每个文件或目录调用一次访问者的 Visit 函数。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryRecursively：递归遍历，虚接口访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override;
// IterateDirectoryRecursively：递归遍历，函数对象访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitorFunc Visitor) override;

	/** 
	 * 对单层目录里的每个文件或目录调用一次访问者的 Visit 函数。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStat：单层遍历 + stat 数据，虚接口访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override;
// IterateDirectoryStat：单层遍历 + stat 数据，函数对象访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitorFunc Visitor) override;

	/** 
	 * 对目录树里的每个文件或目录调用一次访问者的 Visit 函数。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStatRecursively：递归遍历 + stat 数据，虚接口访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override;
// IterateDirectoryStatRecursively：递归遍历 + stat 数据，函数对象访问者（实现在 .cpp）。
	CORE_API bool IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitorFunc Visitor) override;

	/**
	 * 把传入的文件名转成相对路径。
	 *
	 * @param	Filename	要转成相对路径的文件名
	 * @return	使用相对路径的文件名
	 */
// DefaultConvertToRelativePath：默认的相对路径转换实现（静态，可被复用）。
	static CORE_API FString DefaultConvertToRelativePath( const TCHAR* Filename );

	/**
	 * 把传入的文件名转成相对路径。
	 *
	 * @param	Filename	要转成相对路径的文件名
	 * @return	使用相对路径的文件名
	 */
// ConvertToRelativePath：转相对路径（实现在 .cpp）。
	CORE_API FString ConvertToRelativePath( const TCHAR* Filename ) override;

	/**
	 * 把传入的文件名转成绝对路径（用于读取）。
	 *
	 * @param	Filename	要转成绝对路径的文件名；已经是绝对路径也可以安全传入
	 * @return	使用绝对路径的文件名
	 */
// ConvertToAbsolutePathForExternalAppForRead：转绝对路径（外部程序读）（实现在 .cpp）。
	CORE_API FString ConvertToAbsolutePathForExternalAppForRead( const TCHAR* Filename ) override;

	/**
	 * 把传入的文件名转成绝对路径（用于写入）。
	 *
	 * @param	Filename	要转成绝对路径的文件名；已经是绝对路径也可以安全传入
	 * @return	使用绝对路径的文件名
	 */
// ConvertToAbsolutePathForExternalAppForWrite：转绝对路径（外部程序写）（实现在 .cpp）。
	CORE_API FString ConvertToAbsolutePathForExternalAppForWrite( const TCHAR* Filename ) override;

	/**
	 *	返回文件大小。（线程安全）
	 *
	 *	@param Filename		平台无关的 UE 文件名。
	 *	@return				文件大小（字节）；文件不存在返回 INDEX_NONE。
	 **/
// FileSize：取文件大小（字节）（实现在 .cpp）。
	CORE_API int64 FileSize( const TCHAR* Filename ) override;

	/**
	 * 给文件服务器发消息，会阻塞直到完成。如果文件管理器不支持与服务器通信，会立即返回。
	 * immediately if the file manager doesn't support talking to a server.
	 *
	 * @param Message	要发给服务器的字符串消息
	 * @return			true 表示消息已发送且服务器返回成功；false 表示没有服务器或命令失败
	 */
// SendMessageToServer：给文件服务器发消息，直接转调底层。
	virtual bool SendMessageToServer(const TCHAR* Message, IPlatformFile::IFileServerMessageHandler* Handler) override
	{
// 转调底层 IPlatformFile 的同名方法。
		return GetLowLevel().SendMessageToServer(Message, Handler);
	}

// 私有区：内部辅助函数。
private:
// CreateFileReaderInternal：创建读归档的内部实现（带缓冲大小参数）。
	CORE_API FArchive * CreateFileReaderInternal(const TCHAR* Filename, uint32 ReadFlags, uint32 BufferSize);
// CreateFileWriterInternal：创建写归档的内部实现（带缓冲大小参数）。
	CORE_API FArchive* CreateFileWriterInternal(const TCHAR* Filename, uint32 WriteFlags, uint32 BufferSize);

	/**
	 * Copy 在有进度回调时调用的辅助函数。
	 */
// CopyWithProgress：带进度回调的复制实现（Copy 内部调用）。
	CORE_API uint32	CopyWithProgress(const TCHAR* InDestFile, const TCHAR* InSrcFile, bool ReplaceExisting, bool EvenIfReadOnly, bool Attributes, FCopyProgress* Progress, EFileRead ReadFlags, EFileWrite WriteFlags);

// FindFilesRecursiveInternal：递归查找的内部实现。
	CORE_API void FindFilesRecursiveInternal( TArray<FString>& FileNames, const TCHAR* StartDirectory, const TCHAR* Filename, bool Files, bool Directories);
// 类结束。
};


// 分节横幅：FArchiveFileReaderGeneric。
/*-----------------------------------------------------------------------------
	FArchiveFileReaderGeneric
-----------------------------------------------------------------------------*/

// FArchiveFileReaderGeneric：通用文件读归档，带缓冲。
class FArchiveFileReaderGeneric : public FArchive
{
// 公有区。
public:
// 构造函数：接收文件句柄、文件名、大小、缓冲大小、读标志。
	CORE_API FArchiveFileReaderGeneric( IFileHandle* InHandle, const TCHAR* InFilename, int64 InSize, uint32 InBufferSize = PLATFORM_FILE_READER_BUFFER_SIZE, uint32 InFlags = FILEREAD_None );
// 析构函数：关闭文件句柄。
	CORE_API ~FArchiveFileReaderGeneric();

// Seek：跳转到指定位置（final 表示派生类不能再重写）。
	CORE_API virtual void Seek( int64 InPos ) override final;
// Tell：返回当前位置（内联）。
	virtual int64 Tell() override final
	{
// 直接返回 Pos 成员。
		return Pos;
	}
// TotalSize：返回文件总大小（内联）。
	virtual int64 TotalSize() override final
	{
// 直接返回 Size 成员。
		return Size;
	}
// Close：关闭归档。
	CORE_API virtual bool Close() override final;
// Serialize：读写数据（读归档里就是读）。
	CORE_API virtual void Serialize( void* V, int64 Length ) override final;
// GetArchiveName：返回归档名（内联）。
	virtual FString GetArchiveName() const override
	{
// 返回文件名。
		return Filename;
	}
// FlushCache：清空缓冲。
	CORE_API virtual void FlushCache() override final;

// Precache：预读一段数据到缓冲。
	CORE_API virtual bool Precache(int64 PrecacheOffset, int64 PrecacheSize) override;

// 受保护区。
protected:
// InternalPrecache：预读的内部实现。
	CORE_API bool InternalPrecache( int64 PrecacheOffset, int64 PrecacheSize );
	/** 
	 * 平台专属的跳转
	 * @param InPos - 从文件开头算起的偏移量
	 * @return 失败返回 false
	**/
// SeekLowLevel：平台专属的跳转（派生类可重写）。
	CORE_API virtual bool SeekLowLevel(int64 InPos);
	/** 关闭文件句柄 **/
// CloseLowLevel：关闭底层文件句柄。
	CORE_API virtual void CloseLowLevel();
	/** 
	 * 平台专属的读取
	 * @param Dest - 要填充的缓冲区
	 * @param CountToRead - 要读的字节数
	 * @param OutBytesRead - 实际读到的字节数
	**/
// ReadLowLevel：平台专属的读取（派生类可重写）。
	CORE_API virtual void ReadLowLevel(uint8* Dest, int64 CountToRead, int64& OutBytesRead);

	/** 出错时是否抑制日志输出 */
// IsSilent：是否静默（出错不打印日志）（内联）。
	bool IsSilent() const
	{
// 检查 Flags 里有没有 FILEREAD_Silent 位。
		return !!(Flags & FILEREAD_Silent);
	}

	/** 文件名（调试用）。 */
// Filename：文件名（调试用）。
	FString Filename;
// Size：文件总大小。
	int64 Size;
// Pos：当前读写位置。
	int64 Pos;
// BufferBase：缓冲区起始位置。
	int64 BufferBase;
// Handle：底层文件句柄（独占指针，自动关闭）。
	TUniquePtr<IFileHandle> Handle;
	/**
	 * BufferWindow 与底层位置的约定：如果有 BufferWindow 且 Pos 在窗口内，则底层位置在 BufferWindow 末尾；
	 * 如果没有 BufferWindow，或 Pos 在窗口外，则底层位置就在 Pos。
	 */
// BufferArray：缓冲区（64 位长度版本）。
	TArray64<uint8> BufferArray;
// BufferSize：缓冲区大小。
	int64 BufferSize;
// Flags：读标志位。
	uint32 Flags;
// bFirstReadAfterSeek：Seek 之后是否还没读过。
	bool bFirstReadAfterSeek;

// 匿名枚举：内部常量。
	enum
	{
// bPrecacheAsSoonAsPossible：设为 true 时更可能不用等数据，但读请求会更多。
		bPrecacheAsSoonAsPossible = 0 // Setting this to true makes it more likely bytes will be available without waiting due to external Precache calls, but at the cost of a larger number of read requests.
	};

// 友元：测试类可访问私有成员。
	friend class FArchiveFileReaderGenericTest;
// 类结束。
};


// 分节横幅：FArchiveFileWriterGeneric。
/*-----------------------------------------------------------------------------
	FArchiveFileWriterGeneric
-----------------------------------------------------------------------------*/

// FArchiveFileWriterGeneric：通用文件写归档，带缓冲。
class FArchiveFileWriterGeneric : public FArchive
{
// 公有区。
public:
// 构造函数：接收文件句柄、文件名、起始位置、缓冲大小、写标志。
	CORE_API FArchiveFileWriterGeneric( IFileHandle* InHandle, const TCHAR* InFilename, int64 InPos, uint32 InBufferSize = PLATFORM_FILE_WRITER_BUFFER_SIZE, uint32 InFlags = FILEWRITE_None);
// 析构函数：关闭文件句柄。
	CORE_API ~FArchiveFileWriterGeneric();

// Seek：跳转到指定位置（final）。
	CORE_API virtual void Seek( int64 InPos ) override final;
// Tell：返回当前位置（内联）。
	virtual int64 Tell() override final
	{
// 直接返回 Pos 成员。
		return Pos;
	}
// TotalSize：返回文件总大小。
	CORE_API virtual int64 TotalSize() override;
// Close：关闭归档。
	CORE_API virtual bool Close() override final;
// Serialize：读写数据（写归档里就是写）。
	CORE_API virtual void Serialize( void* V, int64 Length ) override final;
// Flush：把缓冲刷到文件。
	CORE_API virtual void Flush() override final;
// GetArchiveName：返回归档名（内联）。
	virtual FString GetArchiveName() const override
	{
// 返回文件名。
		return Filename;
	}

// 受保护区。
protected:
	/**
	 * 把内部缓冲写到文件句柄
	 * @note 不刷新句柄本身，所以数据可能还在 OS 缓存里，尚未真正落盘！
	 * @return 有缓冲数据且写入成功返回 true；没有数据可刷或写入失败返回 false
	 */
// FlushBuffer：把内部缓冲写到文件句柄。
	CORE_API bool FlushBuffer();
	/** 
	 * 平台专属的跳转
	 * @param InPos - 从文件开头算起的偏移量
	 * @return 失败返回 false
	**/
// SeekLowLevel：平台专属的跳转。
	CORE_API virtual bool SeekLowLevel(int64 InPos);
	/** 
	 * 关闭文件句柄
	 * @return 失败返回 false
	**/
// CloseLowLevel：关闭底层文件句柄。
	CORE_API virtual bool CloseLowLevel();
	/** 
	 * 平台专属的写入
	 * @param Src - 要写出的缓冲区
	 * @param CountToWrite - 要写的字节数
	 * @return 失败返回 false
	**/
// WriteLowLevel：平台专属的写入。
	CORE_API virtual bool WriteLowLevel(const uint8* Src, int64 CountToWrite);

// 标记 LogWriteError 已废弃（5.6）：请传入 FPlatformMisc::GetLastError() 的结果。
	UE_DEPRECATED(5.6, "Pass in LastError from FPlatformMisc::GetLastError")
// LogWriteError：记录写错误（旧版，已废弃）。
	CORE_API void LogWriteError(const TCHAR* Message);
	/**
	 * 记录 IO 错误。
	 * 注意：错误发生后、调用本函数之前，不要调用任何平台 API，
	 * 因为系统错误码可能被重置，导致本消息里记录的错误码不正确。
	 * logged in this message.
	 * @param Message 出了什么问题的简要描述
	 */
// LogWriteError：记录写错误（新版，带系统错误码）。
	CORE_API void LogWriteError(const TCHAR* Message, uint32 LastError);

	/** 出错时是否抑制日志输出 */
// IsSilent：是否静默（内联）。
	bool IsSilent() const
	{
// 检查 Flags 里有没有 FILEWRITE_Silent 位。
		return !!(Flags & FILEWRITE_Silent);
	}

	/** 文件名（调试用） */
// Filename：文件名（调试用）。
	FString Filename;
// Flags：写标志位。
	uint32 Flags;
// Pos：当前写入位置。
	int64 Pos;
// Handle：底层文件句柄（独占指针）。
	TUniquePtr<IFileHandle> Handle;
// BufferArray：缓冲区。
	TArray64<uint8> BufferArray;
// BufferSize：缓冲区大小。
	int64 BufferSize;
// bLoggingError：是否正在记录错误（防止递归）。
	bool bLoggingError;
// 类结束。
};
