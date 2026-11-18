# 28 — `HAL/IPlatformFileManagedStorageWrapper.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`（**1085 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `GConfig` / `FPaths` / `FRWLock`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

手机 / 主机的**持久化存储有配额**（比如「存档最多 100MB」）。如果不管，玩家存档写爆了磁盘，游戏就崩了。`IPlatformFileManagedStorageWrapper.h` 就是**存储配额管理器**：它把文件按**分类（Category）**分组，每个分类有配额，写文件前先检查配额。

**它包含 7 个核心类型**：

| 类型 | 作用 |
|---|---|
| `FPersistentManagedFile` | 一个被托管的文件（路径 + 分类索引） |
| `FManagedStorageFileLockRegistry` | **按文件名分配锁**的注册表（引用计数） |
| `FManagedStorageScopeFileLock` | 作用域文件锁（RAII） |
| `FPersistentStorageCategory` | **一个分类**（名字 + 目录列表 + 配额 + 已用大小） |
| `FPersistentStorageManager` | **管理器单例**（所有分类 + 根目录） |
| `FManagedStorageFileWriteHandle` | 托管存储的写句柄（写前查配额） |
| `TManagedStoragePlatformFile<BaseClass>` | **模板化**的平台文件包装器 |

**核心设计 ①：分类 + 配额**

```
FPersistentStorageCategory
├─ CategoryName          "SaveGame"
├─ Directories[]         [".../Saved/SaveGames/"]
├─ StorageQuota          100MB
├─ OptionalStorageQuota  20MB（可选部分）
└─ UsedQuota             当前已用（std::atomic<int64>）
```

**核心设计 ②：按文件名分配锁**（第 63~107 行）

```cpp
TMap<FString, FLockData> FileLockMap;   // 文件名 → 锁数据
struct FLockData {
    TUniquePtr<FCriticalSection> CS;    // 每个文件一把锁
    int32 RefCount = 0;                 // 引用计数
};
```

**为什么不用一把大锁**：不同文件的写操作可以并行，只有同一文件的写才需要互斥。引用计数保证「没人用了就回收锁」。

**核心设计 ③：配额检查用 CAS 循环**（第 295~309 行）

```cpp
do {
    NewUsedQuota = OldUsedQuota + (NewFileSize - OldFileSize);
    if (NewUsedQuota > StorageQuota) return RespectQuota;   // 超配额
} while (!UsedQuota.compare_exchange_weak(OldUsedQuota, NewUsedQuota));
```

`compare_exchange_weak` 是**无锁原子操作**：多线程同时写不同文件时，不需要加锁就能安全更新 `UsedQuota`。

**核心设计 ④：模板化包装器**（第 823~825 行）

```cpp
// NOTE: This is templated rather than a polymorphic wrapper because a lot code
// expects the physical layer not to be a wrapper.
template<class BaseClass>
class TManagedStoragePlatformFile : public BaseClass
```

**为什么用模板**：很多代码期望「物理层不是包装器」（比如 `&BaseClass::DeleteDirectoryRecursively == &IPlatformFile::DeleteDirectoryRecursively` 这种函数指针比较）。模板继承能保持类型身份，多态包装会破坏它。

**说个场景**：

```cpp
// 写存档前自动检查配额
IFileHandle* H = File.OpenWrite(".../SaveGames/Slot1.sav");
if (!H) { /* 配额满了，提示玩家清理 */ }
```

**`IsReady()` 的三重检查**（第 348~355 行）：

```cpp
return GConfig && GConfig->IsReadyForUse()      // 配置就绪
    && FCommandLine::IsInitialized()            // 命令行就绪
    && !GIsCriticalError;                       // 无致命错误
```

未就绪时所有操作**直接透传**给下一层（不追踪），避免启动早期崩溃。

> 🔑 **一句话**：`IPlatformFileManagedStorageWrapper.h` = **存储配额的「守门员」**，按分类管配额，写前查、超了拒。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 30 个 include + 日志分类（第 1~39 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入 Algo::Find：查找算法。
#include "Algo/Find.h"
// 引入 Algo::IndexOf：索引查找算法。
#include "Algo/IndexOf.h"
// 引入 Async：异步任务。
#include "Async/Async.h"
// 引入任务图接口：ENamedThreads。
#include "Async/TaskGraphInterfaces.h"
// 引入 TArray：动态数组。
#include "Containers/Array.h"
// 引入 TArrayView：数组视图（不拥有数据）。
#include "Containers/ArrayView.h"
// 引入 TMap：哈希表。
#include "Containers/Map.h"
// 引入 TSparseArray：稀疏数组。
#include "Containers/SparseArray.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入全局量：GConfig / GIsCriticalError。
#include "CoreGlobals.h"
// 引入基础类型层：uint8 / int32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入通用平台文件接口：IPlatformFile / IFileHandle / FFileStatData。
#include "GenericPlatform/GenericPlatformFile.h"
// 引入临界区：FCriticalSection。
#include "HAL/CriticalSection.h"
// 引入平台 C 运行时。
#include "HAL/PlatformCrt.h"
// 引入平台字符串函数。
#include "HAL/PlatformString.h"
// 引入日志分类：DECLARE_LOG_CATEGORY_EXTERN。
#include "Logging/LogCategory.h"
// 引入日志宏：UE_LOG。
#include "Logging/LogMacros.h"
// 引入数值极限：MAX_int64 / INDEX_NONE。
#include "Math/NumericLimits.h"
// 引入数学工具：FMath::Max。
#include "Math/UnrealMathUtility.h"
// 引入断言宏：check / ensureAlways / verify。
#include "Misc/AssertionMacros.h"
// 引入命令行：FCommandLine。
#include "Misc/CommandLine.h"
// 引入配置缓存：GConfig。
#include "Misc/ConfigCacheIni.h"
// 引入枚举类标志宏：ENUM_CLASS_FLAGS。
#include "Misc/EnumClassFlags.h"
// 引入 TOptional：可选值容器。
#include "Misc/Optional.h"
// 引入命令行解析：FParse。
#include "Misc/Parse.h"
// 引入路径工具：FPaths。
#include "Misc/Paths.h"
// 引入作用域锁：FScopeLock。
#include "Misc/ScopeLock.h"
// 引入读写锁作用域：FReadScopeLock / FWriteScopeLock。
#include "Misc/ScopeRWLock.h"
// 引入 TUniquePtr：独占智能指针。
#include "Templates/UniquePtr.h"
// 引入模板工具：MoveTemp。
#include "Templates/UnrealTemplate.h"
// 引入追踪通道：SCOPED_NAMED_EVENT。
#include "Trace/Detail/Channel.h"

// 引入标准库 atomic：std::atomic<int64>。
#include <atomic>

// LogPlatformFileManagedStorage：托管存储日志分类。
CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogPlatformFileManagedStorage, Log, All);
```

**本段新元素**

- `Algo/Find.h` / `Algo/IndexOf.h` —— `Algo::FindByPredicate` / `Algo::FindBy`
- `Async/Async.h` / `Async/TaskGraphInterfaces.h` —— `AsyncTask` / `ENamedThreads`
- `Containers/Array.h` / `ArrayView.h` / `Map.h` / `SparseArray.h` —— 容器
- `CoreGlobals.h` —— `GConfig` / `GIsCriticalError`
- `GenericPlatform/GenericPlatformFile.h` —— `IPlatformFile` / `IFileHandle`
- `HAL/CriticalSection.h` —— `FCriticalSection`
- `Misc/CommandLine.h` —— `FCommandLine`
- `Misc/ConfigCacheIni.h` —— `GConfig`
- `Misc/EnumClassFlags.h` —— `ENUM_CLASS_FLAGS`
- `Misc/Optional.h` —— `TOptional`
- `Misc/Paths.h` —— `FPaths`
- `Misc/ScopeLock.h` / `ScopeRWLock.h` —— `FScopeLock` / `FReadScopeLock` / `FWriteScopeLock`
- `<atomic>` —— `std::atomic<int64>`
- `DECLARE_LOG_CATEGORY_EXTERN(LogPlatformFileManagedStorage, Log, All)` —— 托管存储日志分类

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~35 行 | 30 个 include |
| 第 37 行 | `<atomic>` |
| 第 39 行 | 日志分类 |

> 类比：像开工前领 30 样工具：算法、异步、容器、全局量、平台文件、锁、命令行、配置、枚举标志、可选值、路径、作用域锁、原子。

### 二、`ManagedStorageInternal` 命名空间（第 41~47 行）

```cpp
// 命名空间 ManagedStorageInternal：内部辅助函数。
namespace ManagedStorageInternal
{
// 说明：和 FPaths::IsUnderDirectory 一样，但假定路径已经是完整路径。
	// 和 FPaths::IsUnderDirectory 一样，但假定路径已经是完整路径
// 说明：而且永远不区分大小写，因为我们只关心过滤，不关心目录是否真的存在。
	// 而且永远不区分大小写，因为我们只关心过滤，不关心目录是否真的存在。
	// not whether a directory actually exists.
// IsUnderDirectory：判断 InPath 是否在 InDirectory 之下。
	CORE_API bool IsUnderDirectory(const FString& InPath, const FString& InDirectory);
// 命名空间结束。
}
```

**本段新元素**

- `namespace ManagedStorageInternal` —— 内部辅助函数命名空间
- `IsUnderDirectory(InPath, InDirectory)` —— 判断路径是否在目录之下
- **和 `FPaths::IsUnderDirectory` 的区别**：假定路径已是完整路径 + 永远不区分大小写

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 41~42 行 | 命名空间开始 |
| 第 43~45 行 | 说明注释 |
| 第 46 行 | `IsUnderDirectory` 声明 |
| 第 47 行 | 命名空间结束 |

> 类比：像门牌号比对：只看前缀，不管大小写。

### 三、`FPersistentManagedFile`（第 49~61 行）

```cpp
// FPersistentManagedFile：一个被托管的持久化文件。
struct FPersistentManagedFile
{
// FullFilename：完整文件路径。
	FString FullFilename;
// Category：所属分类索引（INDEX_NONE 表示不属于任何分类）。
	int32 Category = INDEX_NONE;

// IsValid：是否有效（分类索引 >= 0）。
	bool IsValid() const { return Category >= 0; }
// operator bool：显式转 bool（同 IsValid）。
	explicit operator bool() const { return Category >= 0; }

// Clear：清空（分类索引置 INDEX_NONE）。
	void Clear()
	{
// 分类索引置 INDEX_NONE。
		Category = INDEX_NONE;
	}
// 结构体结束。
};
```

**本段新元素**

- `struct FPersistentManagedFile` —— 一个被托管的文件
- `FString FullFilename` —— 完整文件路径
- `int32 Category = INDEX_NONE` —— 所属分类索引（`INDEX_NONE` 表示不属于任何分类）
- `IsValid()` —— 分类索引 >= 0
- `explicit operator bool()` —— 显式转 bool（同 `IsValid`）
- `Clear()` —— 分类索引置 `INDEX_NONE`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 49~52 行 | 结构体声明 + 2 个成员 |
| 第 54~55 行 | `IsValid` / `operator bool` |
| 第 57~60 行 | `Clear` |
| 第 61 行 | 结构体结束 |

> 类比：像一张借书卡：书名（路径）+ 分类号（Category），分类号 -1 表示「不归任何分类管」。

### 四、`FManagedStorageFileLockRegistry`（按文件名分配锁）（第 63~107 行）

```cpp
// FManagedStorageFileLockRegistry：按文件名分配锁的注册表。
class FManagedStorageFileLockRegistry
{
// 私有区。
private:
// 友元：FManagedStorageScopeFileLock 可访问私有成员。
	friend class FManagedStorageScopeFileLock;

// GetLock：取指定文件的锁（引用计数 +1）。
	FCriticalSection* GetLock(const FString& InFilename)
// 函数体开始。
	{
// 算文件名的哈希。
		uint32 KeyHash = GetTypeHash(InFilename);

// 加锁保护 FileLockMap。
		FScopeLock Lock(&FileLockMapCS);

// 找到或新建该文件的锁数据。
		FLockData& LockData = FileLockMap.FindOrAddByHash(KeyHash, InFilename);
// 引用计数 +1。
		++LockData.RefCount;

// 返回锁指针。
		return LockData.CS.Get();
// 函数体结束。
	}

// ReleaseLock：释放指定文件的锁（引用计数 -1）。
	void ReleaseLock(const FString& InFilename)
// 函数体开始。
	{
// 算文件名的哈希。
		uint32 KeyHash = GetTypeHash(InFilename);

// 加锁保护 FileLockMap。
		FScopeLock Lock(&FileLockMapCS);

// 找到该文件的锁数据。
		FLockData* LockData = FileLockMap.FindByHash(KeyHash, InFilename);
// 断言：必须存在。
		check(LockData);

// 引用计数 -1。
		--LockData->RefCount;

// 如果引用计数归零。
		if (LockData->RefCount == 0)
// 条件体开始。
		{
// 从表里移除。
			FileLockMap.RemoveByHash(KeyHash, InFilename);
// 条件体结束。
		}
// 函数体结束。
	}

// 私有区。
private:

// FLockData：每个文件的锁数据。
	struct FLockData
	{
// CS：临界区（独占指针，懒创建）。
		TUniquePtr<FCriticalSection> CS{ MakeUnique<FCriticalSection>() };
// RefCount：引用计数。
		int32 RefCount = 0;
// 结构体结束。
	};

// FileLockMap：文件名 → 锁数据。
	TMap<FString, FLockData> FileLockMap;
// FileLockMapCS：保护 FileLockMap 的临界区。
	FCriticalSection FileLockMapCS;
// 类结束。
};
```

**本段新元素**

- `class FManagedStorageFileLockRegistry` —— 按文件名分配锁的注册表
- `friend class FManagedStorageScopeFileLock` —— 友元
- `GetLock(InFilename)` —— 取锁（引用计数 +1）
- `GetTypeHash(InFilename)` —— 算文件名哈希
- `FScopeLock Lock(&FileLockMapCS)` —— 加锁保护 `FileLockMap`
- `FileLockMap.FindOrAddByHash(KeyHash, InFilename)` —— 找到或新建锁数据
- `++LockData.RefCount` —— 引用计数 +1
- `ReleaseLock(InFilename)` —— 释放锁（引用计数 -1）
- `FileLockMap.FindByHash(KeyHash, InFilename)` —— 找到锁数据
- `if (LockData->RefCount == 0)` —— 归零就移除
- `struct FLockData` —— 每个文件的锁数据（`TUniquePtr<FCriticalSection> CS` + `int32 RefCount`）
- `TMap<FString, FLockData> FileLockMap` —— 文件名 → 锁数据
- `FCriticalSection FileLockMapCS` —— 保护 `FileLockMap` 的锁

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 63~66 行 | 类声明 + 友元 |
| 第 68~78 行 | `GetLock` |
| 第 80~95 行 | `ReleaseLock` |
| 第 97~103 行 | `FLockData` 结构 |
| 第 105~107 行 | 2 个成员 |

> 类比：像钥匙柜：每个房间（文件）一把钥匙，借的时候登记（+1），还的时候注销（-1），没人借就收起来。

### 五、`FManagedStorageScopeFileLock`（RAII 文件锁）（第 109~137 行）

```cpp
// FManagedStorageScopeFileLock：作用域文件锁（RAII）。
class FManagedStorageScopeFileLock
{
// 公有区。
public:
// 构造函数：保存文件并加锁。
	FManagedStorageScopeFileLock(FPersistentManagedFile InManagedFile)
// 初始化列表：移动构造文件。
		: ManagedFile(MoveTemp(InManagedFile))
// 函数体开始。
	{
// 加锁。
		Lock();
// 函数体结束。
	}

// 析构函数：解锁。
	~FManagedStorageScopeFileLock()
// 函数体开始。
	{
// 解锁。
		Unlock();
// 函数体结束。
	}

// 删除拷贝构造。
	FManagedStorageScopeFileLock(const FManagedStorageScopeFileLock&) = delete;
// 删除拷贝赋值。
	FManagedStorageScopeFileLock& operator=(const FManagedStorageScopeFileLock&) = delete;

// 删除移动构造。
	FManagedStorageScopeFileLock(FManagedStorageScopeFileLock&& InOther) = delete;
// 删除移动赋值。
	FManagedStorageScopeFileLock& operator=(FManagedStorageScopeFileLock&& InOther) = delete;
	
// Unlock：解锁（实现在 .cpp）。
	CORE_API void Unlock();

// 私有区。
private:
// Lock：加锁（实现在 .cpp）。
	CORE_API void Lock();

// 私有区。
private:
// ManagedFile：被锁的文件。
	FPersistentManagedFile ManagedFile;
// pFileCS：锁指针（空表示未加锁）。
	FCriticalSection* pFileCS = nullptr;
// 类结束。
};
```

**本段新元素**

- `class FManagedStorageScopeFileLock` —— 作用域文件锁
- `FManagedStorageScopeFileLock(FPersistentManagedFile)` —— 构造时加锁
- `~FManagedStorageScopeFileLock()` —— 析构时解锁
- 4 个 `= delete` —— 禁拷贝 + 禁移动
- `Unlock()` / `Lock()` —— 实现在 .cpp
- `FPersistentManagedFile ManagedFile` —— 被锁的文件
- `FCriticalSection* pFileCS = nullptr` —— 锁指针（空表示未加锁）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 109~116 行 | 类声明 + 构造 |
| 第 118~121 行 | 析构 |
| 第 123~127 行 | 4 个 delete |
| 第 129~132 行 | `Unlock` / `Lock` |
| 第 134~137 行 | 2 个成员 |

> 类比：像进房间自动锁门、出门自动开锁：不用手动管。

### 六、`EPersistentStorageManagerFileSizeFlags` 枚举（第 139~145 行）

```cpp
// EPersistentStorageManagerFileSizeFlags：文件大小更新标志。
enum class EPersistentStorageManagerFileSizeFlags : uint8
{
// None：无标志。
	None = 0,
// OnlyUpdateIfLess：只在更小时才更新。
	OnlyUpdateIfLess = (1 << 0),
// RespectQuota：遵守配额限制。
	RespectQuota = (1 << 1)
// 枚举体结束。
};
// 给这个 enum class 生成位运算与判位工具。
ENUM_CLASS_FLAGS(EPersistentStorageManagerFileSizeFlags);
```

**本段新元素**

- `enum class EPersistentStorageManagerFileSizeFlags : uint8` —— 文件大小更新标志
- `None = 0` —— 无标志
- `OnlyUpdateIfLess = (1 << 0)` —— 只在更小时才更新
- `RespectQuota = (1 << 1)` —— 遵守配额限制
- `ENUM_CLASS_FLAGS(...)` —— 生成位运算

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 139~144 行 | 枚举定义（3 个值） |
| 第 145 行 | 生成位运算 |

> 类比：像两个开关：只在更小时更新、要不要查配额。

### 七、`FPersistentStorageCategory` 构造 + 6 个 getter（第 147~197 行）

```cpp
// FPersistentStorageCategory：一个持久化存储分类（有配额）。
struct FPersistentStorageCategory
{
// 公有区。
public:
// 构造函数：保存分类名、目录列表、配额、可选配额。
	FPersistentStorageCategory(FString InCategoryName, TArray<FString> InDirectories, const int64 InQuota, const int64 InOptionalQuota)
// 初始化列表：移动构造分类名。
		: CategoryName(MoveTemp(InCategoryName))
// 初始化列表：移动构造目录列表。
		, Directories(MoveTemp(InDirectories))
// 初始化列表：保存配额。
		, StorageQuota(InQuota)
// 初始化列表：保存可选配额。
		, OptionalStorageQuota(InOptionalQuota)
// 函数体开始。
	{
// 断言：可选配额必须小于总配额（或总配额 <= 0 表示无限）。
		check(OptionalStorageQuota < StorageQuota || StorageQuota <= 0); // 可选配额应是总配额的一部分
// 函数体结束。
	}

// GetCategoryName：取分类名。
	const FString& GetCategoryName() const
// 函数体开始。
	{
// 返回分类名。
		return CategoryName;
// 函数体结束。
	}

// GetCategoryQuota：取分类配额。
	int64 GetCategoryQuota() const
// 函数体开始。
	{
// 返回配额。
		return StorageQuota;
// 函数体结束。
	}

// GetCategoryOptionalQuota：取可选配额。
	int64 GetCategoryOptionalQuota() const
// 函数体开始。
	{
// 返回可选配额。
		return OptionalStorageQuota;
// 函数体结束。
	}

// GetUsedSize：取已用大小。
	int64 GetUsedSize() const
// 函数体开始。
	{
// 返回已用配额。
		return UsedQuota;
// 函数体结束。
	}

// GetAvailableSize：取可用大小。
	int64 GetAvailableSize() const
// 函数体开始。
	{
// 实际配额先取配置值。
		int64 ActualStorageQuota = StorageQuota;
// 如果配额为负（表示无限）。
		if (ActualStorageQuota < 0)
// 条件体开始。
		{
// 用 MAX_int64 代替。
			ActualStorageQuota = MAX_int64;
// 条件体结束。
		}
// 返回 配额 - 已用。
		return ActualStorageQuota - UsedQuota;
// 函数体结束。
	}

// IsInCategory：判断路径是否属于本分类。
	bool IsInCategory(const FString& Path) const
// 函数体开始。
	{
// 转调 ShouldManageFile。
		return ShouldManageFile(Path);
// 函数体结束。
	}

// IsCategoryFull：判断分类是否已满。
	bool IsCategoryFull() const
// 函数体开始。
	{
// 可用大小 <= 0 即为满。
		return GetAvailableSize() <= 0;
// 函数体结束。
	}
```

**本段新元素**

- `struct FPersistentStorageCategory` —— 一个持久化存储分类
- 构造函数：`(CategoryName, Directories, Quota, OptionalQuota)`
- `check(OptionalStorageQuota < StorageQuota || StorageQuota <= 0)` —— 可选配额必须是总配额的一部分
- `GetCategoryName()` / `GetCategoryQuota()` / `GetCategoryOptionalQuota()` —— 3 个 getter
- `GetUsedSize()` —— 已用大小
- `GetAvailableSize()` —— 可用大小（配额为负时用 `MAX_int64`）
- `IsInCategory(Path)` —— 转调 `ShouldManageFile`
- `IsCategoryFull()` —— 可用大小 <= 0

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 147~157 行 | 结构体声明 + 构造 |
| 第 159~172 行 | 3 个 getter |
| 第 174~177 行 | `GetUsedSize` |
| 第 179~187 行 | `GetAvailableSize` |
| 第 189~192 行 | `IsInCategory` |
| 第 194~197 行 | `IsCategoryFull` |

> 类比：像一个仓库：有名字、有容量、有已用量，能查还剩多少、满没满。

### 八、`AddOrUpdateFile` / `RemoveFile`（第 199~245 行）

```cpp
// AddOrUpdateFile：添加或更新文件大小。
	EPersistentStorageManagerFileSizeFlags AddOrUpdateFile(const FString& Filename, const int64 FileSize, EPersistentStorageManagerFileSizeFlags Flags)
// 函数体开始。
	{
// 算文件名的哈希。
		uint32 KeyHash = GetTypeHash(Filename);

// 旧文件大小，先置 0。
		int64 OldFileSize = 0;
// 代码块开始。
		{
// 加读锁。
			FReadScopeLock ScopeLock(FileSizesLock);
// 查旧大小。
			int64* pOldFileSize = FileSizes.FindByHash(KeyHash, Filename);
// 如果找到了。
			if (pOldFileSize)
// 条件体开始。
			{
// 保存旧大小。
				OldFileSize = *pOldFileSize;
// 条件体结束。
			}
// 代码块结束。
		}

// 尝试更新配额。
		EPersistentStorageManagerFileSizeFlags Result = TryUpdateQuota(OldFileSize, FileSize, Flags);
// 如果成功（无标志返回）。
		if (Result == EPersistentStorageManagerFileSizeFlags::None)
// 条件体开始。
		{
// 加写锁。
			FWriteScopeLock ScopeLock(FileSizesLock);
// 写入新大小。
			FileSizes.AddByHash(KeyHash, Filename, FileSize);

// 打日志：文件已加入分类。
			UE_LOG(LogPlatformFileManagedStorage, Verbose, TEXT("File %s is added to category %s"), *Filename, *CategoryName);
// 条件体结束。
		}

// 返回结果。
		return Result;
// 函数体结束。
	}

// RemoveFile：从分类里移除文件。
	bool RemoveFile(const FString& Filename)
// 函数体开始。
	{
// 手动加写锁（因为 FWriteScopeLock 不支持提前解锁）。
		FileSizesLock.WriteLock(); // FWriteScopeLock 不支持提前解锁 :(

// 旧大小。
		int64 OldSize;
// 从表里移除并取出旧大小。
		if (FileSizes.RemoveAndCopyValue(Filename, OldSize))
// 条件体开始。
		{
// 提前解锁。
			FileSizesLock.WriteUnlock();

// 已用配额减去旧大小。
			UsedQuota -= OldSize;

// 打日志：文件已从分类移除。
			UE_LOG(LogPlatformFileManagedStorage, Verbose, TEXT("File %s is removed from category %s"), *Filename, *CategoryName);

// 返回成功。
			return true;
// 条件体结束。
		}

// 解锁。
		FileSizesLock.WriteUnlock();
// 返回失败。
		return false;
// 函数体结束。
	}

// GetDirectories：取目录列表。
	const TArray<FString>& GetDirectories() const { return Directories; }
```

**本段新元素**

- `AddOrUpdateFile(Filename, FileSize, Flags)` —— 添加或更新文件大小
- `FReadScopeLock ScopeLock(FileSizesLock)` —— 读锁查旧大小
- `FileSizes.FindByHash(KeyHash, Filename)` —— 按哈希查
- `TryUpdateQuota(OldFileSize, FileSize, Flags)` —— 尝试更新配额
- `FWriteScopeLock ScopeLock(FileSizesLock)` —— 写锁更新表
- `FileSizes.AddByHash(KeyHash, Filename, FileSize)` —— 写入新大小
- `RemoveFile(Filename)` —— 从分类移除文件
- `FileSizesLock.WriteLock()` —— **手动加写锁**（因为 `FWriteScopeLock` 不支持提前解锁）
- `FileSizes.RemoveAndCopyValue(Filename, OldSize)` —— 移除并取出旧大小
- `UsedQuota -= OldSize` —— 已用配额减去

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 199~223 行 | `AddOrUpdateFile` |
| 第 225~243 行 | `RemoveFile` |
| 第 245 行 | `GetDirectories` |

> 类比：像仓库记账：进货时更新库存（AddOrUpdate），出货时扣减（Remove）。

### 九、`CategoryStat` + `GetStat`（第 247~276 行）

```cpp
// CategoryStat：分类统计快照。
	struct CategoryStat
	{
// Print：格式化成字符串。
		FString Print() const
// 函数体开始。
		{
// 如果总大小为负（无限配额）。
			if (TotalSize < 0)
// 条件体开始。
			{
// 返回「已用 / 无限」。
				return FString::Printf(TEXT("Category %s: %.3f MiB (%" INT64_FMT ") / unlimited used"), *CategoryName, (float)UsedSize / 1024.f / 1024.f, UsedSize);
// 条件体结束。
			}
// 否则。
			else
// 否则体开始。
			{
// 返回「已用 / 总量」。
				return FString::Printf(TEXT("Category %s: %.3f MiB (%" INT64_FMT ") / %.3f MiB used"), *CategoryName, (float)UsedSize / 1024.f / 1024.f, UsedSize, (float)TotalSize / 1024.f / 1024.f);
// 否则体结束。
			}
// 函数体结束。
		}

// CategoryName：分类名。
		FString CategoryName;
// UsedSize：已用大小。
		int64 UsedSize = 0;
// TotalSize：总配额（-1 表示无限）。
		int64 TotalSize = -1;
// TotalOptionalSize：可选配额。
		int64 TotalOptionalSize = 0;
// FileSizes：每个文件的大小。
		TMap<FString, int64> FileSizes;
// Directories：目录列表。
		TArray<FString> Directories;
// 结构体结束。
	};

// 说明：如果调用时分类正在被修改，结果可能不准确，但底层是线程安全的。
	// 注意：如果调用时分类正在被修改，结果可能不准确，但底层是线程安全的
// GetStat：取统计快照。
	CategoryStat GetStat() const
// 函数体开始。
	{
// 加读锁。
		FReadScopeLock ScopeLock(FileSizesLock);
// 构造快照返回。
		return CategoryStat{ CategoryName, UsedQuota, StorageQuota, OptionalStorageQuota, FileSizes, Directories };
// 函数体结束。
	}

// GetLockRegistry：取锁注册表。
	FManagedStorageFileLockRegistry& GetLockRegistry() { return FileLockRegistry; }
```

**本段新元素**

- `struct CategoryStat` —— 分类统计快照
- `Print()` —— 格式化成字符串（`%.3f MiB`）
- `INT64_FMT` —— int64 的格式串宏
- `CategoryName` / `UsedSize` / `TotalSize` / `TotalOptionalSize` —— 4 个字段
- `TMap<FString, int64> FileSizes` —— 每个文件的大小
- `TArray<FString> Directories` —— 目录列表
- `GetStat()` —— 加读锁后构造快照
- `GetLockRegistry()` —— 取锁注册表

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 247~267 行 | `CategoryStat` 结构 |
| 第 269~274 行 | `GetStat` |
| 第 276 行 | `GetLockRegistry` |

> 类比：像仓库盘点表：拍个快照，之后慢慢看。

### 十、`TryUpdateQuota`（CAS 循环）+ `ShouldManageFile`（第 278~341 行）

```cpp
// 私有区。
private:
// TryUpdateQuota：尝试更新配额（内部实现）。
	EPersistentStorageManagerFileSizeFlags TryUpdateQuota(const int64 OldFileSize, const int64 NewFileSize, EPersistentStorageManagerFileSizeFlags Flags)
// 函数体开始。
	{
// 断言：旧大小 >= 0。
		check(OldFileSize >= 0);
// 断言：新大小 >= 0。
		check(NewFileSize >= 0);

// 如果新大小 <= 旧大小（缩小或不变）。
		if (NewFileSize <= OldFileSize)
// 条件体开始。
		{
// 已用配额减去差值。
			UsedQuota -= (OldFileSize - NewFileSize);
// 返回无标志（成功）。
			return EPersistentStorageManagerFileSizeFlags::None;
// 条件体结束。
		}

// 如果带 OnlyUpdateIfLess 标志。
		if (EnumHasAnyFlags(Flags, EPersistentStorageManagerFileSizeFlags::OnlyUpdateIfLess))
// 条件体开始。
		{
// 返回该标志（表示不更新）。
			return EPersistentStorageManagerFileSizeFlags::OnlyUpdateIfLess;
// 条件体结束。
		}

// 如果带 RespectQuota 标志且配额 >= 0。
		if (EnumHasAnyFlags(Flags, EPersistentStorageManagerFileSizeFlags::RespectQuota) && StorageQuota >= 0)
// 条件体开始。
		{
// 旧已用配额。
			int64 OldUsedQuota = UsedQuota;
// 新已用配额。
			int64 NewUsedQuota;
// 循环体开始。
			do
// 代码块开始。
			{
// 算新已用配额。
				NewUsedQuota = OldUsedQuota + (NewFileSize - OldFileSize);
// 如果超过配额。
				if (NewUsedQuota > StorageQuota)
// 条件体开始。
				{
// 返回 RespectQuota（表示超配额）。
					return EPersistentStorageManagerFileSizeFlags::RespectQuota;
// 条件体结束。
				}
// CAS 循环：原子更新已用配额。
			} while (!UsedQuota.compare_exchange_weak(OldUsedQuota, NewUsedQuota));

// 返回无标志（成功）。
			return EPersistentStorageManagerFileSizeFlags::None;
// 条件体结束。
		}

// 无配额限制：直接原子加。
		UsedQuota += (NewFileSize - OldFileSize);
// 返回无标志（成功）。
		return EPersistentStorageManagerFileSizeFlags::None;
// 函数体结束。
	}

// ShouldManageFile：判断文件是否应被本分类管理。
	bool ShouldManageFile(const FString& Filename) const
// 函数体开始。
	{
// 遍历所有目录。
		for (const FString& Directory : Directories)
// 循环体开始。
		{
// 如果文件在该目录之下。
			if (ManagedStorageInternal::IsUnderDirectory(Filename, Directory))
// 条件体开始。
			{
// 返回 true。
				return true;
// 条件体结束。
			}
// 循环体结束。
		}

// 返回 false。
		return false;
// 函数体结束。
	}

// 私有区。
private:
// CategoryName：分类名（const）。
	const FString CategoryName;
// Directories：目录列表（const）。
	const TArray<FString> Directories;

// StorageQuota：总配额（-1 表示无限）。
	const int64 StorageQuota = -1;
// OptionalStorageQuota：可选配额。
	const int64 OptionalStorageQuota = 0;

// UsedQuota：已用配额（原子，多线程安全）。
	std::atomic<int64> UsedQuota{ 0 };

// FileSizes：每个文件的大小。
	TMap<FString, int64> FileSizes;
// FileSizesLock：保护 FileSizes 的读写锁（mutable 允许 const 方法加锁）。
	mutable FRWLock FileSizesLock;

// FileLockRegistry：文件锁注册表。
	FManagedStorageFileLockRegistry FileLockRegistry;
// 类结束。
};
```

**本段新元素**

- `TryUpdateQuota(OldFileSize, NewFileSize, Flags)` —— 尝试更新配额
- `if (NewFileSize <= OldFileSize)` —— 缩小或不变，直接减
- `EnumHasAnyFlags(Flags, OnlyUpdateIfLess)` —— 带该标志就返回（不更新）
- `EnumHasAnyFlags(Flags, RespectQuota) && StorageQuota >= 0` —— 需要查配额
- `do { ... } while (!UsedQuota.compare_exchange_weak(...))` —— **CAS 循环**（无锁原子更新）
- `if (NewUsedQuota > StorageQuota) return RespectQuota` —— 超配额
- `UsedQuota += (NewFileSize - OldFileSize)` —— 无配额限制时直接原子加
- `ShouldManageFile(Filename)` —— 遍历目录判断是否属于本分类
- `std::atomic<int64> UsedQuota{ 0 }` —— 已用配额（原子）
- `mutable FRWLock FileSizesLock` —— 读写锁（`mutable` 允许 const 方法加锁）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 279~288 行 | 缩小分支 |
| 第 290~293 行 | `OnlyUpdateIfLess` 分支 |
| 第 295~309 行 | CAS 循环（查配额） |
| 第 311~313 行 | 无配额限制分支 |
| 第 315~326 行 | `ShouldManageFile` |
| 第 328~341 行 | 8 个成员 |

> 类比：像抢购：先看库存够不够（CAS 循环），够就扣，不够就拒。

### 十一、`FPersistentStorageManager` 声明 + `IsReady` + `Initialize`（第 343~422 行）

```cpp
// 说明：整个类不加 CORE_API，因为那样会导出 FCategoryInfo，进而强制生成 FPersistentStorageCategory 的拷贝构造，
// 注意：整个类不加 CORE_API，因为那样会导出 FCategoryInfo，进而强制生成 FPersistentStorageCategory 的拷贝构造，
// 而 std::atomic 不可拷贝，会导致编译错误。
// 而 std::atomic 不可拷贝，会导致编译错误。
// FPersistentStorageManager：持久化存储管理器（单例）。
class FPersistentStorageManager
{
// 公有区。
public:
// IsReady：判断管理器是否可用。
	static bool IsReady()
// 函数体开始。
	{
// 说明：FPersistentStorageManager 依赖 FPaths，而 FPaths 依赖命令行已初始化。
		// FPersistentStorageManager 依赖 FPaths，而 FPaths 依赖命令行已初始化。
// 说明：FPersistentStorageManager 依赖 GConfig。
		// FPersistentStorageManager 依赖 GConfig。
// 说明：依赖就绪前不能构造。
		// 依赖就绪前不能构造。
// 说明：崩溃时会尝试分配内存，但可能卡在日志刷新上。
		// 崩溃时会尝试分配内存，但可能卡在日志刷新上。
// 返回：配置就绪 && 命令行已初始化 && 无致命错误。
		return GConfig && GConfig->IsReadyForUse() && FCommandLine::IsInitialized() && !GIsCriticalError;
// 函数体结束。
	}

// 说明：单例访问。
	/** 单例访问 **/
// Get：取单例（实现在 .cpp）。
	static CORE_API FPersistentStorageManager& Get();

// 构造函数（实现在 .cpp）。
	FPersistentStorageManager();

// Initialize：初始化（扫描所有根目录，把已有文件登记进分类）。
	void Initialize()
// 函数体开始。
	{
// 如果已初始化。
		if (bInitialized)
// 条件体开始。
		{
// 直接返回。
			return;
// 条件体结束。
		}

// 说明：检查并添加文件。
		// 检查并添加文件
// 起一个后台任务做扫描。
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []()
// lambda 体开始。
		{
// FInitStorageVisitor：初始化扫描访问者。
			class FInitStorageVisitor : public IPlatformFile::FDirectoryVisitor
// 类声明开始。
			{
// 公有区。
			public:
// 构造函数：标记为线程安全（允许并行扫描）。
				FInitStorageVisitor() : IPlatformFile::FDirectoryVisitor(EDirectoryVisitorFlags::ThreadSafe) // 并行扫描，范围放宽
// 构造函数体开始。
				{}
// 构造函数体结束。

// Visit：访问一个文件或目录。
				virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
// 函数体开始。
				{
// 如果是文件（不是目录）。
					if (FilenameOrDirectory && !bIsDirectory)
// 条件体开始。
					{
// 取管理器单例。
						FPersistentStorageManager& Man = FPersistentStorageManager::Get();
// 尝试托管该文件。
						if (FPersistentManagedFile File = Man.TryManageFile(FilenameOrDirectory))
// 条件体开始。
						{
// 加文件锁。
							FManagedStorageScopeFileLock ScopeFileLock(File);

// 说明：必须在锁内做，因为其他线程可能正在修改或删除该文件。
							// 必须在锁内做，因为其他线程可能正在修改或删除该文件
// 取文件元数据。
							FFileStatData StatData = IPlatformFile::GetPlatformPhysical().GetStatData(FilenameOrDirectory);
// 如果元数据有效。
							if (ensureAlways(StatData.bIsValid))
// 条件体开始。
							{
// 如果文件大小无效。
								if (!ensureAlways(StatData.FileSize >= 0))
// 条件体开始。
								{
// 打错误日志。
									UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Invalid File Size for %s!"), FilenameOrDirectory);
// 条件体结束。
								}
								
// 登记文件大小。
								Man.AddOrUpdateFile(File, StatData.FileSize);
// 条件体结束。
							}
// 否则。
							else
// 否则体开始。
							{
// 打错误日志：元数据无效。
								UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Invalid Stat Data for %s!"), FilenameOrDirectory);
// 否则体结束。
							}
// 条件体结束。
						}
// 条件体结束。
					}

// 返回 true（继续遍历）。
					return true;
// 函数体结束。
				}
// 类结束。
			};

// 创建访问者实例。
			FInitStorageVisitor Visitor;

// 遍历所有根目录。
			for (const FString& RootDir : FPersistentStorageManager::Get().GetRootDirectories())
// 循环体开始。
			{
// 打日志：正在扫描目录。
				UE_LOG(LogPlatformFileManagedStorage, Display, TEXT("Scan directory %s"), *RootDir);

// 递归遍历该目录。
				IPlatformFile::GetPlatformPhysical().IterateDirectoryRecursively(*RootDir, Visitor);
// 循环体结束。
			}

// 打日志：扫描完成。
			UE_LOG(LogPlatformFileManagedStorage, Display, TEXT("Done scanning root directories"));
// lambda 体结束。
		});

// 标记已初始化。
		bInitialized = true;
// 函数体结束。
	}
```

**本段新元素**

- `class FPersistentStorageManager` —— 持久化存储管理器（单例）
- **为什么整个类不加 `CORE_API`** —— 会导出 `FCategoryInfo`，强制生成 `FPersistentStorageCategory` 拷贝构造，而 `std::atomic` 不可拷贝
- `IsReady()` —— 三重检查：`GConfig` 就绪 + 命令行已初始化 + 无致命错误
- `static Get()` —— 取单例（实现在 .cpp）
- `Initialize()` —— 初始化（起后台任务扫描所有根目录）
- `AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [](){...})` —— 后台任务
- `FInitStorageVisitor` —— 初始化扫描访问者（`EDirectoryVisitorFlags::ThreadSafe` 允许并行）
- `IPlatformFile::GetPlatformPhysical().IterateDirectoryRecursively(...)` —— 递归扫描
- `FManagedStorageScopeFileLock ScopeFileLock(File)` —— 加文件锁
- `ensureAlways(StatData.bIsValid)` —— 断言元数据有效

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 343~347 行 | 类声明 + 说明 |
| 第 348~355 行 | `IsReady` |
| 第 357~360 行 | `Get` / 构造 |
| 第 362~367 行 | `Initialize` 开头 |
| 第 370~419 行 | 后台扫描任务 |
| 第 421~422 行 | 标记已初始化 |

> 类比：像仓库管理员上班第一件事：把所有货架扫一遍，登记现有库存。

### 十二、`TryManageFile` ×2 + `TryManageFileInternal`（第 424~463 行）

```cpp
// TryManageFile：尝试托管文件（const FString& 版）。
	FPersistentManagedFile TryManageFile(const FString& Filename)
// 函数体开始。
	{
// 输出文件。
		FPersistentManagedFile OutFile;
// 转成完整路径。
		OutFile.FullFilename = FPaths::ConvertRelativePathToFull(Filename);

// 内部实现：判断属于哪个分类。
		TryManageFileInternal(OutFile);

// 返回结果。
		return OutFile;
// 函数体结束。
	}

// TryManageFile：尝试托管文件（FString&& 版，移动语义）。
	FPersistentManagedFile TryManageFile(FString&& Filename)
// 函数体开始。
	{
// 输出文件。
		FPersistentManagedFile OutFile;
// 转成完整路径（移动）。
		OutFile.FullFilename = FPaths::ConvertRelativePathToFull(MoveTemp(Filename));

// 内部实现。
		TryManageFileInternal(OutFile);

// 返回结果。
		return OutFile;
// 函数体结束。
	}

// 私有区。
private:
// TryManageFileInternal：判断文件属于哪个分类。
	void TryManageFileInternal(FPersistentManagedFile& OutFile)
// 函数体开始。
	{
// 分类索引从 0 开始。
		int32 CategoryIndex = 0;
// 遍历所有分类。
		for (const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 如果文件属于该分类。
			if (Category.IsInCategory(OutFile.FullFilename))
// 条件体开始。
			{
// 记录分类索引。
				OutFile.Category = CategoryIndex;
// 直接返回。
				return;
// 条件体结束。
			}
// 索引 +1。
			++CategoryIndex;
// 循环体结束。
		}

// 检查是否在某个根目录之下。
		bool bIsUnderRootDir = !!Algo::FindByPredicate(RootDirectories.GetRootDirectories(), [&OutFile](const FString& RootDir) { return ManagedStorageInternal::IsUnderDirectory(OutFile.FullFilename, RootDir); });
// 如果在根目录下。
		if (bIsUnderRootDir)
// 条件体开始。
		{
// 归入默认分类。
			OutFile.Category = Categories.GetDefaultCategoryIndex();
// 条件体结束。
		}
// 函数体结束。
	}
```

**本段新元素**

- `TryManageFile(const FString&)` —— 尝试托管（拷贝版）
- `TryManageFile(FString&&)` —— 尝试托管（移动版）
- `FPaths::ConvertRelativePathToFull(...)` —— 转完整路径
- `TryManageFileInternal(OutFile)` —— 内部实现：判断属于哪个分类
- 遍历所有分类，`Category.IsInCategory(...)` 命中就记录索引
- `Algo::FindByPredicate(RootDirectories, lambda)` —— 查是否在某个根目录下
- `OutFile.Category = Categories.GetDefaultCategoryIndex()` —— 归入默认分类

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 424~432 行 | `TryManageFile`（拷贝版） |
| 第 434~442 行 | `TryManageFile`（移动版） |
| 第 445~463 行 | `TryManageFileInternal` |

> 类比：像给新货分类：先看属于哪个专区，不属于任何专区就放「默认区」。

### 十三、10 个公开方法（第 465~606 行）

```cpp
// 公有区。
public:
// AddOrUpdateFile：添加或更新文件大小。
	EPersistentStorageManagerFileSizeFlags AddOrUpdateFile(const FPersistentManagedFile& File, const int64 FileSize, 
// 参数 Flags：默认无标志。
		EPersistentStorageManagerFileSizeFlags Flags = EPersistentStorageManagerFileSizeFlags::None)
// 函数体开始。
	{
// 如果文件无效。
		if (!File)
// 条件体开始。
		{
// 返回无标志。
			return EPersistentStorageManagerFileSizeFlags::None;
// 条件体结束。
		}

// 转调对应分类的 AddOrUpdateFile。
		return Categories.GetCategories()[File.Category].AddOrUpdateFile(File.FullFilename, FileSize, Flags);
// 函数体结束。
	}

// RemoveFileFromManager：从管理器移除文件。
	bool RemoveFileFromManager(FPersistentManagedFile& File)
// 函数体开始。
	{
// 如果文件无效。
		if (!File)
// 条件体开始。
		{
// 返回 false。
			return false;
// 条件体结束。
		}

// 转调对应分类的 RemoveFile。
		return Categories.GetCategories()[File.Category].RemoveFile(File.FullFilename);
// 函数体结束。
	}

// GetTotalUsedSize：取所有分类的总已用大小。
	int64 GetTotalUsedSize() const
// 函数体开始。
	{
// 累加器。
		int64 TotalUsedSize = 0LL;
// 遍历所有分类。
		for(const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 累加已用大小。
			TotalUsedSize += Category.GetUsedSize();
// 循环体结束。
		}

// 返回总和。
		return TotalUsedSize;
// 函数体结束。
	}

// 说明：查询指定路径的持久化存储使用情况。
	/// <summary>
	/// 查询指定路径的持久化存储使用情况
	/// </summary>
// 参数 Path：要检查的路径。
	/// <param name="Path">要检查的路径</param>
// 参数 UsedSpace：已用空间（输出）。
	/// <param name="UsedSpace">已用空间</param>
// 参数 RemainingSpace：剩余空间（输出）。
	/// <param name="RemainingSpace">剩余空闲空间</param>
// 参数 Quota：该分类的总配额（输出）。
	/// <param name="Quota">该分类分配的总存储</param>
// 参数 OptionalQuota：可选配额（输出，可为空）。
	/// <param name="OptionalQuota">可选配额（总配额的一部分，永远小于总配额）</param>
// 返回：成功返回 true。
	/// <returns>成功返回 true</returns>
// GetPersistentStorageUsage：按路径查询使用情况。
	bool GetPersistentStorageUsage(FString Path, int64& UsedSpace, int64 &RemainingSpace, int64& Quota, int64* OptionalQuota = nullptr)
// 函数体开始。
	{
// 转成完整路径。
		FString FullPath = FPaths::ConvertRelativePathToFull(MoveTemp(Path));

// 遍历所有分类。
		for (const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 如果路径属于该分类。
			if (Category.IsInCategory(FullPath))
// 条件体开始。
			{
// 输出已用大小。
				UsedSpace = Category.GetUsedSize();
// 输出剩余大小。
				RemainingSpace = Category.GetAvailableSize();
// 输出配额。
				Quota = Category.GetCategoryQuota();
// 如果调用方要可选配额。
				if (OptionalQuota != nullptr)
// 条件体开始。
				{
// 输出可选配额。
					*OptionalQuota = Category.GetCategoryOptionalQuota();
// 条件体结束。
				}
// 返回成功。
				return true;
// 条件体结束。
			}
// 循环体结束。
		}
// 返回失败。
		return false;
// 函数体结束。
	}

// GetPersistentStorageUsageByCategory：按分类名查询使用情况。
	bool GetPersistentStorageUsageByCategory(const FString& InCategory, int64& UsedSpace, int64& RemainingSpace, int64& Quota, int64* OptionalQuota = nullptr)
// 函数体开始。
	{
// 按名字查找分类。
		FPersistentStorageCategory* Category = Algo::FindBy(Categories.GetCategories(), InCategory, [](const FPersistentStorageCategory& Cat) { return Cat.GetCategoryName(); });
// 如果找到了。
		if (Category)
// 条件体开始。
		{
// 输出已用大小。
			UsedSpace = Category->GetUsedSize();
// 输出剩余大小。
			RemainingSpace = Category->GetAvailableSize();
// 输出配额。
			Quota = Category->GetCategoryQuota();
// 如果调用方要可选配额。
			if (OptionalQuota != nullptr)
// 条件体开始。
			{
// 输出可选配额。
				*OptionalQuota = Category->GetCategoryOptionalQuota();
// 条件体结束。
			}
// 返回成功。
			return true;
// 条件体结束。
		}
// 返回失败。
		return false;
// 函数体结束。
	}

// 说明：返回所需的额外空闲空间和可选空闲空间。
	/// <summary>
	/// 返回所需的额外空闲空间和可选空闲空间
	/// </summary>
// 参数 RequiredSpace：运行所需的持久化存储空间。
	/// <param name="RequiredSpace">运行所需的持久化存储空间</param>
// 参数 OptionalSpace：标记为可选的持久化存储分类。
	/// <param name="OptionalSpace">标记为可选的持久化存储分类</param>
	/// <returns></returns>
// GetPersistentStorageSize：取总大小信息。
	bool GetPersistentStorageSize(int64& UsedSpace, int64& RequiredSpace, int64& OptionalSpace) const
// 函数体开始。
	{
// 遍历所有分类。
		for (const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 累加已用大小。
			UsedSpace += Category.GetUsedSize();
// 累加所需配额。
			RequiredSpace += Category.GetCategoryQuota();
// 累加可选配额。
			OptionalSpace += Category.GetCategoryOptionalQuota();
// 循环体结束。
		}
// 返回 true。
		return true;
// 函数体结束。
	}

// IsInitialized：是否已初始化。
	bool IsInitialized() const
// 函数体开始。
	{
// 返回标志。
		return bInitialized;
// 函数体结束。
	}

// IsCategoryForFileFull：判断文件所属分类是否已满。
	bool IsCategoryForFileFull(const FPersistentManagedFile& File) const
// 函数体开始。
	{
// 如果文件无效。
		if (!File)
// 条件体开始。
		{
// 返回 false。
			return false;
// 条件体结束。
		}

// 转调分类的 IsCategoryFull。
		return Categories.GetCategories()[File.Category].IsCategoryFull();
// 函数体结束。
	}

// GenerateCategoryStats：生成所有分类的统计快照。
	TMap<FString, FPersistentStorageCategory::CategoryStat> GenerateCategoryStats() const
// 函数体开始。
	{
// 结果表。
		TMap<FString, FPersistentStorageCategory::CategoryStat> CategoryStats;
// 预留空间。
		CategoryStats.Reserve(Categories.GetCategories().Num());

// 遍历所有分类。
		for (const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 加入统计。
			CategoryStats.Add(Category.GetCategoryName(), Category.GetStat());
// 循环体结束。
		}

// 返回结果。
		return CategoryStats;
// 函数体结束。
	}

// GetCategoryStat：取指定分类的统计快照。
	TOptional<FPersistentStorageCategory::CategoryStat> GetCategoryStat(const FString& InCategory) const
// 函数体开始。
	{
// 结果（可选）。
		TOptional<FPersistentStorageCategory::CategoryStat> Result;

// 遍历所有分类。
		for (const FPersistentStorageCategory& Category : Categories.GetCategories())
// 循环体开始。
		{
// 如果名字匹配。
			if (Category.GetCategoryName() == InCategory)
// 条件体开始。
			{
// 填入统计。
				Result.Emplace(Category.GetStat());
// 跳出循环。
				break;
// 条件体结束。
			}
// 循环体结束。
		}

// 返回结果。
		return Result;
// 函数体结束。
	}

// GetRootDirectories：取所有根目录。
	TArrayView<const FString> GetRootDirectories() const { return RootDirectories.GetRootDirectories(); }
```

**本段新元素**

- `AddOrUpdateFile(File, FileSize, Flags)` —— 添加或更新文件大小
- `RemoveFileFromManager(File)` —— 从管理器移除文件
- `GetTotalUsedSize()` —— 所有分类的总已用大小
- `GetPersistentStorageUsage(Path, Used, Remaining, Quota, OptionalQuota)` —— 按路径查使用情况
- `GetPersistentStorageUsageByCategory(...)` —— 按分类名查
- `GetPersistentStorageSize(Used, Required, Optional)` —— 取总大小信息
- `IsInitialized()` —— 是否已初始化
- `IsCategoryForFileFull(File)` —— 文件所属分类是否已满
- `GenerateCategoryStats()` —— 生成所有分类的统计快照
- `GetCategoryStat(InCategory)` —— 取指定分类的统计（`TOptional`）
- `GetRootDirectories()` —— 取所有根目录（`TArrayView`）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 466~475 行 | `AddOrUpdateFile` |
| 第 477~485 行 | `RemoveFileFromManager` |
| 第 487~496 行 | `GetTotalUsedSize` |
| 第 498~526 行 | `GetPersistentStorageUsage` |
| 第 528~543 行 | `GetPersistentStorageUsageByCategory` |
| 第 545~560 行 | `GetPersistentStorageSize` |
| 第 562~575 行 | `IsInitialized` / `IsCategoryForFileFull` |
| 第 577~604 行 | `GenerateCategoryStats` / `GetCategoryStat` |
| 第 606 行 | `GetRootDirectories` |

> 类比：像仓库的 10 个查询窗口：查总量、查分类、查路径、查是否满……

### 十四、私有成员：`FRootDirInfo` + `FCategoryInfo`（第 608~647 行）

```cpp
// 私有区。
private:
// 友元：FManagedStorageScopeFileLock 可访问 Categories。
	friend class FManagedStorageScopeFileLock; // For access to Categories

// bInitialized：是否已初始化。
	bool bInitialized;

// 说明：所有托管存储的顶层目录。
	// 所有托管存储的顶层目录
// 说明：包装类，防止初始化后被修改或扩容。
	// 包装类，防止初始化后被修改或扩容
// FRootDirInfo：根目录信息。
	struct FRootDirInfo
	{
// 私有区。
	private:
// RootDirectories：根目录列表。
		TArray<FString> RootDirectories;

// 公有区。
	public:
// Init：初始化（实现在 .cpp）。
		void Init(TArrayView<const FPersistentStorageCategory> Categories);
// GetRootDirectories：取根目录视图。
		TArrayView<const FString> GetRootDirectories() const { return MakeArrayView(RootDirectories); }
// 结构体结束。
	};
// RootDirectories：根目录信息实例。
	FRootDirInfo RootDirectories;

// 说明：包装分类数组，防止初始化后扩容。
	// 包装分类数组，防止初始化后扩容
// FCategoryInfo：分类信息。
	struct FCategoryInfo
	{
// 私有区。
	private:
// Categories：分类数组。
		TArray<FPersistentStorageCategory> Categories;
// DefaultCategory：默认分类索引（-1 表示无）。
		int32 DefaultCategory = -1;

// 公有区。
	public:
// 构造函数（实现在 .cpp）。
		FCategoryInfo(TArray<FPersistentStorageCategory>&& InCategories, int32 InDefaultCategory);

// GetCategories（非 const 版）：取分类视图。
		TArrayView<FPersistentStorageCategory> GetCategories() { return MakeArrayView(Categories); }
// GetCategories（const 版）：取分类视图。
		TArrayView<const FPersistentStorageCategory> GetCategories() const { return MakeArrayView(Categories); }

// GetDefaultCategory（非 const 版）：取默认分类。
		FPersistentStorageCategory* GetDefaultCategory() { return DefaultCategory >= 0 ? &Categories[DefaultCategory] : nullptr; }
// GetDefaultCategory（const 版）：取默认分类。
		const FPersistentStorageCategory* GetDefaultCategory() const { return DefaultCategory >= 0 ? &Categories[DefaultCategory] : nullptr; }

// GetDefaultCategoryIndex：取默认分类索引。
		int32 GetDefaultCategoryIndex() const { return DefaultCategory; }
// 结构体结束。
	};
// Categories：分类信息实例。
	FCategoryInfo Categories;

// InitCategories：初始化分类（实现在 .cpp）。
	static FCategoryInfo InitCategories();
// 类结束。
};
```

**本段新元素**

- `friend class FManagedStorageScopeFileLock` —— 友元（访问 `Categories`）
- `bool bInitialized` —— 是否已初始化
- `struct FRootDirInfo` —— 根目录信息（**包装类，防止初始化后修改**）
- `Init(TArrayView<const FPersistentStorageCategory>)` —— 初始化（实现在 .cpp）
- `GetRootDirectories()` —— 返回 `TArrayView`（只读视图）
- `struct FCategoryInfo` —— 分类信息（**包装类，防止初始化后扩容**）
- `FCategoryInfo(TArray<FPersistentStorageCategory>&&, int32)` —— 构造（实现在 .cpp）
- `GetCategories()` —— 返回 `TArrayView`（2 个重载）
- `GetDefaultCategory()` —— 取默认分类（2 个重载）
- `GetDefaultCategoryIndex()` —— 取默认分类索引
- `static InitCategories()` —— 初始化分类（实现在 .cpp）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 608~611 行 | 友元 + `bInitialized` |
| 第 613~624 行 | `FRootDirInfo` |
| 第 626~644 行 | `FCategoryInfo` |
| 第 646~647 行 | `InitCategories` + 类结束 |

> 类比：像仓库的「货架布局图」：初始化后就不能改，防止运行中有人乱动。

### 十五、`FManagedStorageFileWriteHandle`（第 649~821 行）

```cpp
// 说明：只写句柄（读操作不追踪）。
// 只写句柄
// FManagedStorageFileWriteHandle：托管存储的写句柄。
class FManagedStorageFileWriteHandle : public IFileHandle
{
// 私有区。
private:
// IsReady：判断管理器是否可用。
	static bool IsReady()
// 函数体开始。
	{
// 说明：管理器就绪前，FManagedStoragePlatformFile 会直接透传给下一层。
		// FManagedStoragePlatformFile will just pass through to LowerLevel until FPersistentStorageManager is ready
// 转调管理器的 IsReady。
		return FPersistentStorageManager::IsReady();
// 函数体结束。
	}

// TryManageFile：尝试托管本文件。
	bool TryManageFile()
// 函数体开始。
	{
// 如果还没托管且管理器就绪。
		if (!File && IsReady())
// 条件体开始。
		{
// 尝试托管。
			File = FPersistentStorageManager::Get().TryManageFile(File.FullFilename);
// 条件体结束。
		}

// 返回是否有效。
		return File.IsValid();
// 函数体结束。
	}

// 公有区。
public:
// 构造函数：保存底层句柄和托管文件。
	FManagedStorageFileWriteHandle(IFileHandle* InFileHandle, FPersistentManagedFile InFile)
// 初始化列表：保存底层句柄。
		: FileHandle(InFileHandle)
// 初始化列表：移动构造托管文件。
		, File(MoveTemp(InFile))
// 函数体开始。
	{
// 函数体结束。
	}

// 析构函数：更新文件大小。
	virtual ~FManagedStorageFileWriteHandle()
// 函数体开始。
	{
// 如果托管失败。
		if (!TryManageFile())
// 条件体开始。
		{
// 直接返回。
			return;
// 条件体结束。
		}

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(File);

// 更新文件大小。
		FPersistentStorageManager::Get().AddOrUpdateFile(File, FileHandle->Size());
// 函数体结束。
	}

// Tell：转调底层。
	virtual int64 Tell() override
// 函数体开始。
	{
// 转调。
		return FileHandle->Tell();
// 函数体结束。
	}

// Seek：转调底层。
	virtual bool Seek(int64 NewPosition) override
// 函数体开始。
	{
// 转调。
		return FileHandle->Seek(NewPosition);
// 函数体结束。
	}

// SeekFromEnd：转调底层。
	virtual bool SeekFromEnd(int64 NewPositionRelativeToEnd = 0) override
// 函数体开始。
	{
// 转调。
		return FileHandle->SeekFromEnd(NewPositionRelativeToEnd);
// 函数体结束。
	}

// Read：转调底层。
	virtual bool Read(uint8* Destination, int64 BytesToRead) override
// 函数体开始。
	{
// 转调。
		return FileHandle->Read(Destination, BytesToRead);
// 函数体结束。
	}

// ReadAt：转调底层。
	virtual bool ReadAt(uint8* Destination, int64 BytesToRead, int64 Offset) override
// 函数体开始。
	{
// 转调。
		return FileHandle->ReadAt(Destination, BytesToRead, Offset);
// 函数体结束。
	}

// Write：写数据（带配额检查）。
	virtual bool Write(const uint8* Source, int64 BytesToWrite) override
// 函数体开始。
	{
// 如果托管失败。
		if (!TryManageFile())
// 条件体开始。
		{
// 直接转调底层（不追踪）。
			return FileHandle->Write(Source, BytesToWrite);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(File);

// 算写入后的新大小。
		int64 NewSize = FMath::Max(FileHandle->Tell() + BytesToWrite, FileHandle->Size());

// 尝试更新配额（遵守配额限制）。
		EPersistentStorageManagerFileSizeFlags Result = Manager.AddOrUpdateFile(File, NewSize, EPersistentStorageManagerFileSizeFlags::RespectQuota);
// 如果返回非 None 说明超配额。
		bool bIsFileCategoryFull = Result != EPersistentStorageManagerFileSizeFlags::None;
// 如果分类已满。
		if (bIsFileCategoryFull)
// 条件体开始。
		{
// 打错误日志。
			UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Failed to write to file %s.  The category of the file has reach quota limit in peristent storage."), *File.FullFilename);
// 返回失败。
			return false;
// 条件体结束。
		}

// 真正写入。
		bool bSuccess = FileHandle->Write(Source, BytesToWrite);
// 如果写入失败。
		if (!bSuccess)
// 条件体开始。
		{
// 取实际文件大小。
			int64 FileSize = FileHandle->Size();
// 如果大小有效。
			if (ensureAlways(FileSize >= 0))
// 条件体开始。
			{
// 回滚配额（用实际大小更新）。
				verify(Manager.AddOrUpdateFile(File, FileSize) == EPersistentStorageManagerFileSizeFlags::None);
// 条件体结束。
			}
// 条件体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}

// Size：转调底层。
	virtual int64 Size() override
// 函数体开始。
	{
// 转调。
		return FileHandle->Size();
// 函数体结束。
	}

// Flush：刷新（带大小更新）。
	virtual bool Flush(const bool bFullFlush = false) override
// 函数体开始。
	{
// 记录刷新前是否已托管。
		const bool bOldIsValid = File.IsValid();
// 如果托管失败。
		if (!TryManageFile())
// 条件体开始。
		{
// 直接转调底层。
			return FileHandle->Flush(bFullFlush);
// 条件体结束。
		}

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(File);

// 真正刷新。
		const bool bSuccess = FileHandle->Flush(bFullFlush);
// 如果之前没托管，强制更新大小。
		const bool bForceSizeUpdate = !bOldIsValid;
// 如果刷新失败或需要强制更新。
		if (!bSuccess || bForceSizeUpdate)
// 条件体开始。
		{
// 取实际文件大小。
			int64 FileSize = FileHandle->Size();
// 如果大小有效。
			if (ensureAlways(FileSize >= 0))
// 条件体开始。
			{
// 更新配额。
				verify(FPersistentStorageManager::Get().AddOrUpdateFile(File, FileSize) == EPersistentStorageManagerFileSizeFlags::None);
// 条件体结束。
			}
// 条件体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}

// Truncate：截断（带配额检查）。
	virtual bool Truncate(int64 NewSize) override
// 函数体开始。
	{
// 如果托管失败。
		if (!TryManageFile())
// 条件体开始。
		{
// 直接转调底层。
			return FileHandle->Truncate(NewSize);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(File);

// 取当前文件大小。
		int64 FileSize = FileHandle->Size();

// 如果新大小 <= 当前大小（缩小）。
		if (NewSize <= FileSize)
// 条件体开始。
		{
// 真正截断。
			bool bSuccess = FileHandle->Truncate(NewSize);
// 重新取大小。
			FileSize = FileHandle->Size();
// 更新配额（缩小不需要检查）。
			verify(Manager.AddOrUpdateFile(File, FileSize) == EPersistentStorageManagerFileSizeFlags::None);
// 返回结果。
			return bSuccess;
// 条件体结束。
		}

// 如果扩大且超配额。
		if (Manager.AddOrUpdateFile(File, NewSize, EPersistentStorageManagerFileSizeFlags::RespectQuota) != EPersistentStorageManagerFileSizeFlags::None)
// 条件体开始。
		{
// 打错误日志。
			UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Failed to truncate file %s.  The category of the file has reach quota limit in peristent storage."), *File.FullFilename);
// 返回失败。
			return false;
// 条件体结束。
		}

// 真正截断。
		if (!FileHandle->Truncate(NewSize))
// 条件体开始。
		{
// 重新取大小。
			FileSize = FileHandle->Size();
// 回滚配额。
			verify(Manager.AddOrUpdateFile(File, FileSize) == EPersistentStorageManagerFileSizeFlags::None);
// 返回失败。
			return false;
// 条件体结束。
		}

// 返回成功。
		return true;
// 函数体结束。
	}

// ShrinkBuffers：转调底层。
	virtual void ShrinkBuffers() override
// 函数体开始。
	{
// 转调。
		FileHandle->ShrinkBuffers();
// 函数体结束。
	}

// 私有区。
private:
// FileHandle：底层文件句柄。
	TUniquePtr<IFileHandle>	FileHandle;
// File：托管文件信息。
	FPersistentManagedFile	File;
// 类结束。
};
```

**本段新元素**

- `class FManagedStorageFileWriteHandle : public IFileHandle` —— 托管存储的写句柄
- `IsReady()` —— 转调管理器的 `IsReady`
- `TryManageFile()` —— 尝试托管本文件（懒托管）
- 构造函数：保存底层句柄 + 托管文件
- 析构函数：**更新文件大小**（`AddOrUpdateFile(File, FileHandle->Size())`）
- `Tell` / `Seek` / `SeekFromEnd` / `Read` / `ReadAt` / `Size` / `ShrinkBuffers` —— 7 个纯转调
- `Write(Source, BytesToWrite)` —— **写前查配额**
- `FMath::Max(FileHandle->Tell() + BytesToWrite, FileHandle->Size())` —— 算写入后新大小
- `AddOrUpdateFile(File, NewSize, RespectQuota)` —— 查配额
- `if (bIsFileCategoryFull)` —— 超配额就返回 false
- `verify(Manager.AddOrUpdateFile(File, FileSize) == None)` —— 写失败时回滚配额
- `Flush(bFullFlush)` —— 刷新 + 大小更新
- `Truncate(NewSize)` —— 截断 + 配额检查（扩大时查）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 649~667 行 | 类声明 + `IsReady` / `TryManageFile` |
| 第 669~686 行 | 构造 / 析构 |
| 第 688~711 行 | 5 个纯转调 |
| 第 713~745 行 | `Write`（查配额） |
| 第 747~750 行 | `Size` |
| 第 752~774 行 | `Flush` |
| 第 776~811 行 | `Truncate` |
| 第 813~821 行 | `ShrinkBuffers` + 成员 |

> 类比：像带限额的信用卡：刷之前先查额度，超了就拒。

### 十六、`TManagedStoragePlatformFile<BaseClass>` 构造（第 823~856 行）

```cpp
// 说明：这里用模板而不是多态包装，因为很多代码期望物理层不是包装器。
// 注意：这里用模板而不是多态包装，因为很多代码期望物理层不是包装器。
// 好处是 IPlatformFile 新增函数时不需要更新这里。
// 好处是 IPlatformFile 新增函数时不需要更新这里。
// TManagedStoragePlatformFile：模板化的托管存储平台文件。
template<class BaseClass>
// 继承 BaseClass（通常是平台文件实现）。
class TManagedStoragePlatformFile : public BaseClass
{
// 私有区。
private:
// IsReady：判断管理器是否可用。
	static bool IsReady()
// 函数体开始。
	{
// 说明：管理器就绪前，直接透传给下一层。
		// FManagedStoragePlatformFile will just pass through to LowerLevel until FPersistentStorageManager is ready
// 转调管理器的 IsReady。
		return FPersistentStorageManager::IsReady();
// 函数体结束。
	}

// 公有区。
public:

// 构造函数：校验基类实现。
	TManagedStoragePlatformFile() : BaseClass()
// 函数体开始。
	{
// 说明：这里用 LowLevelFatalError 因为静态初始化期 UE_LOG 还不可用。
		// 注意：这里用 LowLevelFatalError 因为静态初始化期 UE_LOG 还不可用

// 说明：记账由基类实现调用 DeleteFile() 完成。
		// 记账由基类实现调用 DeleteFile() 完成
// 判断基类是否用通用实现。
		bool bUsingGenericDeleteDirectoryRecursively = (&BaseClass::DeleteDirectoryRecursively == &IPlatformFile::DeleteDirectoryRecursively);
// 断言：必须用通用实现。
		check(bUsingGenericDeleteDirectoryRecursively);
// 如果不是。
		if (!bUsingGenericDeleteDirectoryRecursively)
// 条件体开始。
		{
// 致命错误：无法追踪所有删除。
			LowLevelFatalError(TEXT("TManagedStoragePlatformFile cannot track all deletes!"));
// 条件体结束。
		}

// 说明：记账由 DeleteFile() 和 CopyFile() 完成，基类实现会调用它们。
		// 记账由 DeleteFile() 和 CopyFile() 完成，基类实现会调用它们
// 判断基类是否用通用实现。
		bool bUsingGenericCopyDirectoryTree = (&BaseClass::CopyDirectoryTree == &IPlatformFile::CopyDirectoryTree);
// 断言：必须用通用实现。
		check(bUsingGenericCopyDirectoryTree);
// 如果不是。
		if (!bUsingGenericCopyDirectoryTree)
// 条件体开始。
		{
// 致命错误：无法追踪所有删除和复制。
			LowLevelFatalError(TEXT("TManagedStoragePlatformFile cannot track all deletes and copies!"));
// 条件体结束。
		}
// 函数体结束。
	}
```

**本段新元素**

- **为什么用模板而不是多态包装** —— 很多代码期望物理层不是包装器
- `template<class BaseClass> class TManagedStoragePlatformFile : public BaseClass` —— 模板继承
- `IsReady()` —— 转调管理器的 `IsReady`
- 构造函数：**校验基类实现**
- `&BaseClass::DeleteDirectoryRecursively == &IPlatformFile::DeleteDirectoryRecursively` —— 函数指针比较
- `LowLevelFatalError(...)` —— 致命错误（静态初始化期 `UE_LOG` 不可用）
- **为什么校验** —— 记账依赖基类调用 `DeleteFile()` / `CopyFile()`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 823~826 行 | 说明 + 类声明 |
| 第 828~833 行 | `IsReady` |
| 第 837~847 行 | 校验 `DeleteDirectoryRecursively` |
| 第 849~856 行 | 校验 `CopyDirectoryTree` |

> 类比：像装监控前先确认：这栋楼的电路是不是标准接法，不是就报警。

### 十七、`DeleteFile` / `DeleteFiles`（第 858~914 行）

```cpp
// DeleteFile：删除文件（带记账）。
	virtual bool DeleteFile(const TCHAR* Filename) override
// 函数体开始。
	{
// 如果管理器未就绪。
		if (!IsReady())
// 条件体开始。
		{
// 直接转调基类。
			return BaseClass::DeleteFile(Filename);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 尝试托管该文件。
		FPersistentManagedFile ManagedFile = Manager.TryManageFile(Filename);

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(ManagedFile);

// 真正删除。
		bool bSuccess = BaseClass::DeleteFile(Filename);
// 如果成功。
		if (bSuccess)
// 条件体开始。
		{
// 从管理器移除。
			Manager.RemoveFileFromManager(ManagedFile);
// 条件体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}

// DeleteFiles：批量删除（带记账）。
	virtual bool DeleteFiles(const TArrayView<const TCHAR*>& Filenames) override
// 函数体开始。
	{
// 如果管理器未就绪。
		if (!IsReady())
// 条件体开始。
		{
// 直接转调基类。
			return BaseClass::DeleteFiles(Filenames);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 文件锁数组（每个文件一把锁）。
		TArray<TPair<FPersistentManagedFile, FManagedStorageScopeFileLock>> FileLocks;
// 预留空间。
		FileLocks.Reserve(Filenames.Num());
// 遍历所有文件名。
		for (const TCHAR* Filename : Filenames)
// 循环体开始。
		{
// 尝试托管。
			FPersistentManagedFile ManagedFile = Manager.TryManageFile(Filename);
// 加锁并保存。
			FileLocks.Emplace(ManagedFile, MoveTemp(ManagedFile));
// 循环体结束。
		}

// 真正批量删除。
		bool bSuccess = BaseClass::DeleteFiles(Filenames);

// 文件数量。
		const int32 Count = Filenames.Num();
// 遍历所有文件。
		for (int32 i = 0; i < Count; i++)
// 循环体开始。
		{
// 如果文件已不存在（删除成功）。
			if (!BaseClass::FileExists(Filenames[i]))
// 条件体开始。
			{
// 打日志：移除已删除文件。
				UE_LOG(LogPlatformFileManagedStorage, Display, TEXT("Removing deleted file %s."), *(FileLocks[i].Key.FullFilename));
// 从管理器移除。
				Manager.RemoveFileFromManager(FileLocks[i].Key);
// 条件体结束。
			}
// 否则。
			else
// 否则体开始。
			{
// 打警告：文件还在磁盘上。
				UE_LOG(LogPlatformFileManagedStorage, Warning, TEXT("Not removing deleted file %s. It still exists on disk."), *(FileLocks[i].Key.FullFilename));
// 否则体结束。
			}
// 循环体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}
```

**本段新元素**

- `DeleteFile(Filename)` —— 删除文件（带记账）
- `if (!IsReady())` —— 未就绪直接透传
- `Manager.TryManageFile(Filename)` —— 尝试托管
- `FManagedStorageScopeFileLock ScopeFileLock(ManagedFile)` —— 加文件锁
- `Manager.RemoveFileFromManager(ManagedFile)` —— 删除成功后从管理器移除
- `DeleteFiles(Filenames)` —— 批量删除
- `TArray<TPair<FPersistentManagedFile, FManagedStorageScopeFileLock>> FileLocks` —— 每个文件一把锁
- `FileLocks.Emplace(ManagedFile, MoveTemp(ManagedFile))` —— 加锁并保存
- `if (!BaseClass::FileExists(Filenames[i]))` —— 检查是否真的删掉了
- `UE_LOG(..., Warning, ...)` —— 文件还在磁盘上时打警告

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 858~878 行 | `DeleteFile` |
| 第 880~914 行 | `DeleteFiles`（批量） |

> 类比：像销毁文件：删之前先登记，删完确认真的没了，没删掉就报警。

### 十八、`MoveFile`（第 916~982 行）

```cpp
// MoveFile：移动文件（带记账）。
	virtual bool MoveFile(const TCHAR* To, const TCHAR* From) override
// 函数体开始。
	{
// 如果管理器未就绪。
		if (!IsReady())
// 条件体开始。
		{
// 直接转调基类。
			return BaseClass::MoveFile(To, From);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 托管目标文件。
		FPersistentManagedFile ManagedTo = Manager.TryManageFile(To);
// 托管源文件。
		FPersistentManagedFile ManagedFrom = Manager.TryManageFile(From);

// 加目标文件锁。
		FManagedStorageScopeFileLock ScopeFileLockTo(ManagedTo);
// 加源文件锁。
		FManagedStorageScopeFileLock ScopeFileLockFrom(ManagedFrom);

// 取源文件大小。
		const int64 SizeFrom = this->FileSize(From);
// 如果取不到。
		if (SizeFrom < 0)
// 条件体开始。
		{
// 返回失败。
			return false;
// 条件体结束。
		}

// 尝试给目标分类加配额（只在更小时更新 + 遵守配额）。
		EPersistentStorageManagerFileSizeFlags Result = Manager.AddOrUpdateFile(ManagedTo, SizeFrom, EPersistentStorageManagerFileSizeFlags::OnlyUpdateIfLess | EPersistentStorageManagerFileSizeFlags::RespectQuota);
// 如果超配额。
		if (EnumHasAnyFlags(Result, EPersistentStorageManagerFileSizeFlags::RespectQuota))
// 条件体开始。
		{
// 打错误日志。
			UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Failed to move file to %s.  The target category of the destination has reach quota limit in peristent storage."), To);
// 返回失败。
			return false;
// 条件体结束。
		}

// 真正移动。
		bool bSuccess = BaseClass::MoveFile(To, From);
// 如果成功。
		if (bSuccess)
// 条件体开始。
		{
// 从源分类移除。
			Manager.RemoveFileFromManager(ManagedFrom);
// 给目标分类加配额。
			Manager.AddOrUpdateFile(ManagedTo, SizeFrom);
// 条件体结束。
		}
// 否则。
		else
// 否则体开始。
		{
// 说明：某些实现支持跨卷移动，所以失败时不要假设文件系统状态。
			// 某些实现支持跨卷移动，所以失败时不要假设文件系统状态。
			// of the file system in the case of failure.

// 如果源文件已不存在。
			if (ManagedFrom && !this->FileExists(From))
// 条件体开始。
			{
// 从源分类移除。
				Manager.RemoveFileFromManager(ManagedFrom);
// 条件体结束。
			}

// 如果目标文件被托管。
			if (ManagedTo)
// 条件体开始。
			{
// 如果目标文件不存在。
				if (!this->FileExists(To))
// 条件体开始。
				{
// 从目标分类移除。
					Manager.RemoveFileFromManager(ManagedTo);
// 条件体结束。
				}
// 否则。
				else
// 否则体开始。
				{
// 取目标文件大小。
					const int64 SizeTo = this->FileSize(To);
// 如果大小有效。
					if (ensureAlways(SizeTo >= 0))
// 条件体开始。
					{
// 更新目标分类配额。
						Manager.AddOrUpdateFile(ManagedTo, SizeTo);
// 条件体结束。
					}
// 否则。
					else
// 否则体开始。
					{
// 从目标分类移除。
						Manager.RemoveFileFromManager(ManagedTo);
// 否则体结束。
					}
// 条件体结束。
				}
// 条件体结束。
			}
// 否则体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}
```

**本段新元素**

- `MoveFile(To, From)` —— 移动文件（带记账）
- `Manager.TryManageFile(To)` / `(From)` —— 托管源和目标
- `FManagedStorageScopeFileLock ScopeFileLockTo(ManagedTo)` —— 加目标锁
- `FManagedStorageScopeFileLock ScopeFileLockFrom(ManagedFrom)` —— 加源锁
- `this->FileSize(From)` —— 取源文件大小
- `AddOrUpdateFile(ManagedTo, SizeFrom, OnlyUpdateIfLess | RespectQuota)` —— 查目标配额
- `EnumHasAnyFlags(Result, RespectQuota)` —— 超配额就拒
- `BaseClass::MoveFile(To, From)` —— 真正移动
- **失败时的处理** —— 某些实现支持跨卷移动，不要假设文件系统状态
- `if (ManagedFrom && !this->FileExists(From))` —— 源已不存在就移除记账
- `if (!this->FileExists(To))` —— 目标不存在就移除记账，否则更新大小

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 916~921 行 | 未就绪透传 |
| 第 923~929 行 | 托管 + 加锁 |
| 第 931~942 行 | 查目标配额 |
| 第 944~949 行 | 成功：更新记账 |
| 第 950~979 行 | 失败：按实际状态修正记账 |
| 第 981~982 行 | 返回结果 |

> 类比：像搬家：先确认新家放得下（查配额），搬完再更新两边的账。

### 十九、`OpenWrite`（第 984~1028 行）

```cpp
// OpenWrite：打开文件写（带配额检查）。
	virtual IFileHandle* OpenWrite(const TCHAR* Filename, bool bAppend = false, bool bAllowRead = false) override
// 函数体开始。
	{
// 如果管理器未就绪。
		if (!IsReady())
// 条件体开始。
		{
// 说明：未就绪时也包装句柄，等就绪后会自动开始托管。
			// 未就绪时也包装句柄。FManagedStorageFileWriteHandle 会在就绪后自动开始托管。
			// internally when we become ready.
// 让基类打开。
			IFileHandle* InnerHandle = BaseClass::OpenWrite(Filename, bAppend, bAllowRead);
// 如果打开失败。
			if (!InnerHandle)
// 条件体开始。
			{
// 返回空。
				return nullptr;
// 条件体结束。
			}

// 托管文件信息。
			FPersistentManagedFile ManagedFile;
// 转成完整路径。
			ManagedFile.FullFilename = FPaths::ConvertRelativePathToFull(Filename);
// 包一层写句柄。
			return new FManagedStorageFileWriteHandle(InnerHandle, MoveTemp(ManagedFile));
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 尝试托管。
		FPersistentManagedFile ManagedFile = Manager.TryManageFile(Filename);

// 如果分类已满。
		if (Manager.IsCategoryForFileFull(ManagedFile))
// 条件体开始。
		{
// 打错误日志。
			UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Failed to open file %s for write.  The category of the file has reach quota limit in peristent storage."), Filename);
// 返回空。
			return nullptr;
// 条件体结束。
		}

// 加文件锁。
		FManagedStorageScopeFileLock ScopeFileLock(ManagedFile);

// 让基类打开。
		IFileHandle* InnerHandle = BaseClass::OpenWrite(Filename, bAppend, bAllowRead);
// 如果打开失败。
		if (!InnerHandle)
// 条件体开始。
		{
// 返回空。
			return nullptr;
// 条件体结束。
		}

// 登记文件大小。
		Manager.AddOrUpdateFile(ManagedFile, InnerHandle->Size());
// 如果文件被托管。
		if (ManagedFile)
// 条件体开始。
		{
// 包一层写句柄。
			return new FManagedStorageFileWriteHandle(InnerHandle, MoveTemp(ManagedFile));
// 条件体结束。
		}
// 否则。
		else
// 否则体开始。
		{
// 直接返回底层句柄（不追踪）。
			return InnerHandle;
// 否则体结束。
		}
// 函数体结束。
	}
```

**本段新元素**

- `OpenWrite(Filename, bAppend, bAllowRead)` —— 打开文件写（带配额检查）
- **未就绪时也包装句柄** —— `FManagedStorageFileWriteHandle` 会在就绪后自动开始托管
- `Manager.IsCategoryForFileFull(ManagedFile)` —— 分类已满就拒
- `FManagedStorageScopeFileLock ScopeFileLock(ManagedFile)` —— 加文件锁
- `Manager.AddOrUpdateFile(ManagedFile, InnerHandle->Size())` —— 登记初始大小
- `if (ManagedFile)` —— 被托管就包一层，否则直接返回底层句柄

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 984~999 行 | 未就绪分支（也包装） |
| 第 1001~1009 行 | 查分类是否已满 |
| 第 1011~1019 行 | 加锁 + 打开 + 登记 |
| 第 1020~1028 行 | 按是否托管决定包装 |

> 类比：像开新账户：先查额度够不够，够就开户并登记初始余额。

### 二十、`CopyFile`（第 1030~1085 行）

```cpp
// CopyFile：复制文件（带配额检查）。
	virtual bool CopyFile(const TCHAR* To, const TCHAR* From, EPlatformFileRead ReadFlags = EPlatformFileRead::None, EPlatformFileWrite WriteFlags = EPlatformFileWrite::None) override
// 函数体开始。
	{
// 如果管理器未就绪。
		if (!IsReady())
// 条件体开始。
		{
// 直接转调基类。
			return BaseClass::CopyFile(To, From, ReadFlags, WriteFlags);
// 条件体结束。
		}

// 取管理器单例。
		FPersistentStorageManager& Manager = FPersistentStorageManager::Get();

// 托管目标文件。
		FPersistentManagedFile ManagedTo = Manager.TryManageFile(To);
// 托管源文件。
		FPersistentManagedFile ManagedFrom = Manager.TryManageFile(From);

// 加目标文件锁。
		FManagedStorageScopeFileLock ScopeFileLockTo(ManagedTo);
// 加源文件锁。
		FManagedStorageScopeFileLock ScopeFileLockFrom(ManagedFrom);

// 取源文件大小。
		const int64 SizeFrom = this->FileSize(From);
// 如果取不到。
		if (SizeFrom < 0)
// 条件体开始。
		{
// 返回失败。
			return false;
// 条件体结束。
		}

// 尝试给目标分类加配额。
		EPersistentStorageManagerFileSizeFlags Result = Manager.AddOrUpdateFile(ManagedTo, SizeFrom, EPersistentStorageManagerFileSizeFlags::OnlyUpdateIfLess | EPersistentStorageManagerFileSizeFlags::RespectQuota);
// 如果超配额。
		if (EnumHasAnyFlags(Result, EPersistentStorageManagerFileSizeFlags::RespectQuota))
// 条件体开始。
		{
// 打错误日志。
			UE_LOG(LogPlatformFileManagedStorage, Error, TEXT("Failed to copy file to %s.  The category of the destination has reach quota limit in peristent storage."), To);
// 返回失败。
			return false;
// 条件体结束。
		}

// 真正复制。
		bool bSuccess = BaseClass::CopyFile(To, From, ReadFlags, WriteFlags);
// 如果成功。
		if (bSuccess)
// 条件体开始。
		{
// 更新目标分类配额。
			Manager.AddOrUpdateFile(ManagedTo, SizeFrom);
// 条件体结束。
		}
// 否则如果目标文件被托管。
		else if(ManagedTo)
// 条件体开始。
		{
// 如果目标文件不存在。
			if (!this->FileExists(To))
// 条件体开始。
			{
// 从目标分类移除。
				Manager.RemoveFileFromManager(ManagedTo);
// 条件体结束。
			}
// 否则。
			else
// 否则体开始。
			{
// 取目标文件大小。
				const int64 SizeTo = this->FileSize(To);
// 如果大小有效。
				if (ensureAlways(SizeTo >= 0))
// 条件体开始。
				{
// 更新目标分类配额。
					Manager.AddOrUpdateFile(ManagedTo, SizeTo);
// 条件体结束。
				}
// 否则。
				else
// 否则体开始。
				{
// 从目标分类移除。
					Manager.RemoveFileFromManager(ManagedTo);
// 否则体结束。
				}
// 条件体结束。
			}
// 条件体结束。
		}

// 返回结果。
		return bSuccess;
// 函数体结束。
	}
// 类结束。
};
```

**本段新元素**

- `CopyFile(To, From, ReadFlags, WriteFlags)` —— 复制文件（带配额检查）
- `Manager.TryManageFile(To)` / `(From)` —— 托管源和目标
- `FManagedStorageScopeFileLock ScopeFileLockTo(ManagedTo)` —— 加目标锁
- `FManagedStorageScopeFileLock ScopeFileLockFrom(ManagedFrom)` —— 加源锁
- `this->FileSize(From)` —— 取源文件大小
- `AddOrUpdateFile(ManagedTo, SizeFrom, OnlyUpdateIfLess | RespectQuota)` —— 查目标配额
- `BaseClass::CopyFile(To, From, ReadFlags, WriteFlags)` —— 真正复制
- **失败时的处理** —— 按实际状态修正记账

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1030~1035 行 | 未就绪透传 |
| 第 1037~1043 行 | 托管 + 加锁 |
| 第 1045~1056 行 | 查目标配额 |
| 第 1058~1062 行 | 成功：更新记账 |
| 第 1063~1081 行 | 失败：按实际状态修正 |
| 第 1083~1085 行 | 返回结果 + 类结束 |

> 类比：像复印文件：先确认纸够不够（查配额），复印完更新两边的账。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 30 个 include + 日志分类 | 1~39 | 版权头 + 30 个 include + 日志分类 |
| `ManagedStorageInternal` 命名空间 | 41~47 | `ManagedStorageInternal` 命名空间 |
| `FPersistentManagedFile` | 49~61 | `FPersistentManagedFile` |
| `FManagedStorageFileLockRegistry`（按文件名分配锁） | 63~107 | `FManagedStorageFileLockRegistry`（按文件名分配锁） |
| `FManagedStorageScopeFileLock`（RAII 文件锁） | 109~137 | `FManagedStorageScopeFileLock`（RAII 文件锁） |
| `EPersistentStorageManagerFileSizeFlags` 枚举 | 139~145 | `EPersistentStorageManagerFileSizeFlags` 枚举 |
| `FPersistentStorageCategory` 构造 + 6 个 getter | 147~197 | `FPersistentStorageCategory` 构造 + 6 个 getter |
| `AddOrUpdateFile` / `RemoveFile` | 199~245 | `AddOrUpdateFile` / `RemoveFile` |
| `CategoryStat` + `GetStat` | 247~276 | `CategoryStat` + `GetStat` |
| `TryUpdateQuota`（CAS 循环）+ `ShouldManageFile` | 278~341 | `TryUpdateQuota`（CAS 循环）+ `ShouldManageFile` |
| `FPersistentStorageManager` 声明 + `IsReady` + `Initialize` | 343~422 | `FPersistentStorageManager` 声明 + `IsReady` + `Initialize` |
| `TryManageFile` ×2 + `TryManageFileInternal` | 424~463 | `TryManageFile` ×2 + `TryManageFileInternal` |
| 10 个公开方法 | 465~606 | 10 个公开方法 |
| 私有成员：`FRootDirInfo` + `FCategoryInfo` | 608~647 | 私有成员：`FRootDirInfo` + `FCategoryInfo` |
| `FManagedStorageFileWriteHandle` | 649~821 | `FManagedStorageFileWriteHandle` |
| `TManagedStoragePlatformFile<BaseClass>` 构造 | 823~856 | `TManagedStoragePlatformFile<BaseClass>` 构造 |
| `DeleteFile` / `DeleteFiles` | 858~914 | `DeleteFile` / `DeleteFiles` |
| `MoveFile` | 916~982 | `MoveFile` |
| `OpenWrite` | 984~1028 | `OpenWrite` |
| `CopyFile` | 1030~1085 | `CopyFile` |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `std::atomic<int64> X{ 0 }` | 原子变量 | `std::atomic<int64> UsedQuota{ 0 };` |
| `X.compare_exchange_weak(A, B)` | CAS 原子比较交换 | `UsedQuota.compare_exchange_weak(OldUsedQuota, NewUsedQuota)` |
| `mutable T X` | 允许 const 方法修改 | `mutable FRWLock FileSizesLock;` |
| `FReadScopeLock L(&RWLock)` | 读锁（RAII） | `FReadScopeLock ScopeLock(FileSizesLock);` |
| `FWriteScopeLock L(&RWLock)` | 写锁（RAII） | `FWriteScopeLock ScopeLock(FileSizesLock);` |
| `Map.FindOrAddByHash(H, K)` | 按哈希找到或新建 | `FileLockMap.FindOrAddByHash(KeyHash, InFilename)` |
| `Map.RemoveAndCopyValue(K, Out)` | 移除并取出值 | `FileSizes.RemoveAndCopyValue(Filename, OldSize)` |
| `EnumHasAnyFlags(E, F)` | 枚举位测试 | `EnumHasAnyFlags(Result, RespectQuota)` |
| `TOptional<T>` | 可选值容器 | `TOptional<CategoryStat> Result;` |
| `TArrayView<const T>` | 只读数组视图 | `TArrayView<const FString> GetRootDirectories() const` |
| `template<class B> class X : public B` | 模板继承 | `template<class BaseClass> class TManagedStoragePlatformFile : public BaseClass` |
| `&A::F == &B::F` | 函数指针比较 | `&BaseClass::DeleteFile == &IPlatformFile::DeleteFile` |
| `LowLevelFatalError(TEXT("..."))` | 致命错误（静态初始化期可用） | `LowLevelFatalError(TEXT("..."));` |
| `AsyncTask(ENamedThreads::X, lambda)` | 起异步任务 | `AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [](){...});` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 1085 行 | 1953 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 846 行 | 846 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `/// <summary>` 文档注释 | 英文原文 | 中文 | 已译 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`（1953 行）
2. 脚本验收：`codeOurs=846 / codeEngine=846 / diff=0`
3. 确认后下一个：`HAL\IPlatformFileModule.h`（415 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`（1085 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`（1953 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\15_IPlatformFileManagedStorageWrapper.h_总md.md`