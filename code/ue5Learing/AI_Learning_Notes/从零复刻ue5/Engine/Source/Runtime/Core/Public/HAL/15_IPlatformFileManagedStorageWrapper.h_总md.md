# 15 — `HAL/IPlatformFileManagedStorageWrapper.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`（**1085 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileManagedStorageWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `GConfig` / `FPaths` / `FRWLock`。

---

## 一、这个文件解决什么问题（为什么）

手机 / 主机的**持久化存储有配额**（比如「存档最多 100MB」）。如果不管，玩家存档写爆了磁盘，游戏就崩了。`IPlatformFileManagedStorageWrapper.h` 就是**存储配额管理器**：它把文件按**分类（Category）**分组，每个分类有配额，写文件前先检查配额。

**它包含 6 个核心类型**：

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

## 二、内部结构全景

```
IPlatformFileManagedStorageWrapper.h（1085 行）
├─ 第 1~39 行      版权头 + #pragma once + 30 个 include + <atomic> + 日志分类
├─ 第 41~47 行     ① namespace ManagedStorageInternal（IsUnderDirectory）
├─ 第 49~61 行     ② struct FPersistentManagedFile
├─ 第 63~107 行    ③ class FManagedStorageFileLockRegistry（按文件名分配锁）
├─ 第 109~137 行   ④ class FManagedStorageScopeFileLock（RAII 文件锁）
├─ 第 139~145 行   ⑤ enum class EPersistentStorageManagerFileSizeFlags
├─ 第 147~341 行   ⑥ struct FPersistentStorageCategory（分类 + 配额）
│    ├─ 第 150~197 行  构造 + 6 个 getter
│    ├─ 第 199~243 行  AddOrUpdateFile / RemoveFile
│    ├─ 第 247~274 行  CategoryStat + GetStat
│    ├─ 第 279~313 行  TryUpdateQuota（CAS 循环）
│    ├─ 第 315~326 行  ShouldManageFile
│    └─ 第 328~340 行  8 个成员
├─ 第 343~647 行   ⑦ class FPersistentStorageManager（管理器单例）
│    ├─ 第 348~358 行  IsReady / Get
│    ├─ 第 362~422 行  Initialize（后台扫描）
│    ├─ 第 424~442 行  TryManageFile ×2
│    ├─ 第 445~463 行  TryManageFileInternal
│    ├─ 第 466~606 行  10 个公开方法
│    └─ 第 608~646 行  私有：FRootDirInfo / FCategoryInfo
├─ 第 649~821 行   ⑧ class FManagedStorageFileWriteHandle
└─ 第 823~1085 行  ⑨ template TManagedStoragePlatformFile<BaseClass>
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IPlatformFileManagedStorageWrapper.h` | **是**（`IPlatformFile` / `GConfig` / `FPaths`） | ❌ 不能 | 否 |

**关键依赖**（30 个 include，只列核心）：

| 文件 | 作用 |
|---|---|
| `Algo/Find.h` / `Algo/IndexOf.h` | `Algo::FindByPredicate` / `Algo::FindBy` |
| `Async/Async.h` / `Async/TaskGraphInterfaces.h` | `AsyncTask` / `ENamedThreads` |
| `Containers/Array.h` / `ArrayView.h` / `Map.h` / `SparseArray.h` | 容器 |
| `CoreGlobals.h` | `GConfig` / `GIsCriticalError` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `IFileHandle` |
| `HAL/CriticalSection.h` | `FCriticalSection` |
| `Misc/CommandLine.h` | `FCommandLine` |
| `Misc/ConfigCacheIni.h` | `GConfig` |
| `Misc/EnumClassFlags.h` | `ENUM_CLASS_FLAGS` |
| `Misc/Optional.h` | `TOptional` |
| `Misc/Paths.h` | `FPaths` |
| `Misc/ScopeLock.h` / `ScopeRWLock.h` | `FScopeLock` / `FReadScopeLock` / `FWriteScopeLock` |
| `<atomic>` | `std::atomic<int64>` |

**注意**：`EPlatformFileRead` / `EPlatformFileWrite` / `EDirectoryVisitorFlags` 都**没有在本文件 include**，靠上面 30 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\28_IPlatformFileManagedStorageWrapper.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IPlatformFileModule.h`（415 字节）。