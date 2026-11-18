# 22 — `HAL/FileManagerGeneric.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`（**374 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`
> **⚠️ 本节不能独立编译**——依赖 `IFileManager` / `IPlatformFile` / `FArchive`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

上一节 `FileManager.h` 定义了 `IFileManager` 接口（40+ 个纯虚函数）。如果每个平台都从零实现这 40+ 个函数，工作量巨大且容易出错。

`FileManagerGeneric.h` 就是**通用基类**：它把「能用其他函数推导出来的」都实现了（比如 `FileExists` 可以用 `GetStatData` 推导），平台只需要重写真正需要特化的部分。

**它包含 3 个类**：

| 类 | 作用 |
|---|---|
| `FFileManagerGeneric` | 文件管理器通用基类（实现 `IFileManager` 的大部分方法） |
| `FArchiveFileReaderGeneric` | 通用文件**读**归档（带缓冲） |
| `FArchiveFileWriterGeneric` | 通用文件**写**归档（带缓冲） |

**说个场景**：你调 `IFileManager::Get().CreateFileReader("D:/Data.bin")`，实际返回的就是 `FArchiveFileReaderGeneric`。它内部维护一个 1024 字节的缓冲区，`Serialize()` 时先看数据在不在缓冲里，不在才真正读磁盘 —— 这就是「带缓冲的读」。

**三个缓冲大小宏**（第 16~26 行）：

| 宏 | 默认值 | 用途 |
|---|---|---|
| `PLATFORM_FILE_READER_BUFFER_SIZE` | 1024 | 读缓冲 |
| `PLATFORM_FILE_WRITER_BUFFER_SIZE` | 4096 | 写缓冲 |
| `PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE` | 4096 | 调试文件写缓冲 |

> 🔑 **一句话**：`FileManagerGeneric.h` = **文件管理器的「通用底座」**，把能推导的都实现了，平台只补特化部分。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 10 个 include（第 1~14 行）

```cpp
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
```

**本段新元素**

- `Containers/Array.h` —— `TArray` / `TArray64`
- `Containers/ContainersFwd.h` —— 容器前置声明
- `Containers/UnrealString.h` —— `FString`
- `CoreTypes.h` —— `uint8` / `uint32` / `int64` / `TCHAR`
- `GenericPlatform/GenericPlatformFile.h` —— `IPlatformFile` / `IFileHandle` / `FFileStatData`
- `HAL/FileManager.h` —— **基类 `IFileManager`**
- `HAL/PlatformFileManager.h` —— `FPlatformFileManager`
- `Misc/DateTime.h` —— `FDateTime`
- `Serialization/Archive.h` —— **基类 `FArchive`**
- `Templates/UniquePtr.h` —— `TUniquePtr<IFileHandle>`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~14 行 | 10 个 include |

> 类比：像开工前领十样工具：容器、字符串、类型、平台文件、基类、单例、时间、归档、智能指针。

### 二、3 个缓冲大小宏（第 16~26 行）

```cpp
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
```

**本段新元素**

- `#ifndef X` / `#define X 值` / `#endif` —— 「平台没定义就给默认值」
- `PLATFORM_FILE_READER_BUFFER_SIZE` —— 读缓冲，默认 1024
- `PLATFORM_FILE_WRITER_BUFFER_SIZE` —— 写缓冲，默认 4096
- `PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE` —— 调试写缓冲，默认 4096

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 16~18 行 | 读缓冲宏 |
| 第 20~22 行 | 写缓冲宏 |
| 第 24~26 行 | 调试写缓冲宏 |

> 类比：像给水桶定容量：读用小桶（1KB），写用大桶（4KB），平台可以自己换。

### 三、`FFileManagerGeneric` 类声明 + `GetLowLevel`（第 28~44 行）

```cpp
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
```

**本段新元素**

- `class FFileManagerGeneric : public IFileManager` —— 继承接口
- `FORCEINLINE IPlatformFile& GetLowLevel() const` —— 取底层平台文件接口
- `FPlatformFileManager::Get().GetPlatformFile()` —— 从单例取
- **不缓存 LowLevel** —— 每次取单例，保证永远最新

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 28~34 行 | 类说明注释 |
| 第 35~36 行 | 类声明 + 继承 |
| 第 38~42 行 | `GetLowLevel` 内联实现 |
| 第 44 行 | 公有区 |

> 类比：像不存电话号码，每次现查：虽然多一步，但永远不会打错。

### 四、构造 / 析构（第 46~54 行）

```cpp
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
```

**本段新元素**

- `FFileManagerGeneric() { }` —— 默认构造（空）
- `virtual ~FFileManagerGeneric() { }` —— 虚析构（空）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 46~49 行 | 默认构造 |
| 第 51~54 行 | 虚析构 |

> 类比：像开关机：开机什么都不用做，关机也不用清理。

### 五、命令行 + 沙箱（3 个）（第 56~70 行）

```cpp
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
```

**本段新元素**

- `ProcessCommandLineOptions()` —— 解析命令行（实现在 .cpp）
- `SetSandboxEnabled(bool)` —— 内联转调 `GetLowLevel().SetSandboxEnabled()`
- `IsSandboxEnabled()` —— 内联转调 `GetLowLevel().IsSandboxEnabled()`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 56~60 行 | `ProcessCommandLineOptions` |
| 第 62~65 行 | `SetSandboxEnabled` |
| 第 67~70 行 | `IsSandboxEnabled` |

> 类比：像前台转接电话：沙箱相关的直接转给底层平台。

### 六、打开文件三件套（内联转调）（第 72~87 行）

```cpp
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
```

**本段新元素**

- `CreateFileReader(Filename, ReadFlags)` —— 转调 `CreateFileReaderInternal(..., PLATFORM_FILE_READER_BUFFER_SIZE)`
- `CreateFileWriter(Filename, WriteFlags)` —— 转调 `CreateFileWriterInternal(..., PLATFORM_FILE_WRITER_BUFFER_SIZE)`
- `CreateDebugFileWriter(...)` —— 转调 `CreateFileWriterInternal(..., PLATFORM_DEBUG_FILE_WRITER_BUFFER_SIZE)`
- **三个都只是「补上缓冲大小参数」再转调**

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 72~75 行 | `CreateFileReader` |
| 第 77~80 行 | `CreateFileWriter` |
| 第 82~87 行 | `CreateDebugFileWriter`（条件编译） |

> 类比：像点餐时自动加默认配菜：你只说主食，它帮你补上缓冲大小。

### 七、13 个 CORE_API 声明（实现在 .cpp）（第 89~107 行）

```cpp
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
```

**本段新元素**

- `CORE_API` —— 跨模块导出（实现在 .cpp 里）
- `Delete` / `IsReadOnly` / `Move` / `FileExists` / `DirectoryExists` —— 5 个基础操作
- `FindFiles` / `FindFilesRecursive` —— 2 个查找
- `GetFileAgeSeconds` / `GetTimeStamp` / `GetAccessTimeStamp` / `GetTimeStampPair` / `SetTimeStamp` —— 5 个时间
- `GetFilenameOnDisk` —— 磁盘大小写路径
- `Copy` / `MakeDirectory` / `DeleteDirectory` —— 3 个增删
- `GetStatData` —— 元数据
- **注意**：`GetTimeStampPair`（第 99 行）**没有 `override`** —— 引擎原文如此

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 89~93 行 | 5 个基础操作 |
| 第 94~95 行 | 2 个查找 |
| 第 96~100 行 | 5 个时间 |
| 第 101 行 | `GetFilenameOnDisk` |
| 第 103~105 行 | 3 个增删 |
| 第 107 行 | `GetStatData` |

> 类比：像服务台的 13 个窗口：都开着，但具体办事的人在后屋（.cpp）。

### 八、`FindFiles` 扩展名版（第 109~119 行）

```cpp
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
```

**本段新元素**

- `FindFiles(FoundFiles, Directory, FileExtension)` —— 按扩展名过滤
- `FileExtension = nullptr` —— 默认不过滤（返回全部）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 109~118 行 | 函数说明注释 |
| 第 119 行 | 函数声明 |

> 类比：像筛子：不装筛网就全过，装了只留指定大小。

### 九、8 个 `IterateDirectory`（第 121~155 行）

```cpp
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
```

**本段新元素**

- `IterateDirectory` ×2 —— 单层（虚接口 / 函数对象）
- `IterateDirectoryRecursively` ×2 —— 递归（虚接口 / 函数对象）
- `IterateDirectoryStat` ×2 —— 单层 + stat（虚接口 / 函数对象）
- `IterateDirectoryStatRecursively` ×2 —— 递归 + stat（虚接口 / 函数对象）
- `IPlatformFile::FDirectoryVisitor&` —— 虚接口访问者
- `IPlatformFile::FDirectoryVisitorFunc` —— 函数对象访问者（按值传）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 121~128 行 | 单层 ×2 |
| 第 130~137 行 | 递归 ×2 |
| 第 139~146 行 | 单层 stat ×2 |
| 第 148~155 行 | 递归 stat ×2 |

> 类比：像查户口 8 种查法：单层/递归 × 虚接口/函数对象 × 带不带 stat。

### 十、路径转换 + `FileSize`（第 157~195 行）

```cpp
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
```

**本段新元素**

- `static DefaultConvertToRelativePath(Filename)` —— 静态默认实现（可复用）
- `ConvertToRelativePath(Filename)` —— 转相对路径
- `ConvertToAbsolutePathForExternalAppForRead(Filename)` —— 转绝对路径（外部读）
- `ConvertToAbsolutePathForExternalAppForWrite(Filename)` —— 转绝对路径（外部写）
- `FileSize(Filename)` —— 文件大小（字节），不存在返回 `INDEX_NONE`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 157~163 行 | `DefaultConvertToRelativePath`（静态） |
| 第 165~171 行 | `ConvertToRelativePath` |
| 第 173~179 行 | `ConvertToAbsolutePathForExternalAppForRead` |
| 第 181~187 行 | `ConvertToAbsolutePathForExternalAppForWrite` |
| 第 189~195 行 | `FileSize` |

> 类比：像给外人指路：自己人用相对地址，外人必须给完整地址，而且读写地址可能不同。

### 十一、`SendMessageToServer`（内联转调）（第 197~207 行）

```cpp
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
```

**本段新元素**

- `SendMessageToServer(Message, Handler)` —— 给文件服务器发消息
- 内联转调 `GetLowLevel().SendMessageToServer(...)`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 197~203 行 | 函数说明注释 |
| 第 204~207 行 | 内联实现 |

> 类比：像转接电话：直接转给底层平台。

### 十二、私有区：4 个内部辅助函数（第 209~219 行）

```cpp
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
```

**本段新元素**

- `CreateFileReaderInternal(Filename, ReadFlags, BufferSize)` —— 读归档内部实现
- `CreateFileWriterInternal(Filename, WriteFlags, BufferSize)` —— 写归档内部实现
- `CopyWithProgress(...)` —— 带进度回调的复制
- `FindFilesRecursiveInternal(...)` —— 递归查找内部实现

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 209 行 | 私有区开始 |
| 第 210~211 行 | 2 个 Create 内部实现 |
| 第 213~216 行 | `CopyWithProgress` |
| 第 218 行 | `FindFilesRecursiveInternal` |
| 第 219 行 | 类结束 |

> 类比：像后厨的四个灶台：客人看不到，但菜都从这里出。

### 十三、`FArchiveFileReaderGeneric` 公有接口（第 222~249 行）

```cpp
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
```

**本段新元素**

- `class FArchiveFileReaderGeneric : public FArchive` —— 继承归档基类
- `FArchiveFileReaderGeneric(IFileHandle*, Filename, Size, BufferSize, Flags)` —— 构造
- `Seek` / `Tell` / `TotalSize` / `Close` / `Serialize` / `FlushCache` —— 6 个 `override final`
- `final` —— 派生类不能再重写
- `GetArchiveName()` —— 内联返回 `Filename`
- `Precache(Offset, Size)` —— 预读（没有 `final`，派生类可重写）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 222~226 行 | 分节横幅 + 类声明 |
| 第 229~230 行 | 构造 / 析构 |
| 第 232~242 行 | 6 个 `override final` |
| 第 243~246 行 | `GetArchiveName` 内联 |
| 第 247~249 行 | `FlushCache` / `Precache` |

> 类比：像读书的三个动作：翻到某页（Seek）、看现在第几页（Tell）、读内容（Serialize）。

### 十四、`FArchiveFileReaderGeneric` protected + 成员（第 251~296 行）

```cpp
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
```

**本段新元素**

- `InternalPrecache(Offset, Size)` —— 预读内部实现
- `SeekLowLevel(InPos)` —— 平台专属跳转（虚，派生类可重写）
- `CloseLowLevel()` —— 关闭底层句柄
- `ReadLowLevel(Dest, CountToRead, OutBytesRead)` —— 平台专属读取
- `IsSilent()` —— 检查 `Flags & FILEREAD_Silent`
- `FString Filename` / `int64 Size` / `int64 Pos` / `int64 BufferBase` —— 4 个基础成员
- `TUniquePtr<IFileHandle> Handle` —— 独占指针（自动关闭）
- `TArray64<uint8> BufferArray` —— 缓冲区（64 位长度）
- `int64 BufferSize` / `uint32 Flags` / `bool bFirstReadAfterSeek` —— 3 个状态成员
- `enum { bPrecacheAsSoonAsPossible = 0 }` —— 匿名枚举常量
- `friend class FArchiveFileReaderGenericTest` —— 友元测试类

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 251~252 行 | protected + `InternalPrecache` |
| 第 253~258 行 | `SeekLowLevel` |
| 第 259~260 行 | `CloseLowLevel` |
| 第 261~267 行 | `ReadLowLevel` |
| 第 269~273 行 | `IsSilent` |
| 第 275~280 行 | 5 个成员 |
| 第 281~288 行 | 缓冲相关 4 个成员 |
| 第 290~295 行 | 匿名枚举 + 友元 |
| 第 296 行 | 类结束 |

> 类比：像读书人的书签和笔记：读到哪（Pos）、缓冲到哪（BufferBase）、书本身（Handle）。

### 十五、`FArchiveFileWriterGeneric` 公有接口（第 299~321 行）

```cpp
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
```

**本段新元素**

- `class FArchiveFileWriterGeneric : public FArchive` —— 继承归档基类
- `FArchiveFileWriterGeneric(IFileHandle*, Filename, InPos, BufferSize, Flags)` —— 构造
- `Seek` / `Tell` / `Close` / `Serialize` / `Flush` —— 5 个 `override final`
- `TotalSize()` —— 没有 `final`（派生类可重写）
- `GetArchiveName()` —— 内联返回 `Filename`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 299~303 行 | 分节横幅 + 类声明 |
| 第 306~307 行 | 构造 / 析构 |
| 第 309~317 行 | 5 个 `override final` + `TotalSize` |
| 第 318~321 行 | `GetArchiveName` 内联 |

> 类比：像写书的三个动作：翻到某页（Seek）、写内容（Serialize）、保存（Flush）。

### 十六、`FArchiveFileWriterGeneric` protected + 成员（第 323~374 行）

```cpp
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
```

**本段新元素**

- `FlushBuffer()` —— 把内部缓冲写到句柄（**不刷新句柄本身**）
- `SeekLowLevel(InPos)` —— 平台专属跳转
- `CloseLowLevel()` —— 关闭底层句柄
- `WriteLowLevel(Src, CountToWrite)` —— 平台专属写入
- `LogWriteError(Message)` —— **已废弃（5.6）**
- `LogWriteError(Message, LastError)` —— 新版（带系统错误码）
- `IsSilent()` —— 检查 `Flags & FILEWRITE_Silent`
- `FString Filename` / `uint32 Flags` / `int64 Pos` —— 3 个基础成员
- `TUniquePtr<IFileHandle> Handle` —— 独占指针
- `TArray64<uint8> BufferArray` / `int64 BufferSize` —— 缓冲
- `bool bLoggingError` —— 防止递归记录错误

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 323~329 行 | `FlushBuffer` |
| 第 330~335 行 | `SeekLowLevel` |
| 第 336~340 行 | `CloseLowLevel` |
| 第 341~347 行 | `WriteLowLevel` |
| 第 349~350 行 | 废弃的 `LogWriteError` |
| 第 351~358 行 | 新版 `LogWriteError` |
| 第 360~364 行 | `IsSilent` |
| 第 366~373 行 | 6 个成员 |
| 第 374 行 | 类结束 |

> 类比：像写书人的草稿本：先写草稿（BufferArray），攒够了再抄到正式本（FlushBuffer）。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 10 个 include | 1~14 | 版权头 + 10 个 include |
| 3 个缓冲大小宏 | 16~26 | 3 个缓冲大小宏 |
| `FFileManagerGeneric` 类声明 + `GetLowLevel` | 28~44 | `FFileManagerGeneric` 类声明 + `GetLowLevel` |
| 构造 / 析构 | 46~54 | 构造 / 析构 |
| 命令行 + 沙箱（3 个） | 56~70 | 命令行 + 沙箱（3 个） |
| 打开文件三件套（内联转调） | 72~87 | 打开文件三件套（内联转调） |
| 13 个 CORE_API 声明（实现在 .cpp） | 89~107 | 13 个 CORE_API 声明（实现在 .cpp） |
| `FindFiles` 扩展名版 | 109~119 | `FindFiles` 扩展名版 |
| 8 个 `IterateDirectory` | 121~155 | 8 个 `IterateDirectory` |
| 路径转换 + `FileSize` | 157~195 | 路径转换 + `FileSize` |
| `SendMessageToServer`（内联转调） | 197~207 | `SendMessageToServer`（内联转调） |
| 私有区：4 个内部辅助函数 | 209~219 | 私有区：4 个内部辅助函数 |
| `FArchiveFileReaderGeneric` 公有接口 | 222~249 | `FArchiveFileReaderGeneric` 公有接口 |
| `FArchiveFileReaderGeneric` protected + 成员 | 251~296 | `FArchiveFileReaderGeneric` protected + 成员 |
| `FArchiveFileWriterGeneric` 公有接口 | 299~321 | `FArchiveFileWriterGeneric` 公有接口 |
| `FArchiveFileWriterGeneric` protected + 成员 | 323~374 | `FArchiveFileWriterGeneric` protected + 成员 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `class X : public Y` | 继承 | `class FFileManagerGeneric : public IFileManager` |
| `FORCEINLINE T& F() const` | 强制内联 | `FORCEINLINE IPlatformFile& GetLowLevel() const` |
| `override final` | 重写且禁止再重写 | `virtual void Seek(int64 InPos) override final;` |
| `TUniquePtr<X>` | 独占智能指针（自动 delete） | `TUniquePtr<IFileHandle> Handle;` |
| `TArray64<uint8>` | 64 位长度的字节数组 | `TArray64<uint8> BufferArray;` |
| `enum { X = 0 };` | 匿名枚举常量 | `enum { bPrecacheAsSoonAsPossible = 0 };` |
| `friend class X;` | 友元类 | `friend class FArchiveFileReaderGenericTest;` |
| `UE_DEPRECATED(v, "msg")` | 标记废弃 | `UE_DEPRECATED(5.6, "...")` |
| `!!(X & Y)` | 转成 bool | `return !!(Flags & FILEREAD_Silent);` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 374 行 | 525 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 179 行 | 179 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `GetTimeStampPair` 无 `override` | 引擎原文如此 | 照抄不改 | 引擎遗漏，不补 |
| `UE_DEPRECATED` 字符串 | 英文原文 | 保留英文 | API 契约，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`（525 行）
2. 脚本验收：`codeOurs=179 / codeEngine=179 / diff=0`
3. 确认后下一个：`HAL\FMemory.inl`（8498 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`（374 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`（525 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\09_FileManagerGeneric.h_总md.md`