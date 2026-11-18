# 08 — `HAL/FileManager.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`（**291 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FileManager.h`
> **⚠️ 本节不能独立编译**——依赖 `GenericPlatform/GenericPlatformFile.h` 的 `IPlatformFile` / `FFileStatData`。

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
| **路径** | `ConvertToRelativePath` / `ConvertToAbsolutePathForExternalAppForRead` / `ConvertToAbsolutePathForExternalAppForWrite` / `GetFilenameOnDisk` |
| **其他** | `Get` / `ProcessCommandLineOptions` / `SetSandboxEnabled` / `IsSandboxEnabled` / `SendMessageToServer` |

> 🔑 **一句话**：`FileManager.h` = **文件操作的「总服务台」**，40+ 个窗口，各平台自己派人值班。

---

## 二、内部结构全景

```
FileManager.h（291 行）
├─ 第 1~8 行      版权头 + #pragma once + 4 个 include
├─ 第 10~11 行    ① MAX_UNREAL_FILENAME_LENGTH_DEPRECATED 宏
├─ 第 14~23 行    ② enum EFileWrite（7 个值）
├─ 第 26~32 行    ③ enum EFileRead（4 个值）
├─ 第 35~40 行    ④ enum ECopyResult（3 个值）
├─ 第 43~46 行    ⑤ struct FCopyProgress（1 个纯虚 Poll）
├─ 第 49~54 行    ⑥ enum EFileOpenFlags（3 个值）
└─ 第 56~291 行   ⑦ class IFileManager
     ├─ 第 58~61 行   protected 构造函数
     ├─ 第 66 行      static Get()
     ├─ 第 69~74 行   ProcessCommandLineOptions / SetSandboxEnabled / IsSandboxEnabled
     ├─ 第 85~101 行  CreateFileReader / CreateFileWriter / CreateDebugFileWriter
     ├─ 第 104~128 行 IsReadOnly / Delete / Copy / Move / FileExists / DirectoryExists / MakeDirectory / DeleteDirectory / GetStatData
     ├─ 第 131~146 行 FindFiles ×2 / FindFilesRecursive
     ├─ 第 154~210 行 IterateDirectory ×8（4 种 × 2 重载）
     ├─ 第 213~236 行 GetFileAgeSeconds / GetTimeStamp / GetTimeStampPair / SetTimeStamp / GetAccessTimeStamp
     ├─ 第 245~263 行 ConvertToRelativePath / ConvertToAbsolutePathForExternalAppForRead / ForWrite
     ├─ 第 271 行     FileSize
     ├─ 第 281 行     SendMessageToServer
     └─ 第 290 行     GetFilenameOnDisk
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/FileManager.h` | **是**（`IPlatformFile` / `FFileStatData`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint8` / `uint32` / `int64` / `TCHAR` |
| `Containers/UnrealString.h` | `FString` |
| `Misc/DateTime.h` | `FDateTime` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `FFileStatData` / `FDirectoryVisitor` / `FDirectoryStatVisitor` |

**注意**：`FArchive` / `TArray` / `INDEX_NONE` 都**没有在本文件 include**，靠上面 4 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\21_FileManager.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `FileManagerGeneric.h`（15333 字节）。