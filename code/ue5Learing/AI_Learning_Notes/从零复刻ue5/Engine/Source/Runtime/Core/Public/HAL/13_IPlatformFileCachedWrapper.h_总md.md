# 13 — `HAL/IPlatformFileCachedWrapper.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`（**468 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `IFileHandle` / `FOpenMappedResult`。

---

## 一、这个文件解决什么问题（为什么）

读文件时，如果每次 `Read(1 字节)` 都真的去问操作系统，系统调用开销会吃掉全部性能。解法是**加一层缓存**：一次读 64KB 到内存，之后的小读全从内存拿。

`IPlatformFileCachedWrapper.h` 就是这层缓存，它包含两个类：

| 类 | 作用 |
|---|---|
| `FCachedFileHandle` | **带缓存的文件句柄**（包住底层 `IFileHandle`） |
| `FCachedReadPlatformFile` | **带缓存的平台文件包装器**（包住底层 `IPlatformFile`） |

**核心设计：双缓存槽轮转**

```
BufferCache[2][64KB]     ← 2 个 64KB 缓存块
CacheStart[2]            ← 每块的起始文件位置
CacheEnd[2]              ← 每块的结束文件位置
CurrentCache             ← 当前正在填的槽（0 或 1，轮转）
```

**为什么是 2 个槽**：读小块时，如果只用一个槽，每次跨块都要重新填。用 2 个槽轮转，可以「一边用旧块、一边填新块」，减少重填次数。

**两个位置的区别**（这是本文件最容易搞混的地方）：

| 成员 | 含义 |
|---|---|
| `FilePos` | **逻辑位置** —— 调用方以为自己在哪 |
| `TellPos` | **底层实际位置** —— 底层句柄真正在哪 |

因为缓存的存在，两者可能不同。比如 `FilePos = 100`，但底层已经读到 `65536`（因为整块读进缓存了）。

**说个场景**：

```cpp
IFileHandle* H = File.OpenRead("Data.bin");   // 返回 FCachedFileHandle
H->Read(&Byte, 1);   // 第一次：底层读 64KB 到缓存，返回第 1 字节
H->Read(&Byte, 1);   // 第二次：直接从缓存拿，零系统调用
```

**`ShouldBeUsed` 的启用条件**（第 289~316 行）：

```cpp
bResult = !bPlatformProvidesFileCache   // 平台自己没缓存
       && !bPlatformWindows             // 不是 Windows
       && FPlatformProperties::RequiresCookedData();  // 需要 cook 数据
```

桌面平台可以用命令行 `-CachedReadFile` / `-NoCachedReadFile` 强制开关。

> 🔑 **一句话**：`IPlatformFileCachedWrapper.h` = **文件读的「预读缓冲」**，一次读 64KB，之后小读全走内存。

---

## 二、内部结构全景

```
IPlatformFileCachedWrapper.h（468 行）
├─ 第 1~20 行     版权头 + #pragma once + 14 个 include + 1 个前置声明
├─ 第 22~260 行   ① class FCachedFileHandle : public IFileHandle
│    ├─ 第 25~35 行   构造函数（保存底层句柄 + 初始化位置/大小 + 清缓存）
│    ├─ 第 37~39 行   虚析构
│    ├─ 第 42~45 行   Tell()          返回 FilePos
│    ├─ 第 47~55 行   Seek()          越界检查 + 更新 FilePos
│    ├─ 第 57~60 行   SeekFromEnd()   换算后调 Seek
│    ├─ 第 62~137 行  Read()          ★ 核心：大块直读 / 小块走缓存
│    ├─ 第 139~143 行 ReadAt()        不走缓存，直接转底层
│    ├─ 第 145~167 行 Write()         写后清缓存
│    ├─ 第 169~172 行 Size()
│    ├─ 第 174~181 行 Flush()
│    ├─ 第 183~196 行 Truncate()      截断后清缓存
│    ├─ 第 198~201 行 ShrinkBuffers()
│    ├─ 第 203~248 行 private：3 个常量 + 4 个辅助函数
│    │    ├─ BufferCacheSize = 64KB
│    │    ├─ BufferSizeMask = ~(64KB-1)
│    │    ├─ CacheCount = 2
│    │    ├─ InnerSeek()      带位置缓存优化
│    │    ├─ InnerRead()      带位置更新
│    │    ├─ GetCacheIndex()  查位置落在哪个槽
│    │    └─ FlushCache()     清空所有槽
│    └─ 第 250~259 行 10 个成员变量
└─ 第 262~468 行   ② class FCachedReadPlatformFile : public IPlatformFile
     ├─ 第 266~269 行  GetTypeName()  返回 "CachedReadFile"
     ├─ 第 271~274 行  默认构造
     ├─ 第 276~280 行  4 个 using（让基类重载可见）
     ├─ 第 282~288 行  Initialize()   保存下一层
     ├─ 第 289~316 行  ShouldBeUsed() ★ 启用条件判断
     ├─ 第 317~328 行  GetLowerLevel / SetLowerLevel / GetName
     ├─ 第 329~368 行  10 个纯转调（FileExists / FileSize / DeleteFile …）
     ├─ 第 369~386 行  OpenRead / OpenWrite ★ 包一层 FCachedFileHandle
     ├─ 第 387~418 行  8 个纯转调（目录相关）
     ├─ 第 419~426 行  FindFiles / FindFilesRecursively（无 override）
     ├─ 第 427~442 行  4 个纯转调（Copy / CreateTree）
     ├─ 第 443~454 行  3 个纯转调（路径转换 / 服务器消息）
     └─ 第 455~466 行  3 个纯转调（异步读 / 映射 / 优先级）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IPlatformFileCachedWrapper.h` | **是**（`IPlatformFile` / `IFileHandle`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint8` / `uint32` / `int32` / `int64` / `TCHAR` |
| `Misc/AssertionMacros.h` | `check` |
| `HAL/UnrealMemory.h` | `FMemory::Memcpy` |
| `Math/UnrealMathUtility.h` | `FMath::Min` / `FMath::Max` |
| `Serialization/Archive.h` | `FArchive` |
| `Containers/UnrealString.h` | `FString` |
| `Misc/Parse.h` | `FParse::Param` |
| `Logging/LogMacros.h` | `UE_LOG` |
| `Misc/DateTime.h` | `FDateTime` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `IFileHandle` / `FFileStatData` |
| `HAL/IPlatformFileLogWrapper.h` | `LogPlatformFile` 日志分类 |
| `Templates/UniquePtr.h` | `TUniquePtr<IFileHandle>` |
| `Async/MappedFileHandle.h` | `FOpenMappedResult` |

**注意**：`TArray` / `EPlatformFileRead` / `EPlatformFileWrite` / `EAsyncIOPriorityAndFlags` / `EOpenReadFlags` 都**没有在本文件 include**，靠上面 14 个头间接引入（引擎的隐式依赖，照抄不改）。

**引擎原文瑕疵**：`FindFiles`（第 419 行）和 `FindFilesRecursively`（第 423 行）**没有 `override` 关键字** —— 照抄不改。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\26_IPlatformFileCachedWrapper.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IPlatformFileLogWrapper.h`（22959 字节）。