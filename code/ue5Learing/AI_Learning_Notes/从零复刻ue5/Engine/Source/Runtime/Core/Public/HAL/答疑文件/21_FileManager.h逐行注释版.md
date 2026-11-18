# 21 — `HAL/FileManager.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`（**291 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`
> **⚠️ 本节不能独立编译**——依赖 `GenericPlatform/GenericPlatformFile.h` 的 `IPlatformFile` / `FFileStatData`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

引擎里到处要读写文件：加载资源、存配置、导日志。如果每个地方都直接调 `fopen` / `CreateFile`，跨平台就崩了。所以 UE 抽象出 `IFileManager` —— **文件操作的统一入口**，各平台各自实现（`FWindowsFileManager` / `FUnixFileManager`…）。

**它包含 4 个枚举 + 1 个接口 + 1 个回调**：

| 类型 | 作用 |
|---|---|
| `EFileWrite` | 写文件标志位（7 个，可组合） |
| `EFileRead` | 读文件标志位（4 个，可组合） |
| `ECopyResult` | 复制结果码（3 个） |
| `EFileOpenFlags` | 打开文件标志位（3 个） |
| `FCopyProgress` | 复制进度回调接口 |
| `IFileManager` | **文件管理器接口**（40+ 个纯虚函数） |

**说个场景**：你想读一个文件：

```cpp
FArchive* Ar = IFileManager::Get().CreateFileReader(TEXT("D:/Data.bin"));
if (Ar) { Ar->Serialize(Buffer, Size); delete Ar; }   // 用完必须自己 delete
```

**`IFileManager` 的 40+ 个方法分 8 类**：

| 类别 | 方法 |
|---|---|
| **打开** | `CreateFileReader` / `CreateFileWriter` / `CreateDebugFileWriter` |
| **增删改** | `Delete` / `Copy` / `Move` / `MakeDirectory` / `DeleteDirectory` |
| **查询** | `FileExists` / `DirectoryExists` / `IsReadOnly` / `FileSize` / `GetStatData` |
| **查找** | `FindFiles`（2 个重载）/ `FindFilesRecursive` |
| **遍历** | `IterateDirectory` / `IterateDirectoryRecursively` / `IterateDirectoryStat` / `IterateDirectoryStatRecursively`（各 2 个重载 = 8 个） |
| **时间** | `GetFileAgeSeconds` / `GetTimeStamp` / `GetTimeStampPair` / `SetTimeStamp` / `GetAccessTimeStamp` |
| **路径** | `ConvertToRelativePath` / `ConvertToAbsolutePathForExternalAppForRead` / `ForWrite` / `GetFilenameOnDisk` |
| **其他** | `Get` / `ProcessCommandLineOptions` / `SetSandboxEnabled` / `IsSandboxEnabled` / `SendMessageToServer` |

> 🔑 **一句话**：`FileManager.h` = **文件操作的「总服务台」**，40+ 个窗口，各平台自己派人值班。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + include + 文件名长度宏（第 1~11 行）

```cpp
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
```

**本段新元素**

- `CoreTypes.h` —— `uint8` / `uint32` / `int64` / `TCHAR`
- `Containers/UnrealString.h` —— `FString`
- `Misc/DateTime.h` —— `FDateTime`
- `GenericPlatform/GenericPlatformFile.h` —— `IPlatformFile` / `FFileStatData`
- `MAX_UNREAL_FILENAME_LENGTH_DEPRECATED` —— 文件名最大长度（已废弃）
- `PLATFORM_MAX_FILEPATH_LENGTH_DEPRECATED` —— 平台定义的最大路径长度

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~8 行 | 4 个 include |
| 第 10~11 行 | 文件名长度宏 |

> 类比：像开工前领工具：类型、字符串、时间、平台文件接口，四样齐了。

### 二、`EFileWrite` 写文件标志位（第 14~23 行）

```cpp
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
```

**本段新元素**

- `enum EFileWrite` —— 普通枚举（非 enum class），可直接当整数用
- `FILEWRITE_None = 0x00` —— 无标志
- `FILEWRITE_NoFail = 0x01` —— 失败不断言
- `FILEWRITE_NoReplaceExisting = 0x02` —— 不覆盖已存在文件
- `FILEWRITE_EvenIfReadOnly = 0x04` —— 只读也写
- `FILEWRITE_Append = 0x08` —— 追加模式
- `FILEWRITE_AllowRead = 0x10` —— 允许同时读
- `FILEWRITE_Silent = 0x20` —— 静默（失败不打印）
- `0x01` / `0x02` / `0x04` … —— 十六进制位标志，可 `|` 组合

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 14~15 行 | 枚举声明 |
| 第 16~22 行 | 7 个标志位 |
| 第 23 行 | 枚举结束 |

> 类比：像七个开关：想覆盖就关掉 NoReplace，想追加就打开 Append，可以同时拨好几个。

### 三、`EFileRead` 读文件标志位（第 26~32 行）

```cpp
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
```

**本段新元素**

- `FILEREAD_None = 0x00` —— 无标志
- `FILEREAD_NoFail = 0x01` —— 失败不断言
- `FILEREAD_Silent = 0x02` —— 静默
- `FILEREAD_AllowWrite = 0x04` —— 允许同时写

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 26~27 行 | 枚举声明 |
| 第 28~31 行 | 4 个标志位 |
| 第 32 行 | 枚举结束 |

> 类比：读文件只有四个开关，比写少三个。

### 四、`ECopyResult` 复制结果码（第 35~40 行）

```cpp
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
```

**本段新元素**

- `COPY_OK = 0x00` —— 成功
- `COPY_Fail = 0x01` —— 失败
- `COPY_Canceled = 0x02` —— 被取消

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 35~36 行 | 枚举声明 |
| 第 37~39 行 | 3 个结果码 |
| 第 40 行 | 枚举结束 |

> 类比：像快递的三种状态：已签收、投递失败、用户取消。

### 五、`FCopyProgress` 进度回调（第 43~46 行）

```cpp
// FCopyProgress：复制进度回调接口。
struct FCopyProgress
{
// Poll：每复制一段就回调一次，Fraction 是 0~1 的进度；返回 false 表示取消复制。
	virtual bool Poll( float Fraction )=0;
};
```

**本段新元素**

- `struct FCopyProgress` —— 复制进度回调接口
- `virtual bool Poll(float Fraction) = 0` —— 纯虚函数，每复制一段回调一次
- `Fraction` —— 0~1 的进度值
- 返回 `false` 表示取消复制

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 43~44 行 | 结构体声明 |
| 第 45 行 | 纯虚 `Poll` |
| 第 46 行 | 结构体结束 |

> 类比：像下载进度条：每下一点就回调一次，你返回 false 就取消。

### 六、`EFileOpenFlags` 打开标志位（第 49~54 行）

```cpp
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
```

**本段新元素**

- `IO_READ = 0x01` —— 以读方式打开
- `IO_WRITE = 0x02` —— 以写方式打开
- `IO_APPEND = 0x40` —— 写时保留已有数据，指针移到末尾

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 49~50 行 | 枚举声明 |
| 第 51~53 行 | 3 个标志位 |
| 第 54 行 | 枚举结束 |

> 类比：像开门的三种方式：只进（读）、只出（写）、从末尾接着写（追加）。

### 七、`IFileManager` 类声明 + 单例 + 沙箱（第 56~74 行）

```cpp
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
```

**本段新元素**

- `class IFileManager` —— 文件管理器接口
- `protected: IFileManager() {}` —— 构造函数 protected，只能派生
- `static CORE_API IFileManager& Get()` —— 取全局单例
- `ProcessCommandLineOptions()` —— 解析命令行
- `SetSandboxEnabled(bool)` / `IsSandboxEnabled()` —— 沙箱开关

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 56~61 行 | 类声明 + protected 构造 |
| 第 66 行 | `Get()` 单例 |
| 第 69 行 | `ProcessCommandLineOptions` |
| 第 72~74 行 | 沙箱开关 |

> 类比：像服务台的总机：只有一个号码（Get），打过去什么都能问。

### 八、打开文件三件套（第 85~101 行）

```cpp
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
```

**本段新元素**

- `CreateFileReader(Filename, ReadFlags=0)` —— 打开读，返回 `FArchive*`
- `CreateFileWriter(Filename, WriteFlags=0)` —— 打开写，返回 `FArchive*`
- `CreateDebugFileWriter(...)` —— 打开调试文件写（`#if ALLOW_DEBUG_FILES` 内）
- `FArchive*` —— 序列化基类，读写都通过它
- **用完必须调用方 `delete`** —— 引擎注释明确说明

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 85 行 | `CreateFileReader` |
| 第 96 行 | `CreateFileWriter` |
| 第 99~101 行 | `CreateDebugFileWriter`（条件编译） |

> 类比：像借书：借的时候给你一本书（FArchive），还的时候要你自己还（delete）。

### 九、增删改查 9 个方法（第 104~128 行）

```cpp
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
```

**本段新元素**

- `IsReadOnly(Filename)` —— 查询只读
- `Delete(Filename, RequireExists, EvenReadOnly, Quiet)` —— 删除文件
- `Copy(Dest, Src, Replace, EvenIfReadOnly, Attributes, Progress, ReadFlags, WriteFlags)` —— 复制（8 个参数）
- `Move(Dest, Src, Replace, EvenIfReadOnly, Attributes, bDoNotRetryOrError)` —— 移动/重命名
- `FileExists` / `DirectoryExists` —— 存在性查询
- `MakeDirectory(Path, Tree)` —— 创建目录（Tree 递归）
- `DeleteDirectory(Path, RequireExists, Tree)` —— 删除目录
- `GetStatData(FilenameOrDirectory)` —— 取元数据（`FFileStatData`）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 104 行 | `IsReadOnly` |
| 第 107 行 | `Delete` |
| 第 110 行 | `Copy`（参数最多） |
| 第 113 行 | `Move` |
| 第 116~119 行 | `FileExists` / `DirectoryExists` |
| 第 122~125 行 | `MakeDirectory` / `DeleteDirectory` |
| 第 128 行 | `GetStatData` |

> 类比：像文件柜的九个抽屉：查、删、复制、移动、看有没有、建目录、删目录、看详情。

### 十、查找 3 个方法（第 131~146 行）

```cpp
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
```

**本段新元素**

- `FindFiles(FileNames, Filename, Files, Directories)` —— 按通配符查找
- `FindFiles(FoundFiles, Directory, FileExtension)` —— 按扩展名过滤
- `FindFilesRecursive(FileNames, StartDirectory, Filename, Files, Directories, bClearFileNames)` —— 递归查找
- `TArray<FString>&` —— 输出参数（引用传数组）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 131 行 | `FindFiles`（通配符版） |
| 第 143 行 | `FindFiles`（扩展名版） |
| 第 146 行 | `FindFilesRecursive` |

> 类比：像找东西：按名字找（通配符）、按类型找（扩展名）、连子文件夹一起找（递归）。

### 十一、遍历目录 8 个方法（第 154~210 行）

```cpp
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
```

**本段新元素**

- `IterateDirectory(Directory, FDirectoryVisitor&)` —— 单层，虚接口访问者
- `IterateDirectory(Directory, FDirectoryVisitorFunc)` —— 单层，函数对象访问者
- `IterateDirectoryRecursively(...)` —— 递归，2 个重载
- `IterateDirectoryStat(...)` —— 单层 + stat 数据，2 个重载
- `IterateDirectoryStatRecursively(...)` —— 递归 + stat 数据，2 个重载
- `IPlatformFile::FDirectoryVisitor` —— 虚接口访问者（要继承）
- `IPlatformFile::FDirectoryVisitorFunc` —— 函数对象访问者（传 lambda）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 154 行 | 单层 + 虚接口 |
| 第 162 行 | 单层 + 函数对象 |
| 第 170 行 | 递归 + 虚接口 |
| 第 178 行 | 递归 + 函数对象 |
| 第 186 行 | 单层 stat + 虚接口 |
| 第 194 行 | 单层 stat + 函数对象 |
| 第 202 行 | 递归 stat + 虚接口 |
| 第 210 行 | 递归 stat + 函数对象 |

> 类比：像查户口：单层/递归 × 虚接口/函数对象 × 带不带 stat = 8 种查法。

### 十二、时间相关 5 个方法（第 213~236 行）

```cpp
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
```

**本段新元素**

- `GetFileAgeSeconds(Filename)` —— 文件年龄（秒）
- `GetTimeStamp(Path)` —— 修改时间（失败返回 `FDateTime::MinValue()`）
- `GetTimeStampPair(PathA, PathB, OutA, OutB)` —— 一次取两个（省系统调用）
- `SetTimeStamp(Path, TimeStamp)` —— 设置修改时间
- `GetAccessTimeStamp(Filename)` —— 最后访问时间

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 213 行 | `GetFileAgeSeconds` |
| 第 218 行 | `GetTimeStamp` |
| 第 226 行 | `GetTimeStampPair` |
| 第 231 行 | `SetTimeStamp` |
| 第 236 行 | `GetAccessTimeStamp` |

> 类比：像查档案的时间戳：多久没改（年龄）、什么时候改的、什么时候被看过。

### 十三、路径转换 3 个方法（第 245~263 行）

```cpp
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
```

**本段新元素**

- `ConvertToRelativePath(Filename)` —— 转相对路径
- `ConvertToAbsolutePathForExternalAppForRead(Filename)` —— 转绝对路径（外部程序读）
- `ConvertToAbsolutePathForExternalAppForWrite(Filename)` —— 转绝对路径（外部程序写）
- **为什么分 Read / Write** —— 某些平台（如 macOS sandbox）读写路径不同

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 245 行 | `ConvertToRelativePath` |
| 第 254 行 | `ConvertToAbsolutePathForExternalAppForRead` |
| 第 263 行 | `ConvertToAbsolutePathForExternalAppForWrite` |

> 类比：像给外人指路：自己人用相对地址（相对路径），外人必须给完整地址（绝对路径）。

### 十四、`FileSize` / `SendMessageToServer` / `GetFilenameOnDisk`（第 271~291 行）

```cpp
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
```

**本段新元素**

- `FileSize(Filename)` —— 文件大小（字节），不存在返回 `INDEX_NONE`
- `SendMessageToServer(Message, Handler)` —— 给文件服务器发消息（阻塞）
- `GetFilenameOnDisk(Filename)` —— 返回与磁盘一致的大小写路径
- `IPlatformFile::IFileServerMessageHandler*` —— 消息处理器

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 271 行 | `FileSize` |
| 第 281 行 | `SendMessageToServer` |
| 第 290 行 | `GetFilenameOnDisk` |
| 第 291 行 | 类结束 |

> 类比：像三个特殊窗口：问大小、给服务器捎话、问磁盘上真实的大小写。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + include + 文件名长度宏 | 1~11 | 版权头 + include + 文件名长度宏 |
| `EFileWrite` 写文件标志位 | 14~23 | `EFileWrite` 写文件标志位 |
| `EFileRead` 读文件标志位 | 26~32 | `EFileRead` 读文件标志位 |
| `ECopyResult` 复制结果码 | 35~40 | `ECopyResult` 复制结果码 |
| `FCopyProgress` 进度回调 | 43~46 | `FCopyProgress` 进度回调 |
| `EFileOpenFlags` 打开标志位 | 49~54 | `EFileOpenFlags` 打开标志位 |
| `IFileManager` 类声明 + 单例 + 沙箱 | 56~74 | `IFileManager` 类声明 + 单例 + 沙箱 |
| 打开文件三件套 | 85~101 | 打开文件三件套 |
| 增删改查 9 个方法 | 104~128 | 增删改查 9 个方法 |
| 查找 3 个方法 | 131~146 | 查找 3 个方法 |
| 遍历目录 8 个方法 | 154~210 | 遍历目录 8 个方法 |
| 时间相关 5 个方法 | 213~236 | 时间相关 5 个方法 |
| 路径转换 3 个方法 | 245~263 | 路径转换 3 个方法 |
| `FileSize` / `SendMessageToServer` / `GetFilenameOnDisk` | 271~291 | `FileSize` / `SendMessageToServer` / `GetFilenameOnDisk` |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `enum X { A = 0x01, B = 0x02 }` | 位标志枚举 | `enum EFileWrite { FILEWRITE_NoFail = 0x01, ... }` |
| `virtual T F() = 0;` | 纯虚函数 | `virtual bool FileExists(const TCHAR*) = 0;` |
| `static CORE_API T& Get()` | 静态单例访问 | `static CORE_API IFileManager& Get();` |
| `T& Out` | 输出参数（引用） | `TArray<FString>& FileNames` |
| `T* Ptr = nullptr` | 默认参数 | `FCopyProgress* Progress = nullptr` |
| `#if ALLOW_DEBUG_FILES` | 条件编译 | `#if ALLOW_DEBUG_FILES` |
| `protected: X() {}` | protected 构造（只能派生） | `IFileManager() {}` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 291 行 | 366 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 85 行 | 85 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `@param` / `@return` | 英文说明 | 中文说明 | 保留 Doxygen 标签 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`（366 行）
2. 脚本验收：`codeOurs=85 / codeEngine=85 / diff=0`
3. 确认后下一个：`HAL\FileManagerGeneric.h`（15333 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`（291 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`（366 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\08_FileManager.h_总md.md`