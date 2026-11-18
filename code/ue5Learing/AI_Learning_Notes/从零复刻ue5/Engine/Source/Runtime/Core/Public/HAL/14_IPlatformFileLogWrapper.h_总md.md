# 14 — `HAL/IPlatformFileLogWrapper.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`（**513 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileLogWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `IFileHandle` / `FScopeLock`。

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

## 二、内部结构全景

```
IPlatformFileLogWrapper.h（513 行）
├─ 第 1~23 行     版权头 + #pragma once + 14 个 include + 4 个前置声明
├─ 第 25~38 行    ① LogPlatformFile 日志分类 + bSuppressFileLog + FILE_LOG 宏
├─ 第 40~143 行   ② class FLoggedFileHandle : public IFileHandle
│    ├─ 第 44~48 行   3 个成员（FileHandle / Filename / PlatformFile）
│    ├─ 第 51~52 行   构造 / 析构（实现在 .cpp）
│    ├─ 第 54~62 行   Tell()
│    ├─ 第 63~71 行   Seek()
│    ├─ 第 72~80 行   SeekFromEnd()
│    ├─ 第 81~89 行   Read()
│    ├─ 第 90~98 行   ReadAt()
│    ├─ 第 99~107 行  Write()
│    ├─ 第 108~116 行 Size()
│    ├─ 第 117~125 行 Flush()
│    ├─ 第 126~134 行 Truncate()
│    └─ 第 135~142 行 ShrinkBuffers()
├─ 第 145~513 行  ③ class FLoggedPlatformFile : public IPlatformFile
│    ├─ 第 147~152 行 3 个成员（LowerLevel / LogFileCritical / OpenHandles）
│    ├─ 第 155~186 行 GetTypeName / 构造 / 4 个 using / GetLowerLevel / SetLowerLevel / GetName
│    ├─ 第 171~173 行 ShouldBeUsed / Initialize（实现在 .cpp）
│    ├─ 第 188~363 行 15 个文件操作（每个都是三段式）
│    ├─ 第 365~382 行 FLogVisitor（内部类）
│    ├─ 第 384~407 行 2 个 IterateDirectory
│    ├─ 第 409~426 行 FLogStatVisitor（内部类）
│    ├─ 第 428~451 行 2 个 IterateDirectoryStat
│    ├─ 第 453~474 行 DeleteDirectoryRecursively / CopyFile
│    ├─ 第 476~493 行 OnHandleOpen / OnHandleClosed / HandleDumpCommand（非 Shipping）
│    ├─ 第 494~505 行 OpenAsyncRead
│    ├─ 第 506 行     OpenMappedEx（实现在 .cpp）
│    └─ 第 508~513 行 SetAsyncMinimumPriority
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IPlatformFileLogWrapper.h` | **是**（`IPlatformFile` / `IFileHandle`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `Containers/Map.h` | `TMap<FString, int32>`（OpenHandles） |
| `Containers/UnrealString.h` | `FString` |
| `CoreTypes.h` | `uint8` / `int32` / `int64` / `TCHAR` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `IFileHandle` / `FFileStatData` |
| `HAL/CriticalSection.h` | `FCriticalSection` |
| `HAL/PlatformMisc.h` | `FPlatformProperties` |
| `HAL/PlatformTime.h` | `FPlatformTime::Seconds` |
| `Logging/LogCategory.h` | `DECLARE_LOG_CATEGORY_EXTERN` |
| `Logging/LogMacros.h` | `UE_LOG` |
| `Math/Color.h` | `FColor::Emerald` |
| `Misc/DateTime.h` | `FDateTime` |
| `Misc/ScopeLock.h` | `FScopeLock` |
| `Templates/UniquePtr.h` | `TUniquePtr<IFileHandle>` |
| `Trace/Detail/Channel.h` | `SCOPED_NAMED_EVENT_FSTRING` |

**注意**：`TArray` / `EPlatformFileRead` / `EPlatformFileWrite` / `EAsyncIOPriorityAndFlags` / `EOpenReadFlags` / `FOpenMappedResult` 都**没有在本文件 include**，靠上面 14 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\27_IPlatformFileLogWrapper.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IPlatformFileManagedStorageWrapper.h`（31770 字节）。