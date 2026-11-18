# 09 — `HAL/FileManagerGeneric.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`（**374 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManagerGeneric.h`
> **⚠️ 本节不能独立编译**——依赖 `IFileManager` / `IPlatformFile` / `FArchive`。

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

## 二、内部结构全景

```
FileManagerGeneric.h（374 行）
├─ 第 1~14 行     版权头 + #pragma once + 10 个 include
├─ 第 16~26 行    ① 3 个缓冲大小宏（读 1024 / 写 4096 / 调试 4096）
├─ 第 28~219 行   ② class FFileManagerGeneric : public IFileManager
│    ├─ 第 38~42 行   GetLowLevel()（每次取单例，不缓存）
│    ├─ 第 49~54 行   构造 / 析构
│    ├─ 第 60~70 行   ProcessCommandLineOptions / 沙箱开关
│    ├─ 第 72~87 行   CreateFileReader / Writer / DebugWriter（内联转调 Internal）
│    ├─ 第 89~107 行  13 个 CORE_API 声明（实现在 .cpp）
│    ├─ 第 119 行     FindFiles（扩展名版）
│    ├─ 第 127~155 行 8 个 IterateDirectory
│    ├─ 第 163~195 行 DefaultConvertToRelativePath / 3 个路径转换 / FileSize
│    ├─ 第 204~207 行 SendMessageToServer（内联转调）
│    └─ 第 209~218 行 private：4 个内部辅助函数
├─ 第 222~296 行   ③ class FArchiveFileReaderGeneric : public FArchive
│    ├─ 第 229~249 行 构造 / 析构 / Seek / Tell / TotalSize / Close / Serialize / GetArchiveName / FlushCache / Precache
│    ├─ 第 251~267 行 protected：InternalPrecache / SeekLowLevel / CloseLowLevel / ReadLowLevel
│    ├─ 第 270~273 行 IsSilent()
│    ├─ 第 276~288 行 9 个成员（Filename / Size / Pos / BufferBase / Handle / BufferArray / BufferSize / Flags / bFirstReadAfterSeek）
│    └─ 第 290~295 行 匿名枚举 + 友元
└─ 第 299~374 行   ④ class FArchiveFileWriterGeneric : public FArchive
     ├─ 第 306~321 行 构造 / 析构 / Seek / Tell / TotalSize / Close / Serialize / Flush / GetArchiveName
     ├─ 第 323~358 行 protected：FlushBuffer / SeekLowLevel / CloseLowLevel / WriteLowLevel / LogWriteError ×2
     ├─ 第 361~364 行 IsSilent()
     └─ 第 367~373 行 6 个成员（Filename / Flags / Pos / Handle / BufferArray / BufferSize / bLoggingError）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/FileManagerGeneric.h` | **是**（`IFileManager` / `IPlatformFile` / `FArchive`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `Containers/Array.h` | `TArray` / `TArray64` |
| `Containers/UnrealString.h` | `FString` |
| `CoreTypes.h` | `uint8` / `uint32` / `int64` / `TCHAR` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `IFileHandle` / `FFileStatData` |
| `HAL/FileManager.h` | **基类 `IFileManager`** |
| `HAL/PlatformFileManager.h` | `FPlatformFileManager` |
| `Misc/DateTime.h` | `FDateTime` |
| `Serialization/Archive.h` | **基类 `FArchive`** |
| `Templates/UniquePtr.h` | `TUniquePtr<IFileHandle>` |

**注意**：`GetTimeStampPair`（第 99 行）**没有 `override` 关键字** —— 引擎原文如此，照抄不改。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\22_FileManagerGeneric.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `FMemory.inl`（8498 字节）。