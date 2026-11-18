// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint8 / uint32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入 FDateTime：文件时间戳类型。
#include "Misc/DateTime.h"
// 引入通用平台文件接口：IPlatformFile / FFileStatData / FDirectoryVisitor 都来自这里。
#include "GenericPlatform/GenericPlatformFile.h"

// 文件名最大长度。目前没有限制；如果将来需要，大概会用缩短规则。
// MAX_UNREAL_FILENAME_LENGTH_DEPRECATED：文件名最大长度（已废弃，改用 PLATFORM_MAX_FILEPATH_LENGTH_DEPRECATED）。
#define MAX_UNREAL_FILENAME_LENGTH_DEPRECATED (PLATFORM_MAX_FILEPATH_LENGTH_DEPRECATED)


// EFileWrite：写文件的标志位（可组合）。
enum EFileWrite
{
// 0x00：无标志。
	FILEWRITE_None				= 0x00,
// 0x01：失败时不断言（默认失败会触发断言）。
	FILEWRITE_NoFail            = 0x01,
// 0x02：不覆盖已存在的文件。
	FILEWRITE_NoReplaceExisting = 0x02,
// 0x04：即使文件只读也写。
	FILEWRITE_EvenIfReadOnly    = 0x04,
// 0x08：追加模式（写到文件末尾）。
	FILEWRITE_Append			= 0x08,
// 0x10：允许同时读。
	FILEWRITE_AllowRead			= 0x10,
// 0x20：静默模式（失败不打印日志）。
	FILEWRITE_Silent			= 0x20
};


// EFileRead：读文件的标志位（可组合）。
enum EFileRead
{
// 0x00：无标志。
	FILEREAD_None				= 0x00,
// 0x01：失败时不断言。
	FILEREAD_NoFail             = 0x01,
// 0x02：静默模式（失败不打印日志）。
	FILEREAD_Silent				= 0x02,
// 0x04：允许同时写。
	FILEREAD_AllowWrite			= 0x04,
};


// ECopyResult：文件复制的结果码。
enum ECopyResult
{
// 0x00：复制成功。
	COPY_OK						= 0x00,
// 0x01：复制失败。
	COPY_Fail					= 0x01,
// 0x02：复制被取消。
	COPY_Canceled				= 0x02,
};


// FCopyProgress：复制进度回调接口。
struct FCopyProgress
{
// Poll：每复制一段就回调一次，Fraction 是 0~1 的进度；返回 false 表示取消复制。
	virtual bool Poll( float Fraction )=0;
};


// EFileOpenFlags：打开文件的标志位（可组合）。
enum EFileOpenFlags
{
// 0x01：以读方式打开。
	IO_READ			= 0x01,					// Open for reading
// 0x02：以写方式打开。
	IO_WRITE		= 0x02,					// Open for writing
// 0x40：写时保留已有数据，文件指针移到末尾。
	IO_APPEND		= 0x40,					// When writing, keep the existing data, set the filepointer to the end of the existing data
};

// IFileManager：文件管理器接口，所有文件操作的统一入口。
class IFileManager
{
// 受保护区。
protected:

	/** 构造函数。 */
// 构造函数：protected，只能由派生类（平台实现）调用。
	IFileManager() {}

// 公有区。
public:

	/** 取单例，平台专属实现，同时会调用 PreInit()。 */
// Get：取全局唯一的文件管理器实例。
	static CORE_API IFileManager& Get();

	/** 允许文件管理器处理命令行参数。 */
// ProcessCommandLineOptions：解析命令行里的文件相关参数。
	virtual void ProcessCommandLineOptions() = 0;

	/** 启用/禁用沙箱（如果平台用了沙箱）。 */
// SetSandboxEnabled：开关沙箱模式。
	virtual void SetSandboxEnabled(bool bInEnabled) = 0;
	/** 返回沙箱是否启用。 */
// IsSandboxEnabled：查询沙箱是否启用。
	virtual bool IsSandboxEnabled() const = 0;

	/**
	 * 打开文件读，并创建一个 FArchive 用来从里面读数据。
	 *
	 * @param Filename	要打开的文件路径
	 * @param ReadFlags	可选标志位，取值见 EFileRead
	 *
	 * @return 成功返回 FArchive 指针，失败返回 nullptr。
	 * 注意：用完要由调用方 delete 这个 FArchive。
	 */
// CreateFileReader：打开文件读，返回 FArchive（用完要调用方 delete）。
	virtual FArchive* CreateFileReader( const TCHAR* Filename, uint32 ReadFlags=0 )=0;

	/**
	 * Opens a file for writing and create an FArchive which can be used to write to it.
	 * 打开文件写，并创建一个 FArchive 用来往里面写数据。
	 * @param Filename		目标文件路径
	 * @param WriteFlags	可选标志位，取值见 EFileWrite
	 *
	 * @return 成功返回 FArchive 指针，失败返回 nullptr。
	 * 注意：用完要由调用方 delete 这个 FArchive。
	 */
// CreateFileWriter：打开文件写，返回 FArchive（用完要调用方 delete）。
	virtual FArchive* CreateFileWriter( const TCHAR* Filename, uint32 WriteFlags=0 )=0;

	// 如果要写调试文件，应该用 CreateDebugFileWriter，并把调用代码包在 #if ALLOW_DEBUG_FILES 里。
// 如果允许调试文件（ALLOW_DEBUG_FILES）。
#if ALLOW_DEBUG_FILES
// CreateDebugFileWriter：打开调试文件写（只在允许调试文件时存在）。
	virtual FArchive* CreateDebugFileWriter(const TCHAR* Filename, uint32 WriteFlags=0 ) = 0;
// 条件编译结束。
#endif

	/** 检查文件是否只读。 */
// IsReadOnly：查询文件是否只读。
	virtual bool IsReadOnly( const TCHAR* Filename )=0;

	/** 删除文件。 */
// Delete：删除文件（RequireExists 要求文件必须存在，EvenReadOnly 即使只读也删，Quiet 静默）。
	virtual bool Delete( const TCHAR* Filename, bool RequireExists=0, bool EvenReadOnly=0, bool Quiet=0 )=0;

	/** 复制文件。 */
// Copy：复制文件（Replace 覆盖，EvenIfReadOnly 只读也复制，Attributes 复制属性，Progress 进度回调）。
	virtual uint32 Copy( const TCHAR* Dest, const TCHAR* Src, bool Replace=1, bool EvenIfReadOnly=0, bool Attributes=0, FCopyProgress* Progress = nullptr, EFileRead ReadFlags=FILEREAD_None, EFileWrite WriteFlags=FILEWRITE_None)=0; // utility

	/** 移动/重命名文件。 */
// Move：移动或重命名文件。
	virtual bool Move( const TCHAR* Dest, const TCHAR* Src, bool Replace=1, bool EvenIfReadOnly=0, bool Attributes=0, bool bDoNotRetryOrError=0 )=0;

	/** 检查文件是否存在。 */
// FileExists：查询文件是否存在。
	virtual bool FileExists( const TCHAR* Filename )=0;

	/** 检查目录是否存在。 */
// DirectoryExists：查询目录是否存在。
	virtual bool DirectoryExists( const TCHAR* InDirectory )=0;

	/** 创建目录。 */
// MakeDirectory：创建目录（Tree 为 true 时递归创建父目录）。
	virtual bool MakeDirectory( const TCHAR* Path, bool Tree=0 )=0;

	/** 删除目录。 */
// DeleteDirectory：删除目录（Tree 为 true 时递归删除）。
	virtual bool DeleteDirectory( const TCHAR* Path, bool RequireExists=0, bool Tree=0 )=0;

	/** 返回文件或目录的 stat 数据。使用前先检查 FFileStatData::bIsValid。 */
// GetStatData：取文件/目录的元数据（大小、时间、属性）。
	virtual FFileStatData GetStatData(const TCHAR* FilenameOrDirectory) = 0;

	/** 查找文件或目录。 */
// FindFiles：按通配符查找文件和/或目录。
	virtual void FindFiles( TArray<FString>& FileNames, const TCHAR* Filename, bool Files, bool Directories)=0;

	/**
	 * 查找指定目录下的所有文件，可选按扩展名过滤。
	 *
	 * @param[out] FoundFiles 匹配扩展名的文件；没指定扩展名时返回全部文件。
	 *
	 * @param[in] Directory 要搜索的目录绝对路径，例如 "C:\UE4\Pictures"
	 *
	 * @param[in] FileExtension 为 NULL 或空字符串时返回全部文件；
	 *			否则可以是 .EXT 或 EXT 形式，只返回该扩展名的文件。
	 */
// FindFiles：按扩展名过滤查找目录下的文件。
	virtual void FindFiles(TArray<FString>& FoundFiles, const TCHAR* Directory, const TCHAR* FileExtension = nullptr) = 0;

	/** 递归查找文件或目录。 */
// FindFilesRecursive：递归查找（bClearFileNames 为 true 时先清空结果数组）。
	virtual void FindFilesRecursive( TArray<FString>& FileNames, const TCHAR* StartDirectory, const TCHAR* Filename, bool Files, bool Directories, bool bClearFileNames=true) = 0; // utility

	/** 
	 * 对单层目录里的每个文件或目录调用一次访问者的 Visit 函数。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectory：遍历单层目录（不递归），用虚接口访问者。
	virtual bool IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) = 0;

	/**
	 * 对单层目录里的每个文件或目录调用一次访问者。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者（签名见 FDirectoryVisitor::Visit）。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectory：遍历单层目录，用函数对象访问者。
	virtual bool IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitorFunc Visitor) = 0;

	/** 
	 * 对目录树里的每个文件或目录调用一次访问者的 Visit 函数。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryRecursively：递归遍历目录树，用虚接口访问者。
	virtual bool IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) = 0;

	/**
	 * 对目录树里的每个文件或目录调用一次访问者。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者（签名见 FDirectoryVisitor::Visit）。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryRecursively：递归遍历目录树，用函数对象访问者。
	virtual bool IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitorFunc Visitor) = 0;

	/** 
	 * 对单层目录里的每个文件或目录调用一次访问者的 Visit 函数。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStat：遍历单层目录并带 stat 数据，用虚接口访问者。
	virtual bool IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) = 0;

	/**
	 * 对单层目录里的每个文件或目录调用一次访问者。本函数不遍历子目录。
	 * @param Directory		要遍历的目录。
	 * @param Visitor		对目录里每个元素调用的访问者（签名见 FDirectoryStatVisitor::Visit）。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStat：遍历单层目录并带 stat 数据，用函数对象访问者。
	virtual bool IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitorFunc Visitor) = 0;

	/** 
	 * 对目录树里的每个文件或目录调用一次访问者的 Visit 函数。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStatRecursively：递归遍历并带 stat 数据，用虚接口访问者。
	virtual bool IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) = 0;

	/**
	 * 对目录树里的每个文件或目录调用一次访问者。本函数会遍历子目录。
	 * @param Directory		要递归遍历的目录。
	 * @param Visitor		对目录及所有子目录里每个元素调用的访问者（签名见 FDirectoryStatVisitor::Visit）。
	 * @return				目录不存在或访问者返回 false 时返回 false。
	**/
// IterateDirectoryStatRecursively：递归遍历并带 stat 数据，用函数对象访问者。
	virtual bool IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitorFunc Visitor) = 0;

	/** 取文件年龄（秒）。 */
// GetFileAgeSeconds：文件从修改到现在过了多少秒。
	virtual double GetFileAgeSeconds( const TCHAR* Filename )=0;

	/** 
	 * @return 给定文件的修改时间（失败返回 FDateTime::MinValue()）
	 */
// GetTimeStamp：取文件修改时间（失败返回 FDateTime::MinValue()）。
	virtual FDateTime GetTimeStamp( const TCHAR* Path ) = 0;

	/**
	 * @param[in] PathA 第一个文件
	 * @param[in] PathB 第二个文件
	 * @param[out] OutTimeStampA 第一个文件的修改时间（失败返回 FDateTime::MinValue()）
	 * @param[out] OutTimeStampB 第二个文件的修改时间（失败返回 FDateTime::MinValue()）
	*/
// GetTimeStampPair：一次取两个文件的修改时间（省一次系统调用）。
	virtual void GetTimeStampPair(const TCHAR* PathA, const TCHAR* PathB, FDateTime& OutTimeStampA, FDateTime& OutTimeStampB) = 0;

	/** 
	 * 设置给定文件的修改时间
	 */
// SetTimeStamp：设置文件修改时间。
	virtual bool SetTimeStamp( const TCHAR* Path, FDateTime TimeStamp ) = 0;

	/** 
	 * @return 给定文件的最后访问时间（失败返回 FDateTime::MinValue()）
	 */
// GetAccessTimeStamp：取文件最后访问时间。
	virtual FDateTime GetAccessTimeStamp( const TCHAR* Filename ) = 0;

	/**
	 * 把传入的文件名转成相对路径。
	 *
	 * @param	Filename	要转成相对路径的文件名
	 * 
	 * @return	使用相对路径的文件名
	 */
// ConvertToRelativePath：把文件名转成相对路径。
	virtual FString ConvertToRelativePath( const TCHAR* Filename ) = 0;

	/**
	 * 把传入的文件名转成绝对路径（用于读取）。
	 *
	 * @param	Filename	要转成绝对路径的文件名；已经是绝对路径也可以安全传入
	 * 
	 * @return	使用绝对路径的文件名
	 */
// ConvertToAbsolutePathForExternalAppForRead：转成绝对路径（给外部程序读用）。
	virtual FString ConvertToAbsolutePathForExternalAppForRead( const TCHAR* Filename ) = 0;

	/**
	 * 把传入的文件名转成绝对路径（用于写入）。
	 *
	 * @param	Filename	要转成绝对路径的文件名；已经是绝对路径也可以安全传入
	 * 
	 * @return	使用绝对路径的文件名
	 */
// ConvertToAbsolutePathForExternalAppForWrite：转成绝对路径（给外部程序写用）。
	virtual FString ConvertToAbsolutePathForExternalAppForWrite( const TCHAR* Filename ) = 0;

	/**
	 *	返回文件大小。（线程安全）
	 *
	 *	@param Filename		平台无关的 UE 文件名。
	 *	@return				文件大小（字节）；文件不存在返回 INDEX_NONE。
	 **/
// FileSize：取文件大小（字节），文件不存在返回 INDEX_NONE。
	virtual int64 FileSize( const TCHAR* Filename )=0;

	/**
	 * 给文件服务器发消息，会阻塞直到完成。如果文件管理器不支持与服务器通信，会立即返回。
	 * immediately if the file manager doesn't support talking to a server.
	 *
	 * @param Message	要发给服务器的字符串消息
	 *
	 * @return			true 表示消息已发送且服务器返回成功；false 表示没有服务器或命令失败
	 */
// SendMessageToServer：给文件服务器发消息（阻塞直到完成）。
	virtual bool SendMessageToServer(const TCHAR* Message, IPlatformFile::IFileServerMessageHandler* Handler)=0;

	/**
	* 对大小写不敏感的文件系统，返回与文件系统里大小写一致的文件完整路径。
	*
	* @param Filename	要查询的文件名
	*
	* @return	与文件系统里大小写一致的文件名。
	*/
// GetFilenameOnDisk：在大小写不敏感的文件系统上，返回与磁盘一致的大小写路径。
	virtual FString GetFilenameOnDisk(const TCHAR* Filename) = 0;
// 类结束。
};
