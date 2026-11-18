# 17 — `HAL/IPlatformFileOpenLogWrapper.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileOpenLogWrapper.h`（**286 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileOpenLogWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `IAsyncReadFileHandle`。

---

## 一、这个文件解决什么问题（为什么）

游戏启动慢，你想知道「**文件是按什么顺序打开的**」。`IPlatformFileOpenLogWrapper.h` 就是**文件打开顺序记录器**：它把每次 `OpenRead` 的文件名 + 顺序号写进日志文件，之后可以分析「哪些文件该提前打包」「哪些文件被重复打开」。

**它包含 3 个东西**：

| 类型 | 作用 |
|---|---|
| `FILE_OPEN_ORDER_LABEL(...)` | 往日志里写一条**标签**的宏（标记阶段） |
| `FLoggingAsyncReadFileHandle` | 带日志的异步读句柄（记录「开始读」的时刻） |
| `FPlatformFileOpenLog` | **文件打开顺序记录器**（包住 `IPlatformFile`） |

**核心设计 ①：只在非 Shipping 构建启用**（第 25 行 / 第 281~285 行）

```cpp
#if !UE_BUILD_SHIPPING
    // 完整实现
#else
    #define FILE_OPEN_ORDER_LABEL(Format, ...)   // 空宏
#endif
```

Shipping 下 `FILE_OPEN_ORDER_LABEL` 变成空宏，**零开销**。

**核心设计 ②：命令行开关**（第 84~88 行）

```cpp
bool bResult = FParse::Param(CmdLine, TEXT("FileOpenLog"))
            || FParse::Param(CmdLine, TEXT("FilePackageOpenLog"));
```

只有加 `-FileOpenLog` 或 `-FilePackageOpenLog` 才启用。

**核心设计 ③：只记录「第一次打开」**（第 239~247 行）

```cpp
else if (FilenameAccessMap.Find(Filename) == nullptr)   // 没记录过
{
    FilenameAccessMap.Emplace(Filename, ++OpenOrder);   // 记录顺序号
    FString Text = FString::Printf(TEXT("\"%s\" %llu\n"), Filename, OpenOrder);
    // 写入所有输出文件
}
```

`bLogDuplicates = false` 时，同一文件只记第一次。这样日志里就是**纯净的首次打开顺序**。

**核心设计 ④：异步读的特殊处理**（第 221~225 行）

```cpp
virtual IAsyncReadFileHandle* OpenAsyncRead(...) override
{
    // 这里不能记录「打开」……真正重要的是「开始读」的时刻！
    return new FLoggingAsyncReadFileHandle(this, Filename, LowerLevel->OpenAsyncRead(...));
}
```

异步读的「打开」和「真正读」是分开的，所以包一层 `FLoggingAsyncReadFileHandle`，在 `ReadRequest` 时才记录。

**核心设计 ⑤：多输出文件**（第 62 行）

```cpp
TArray<IFileHandle*> LogOutput;   // 可同时写多个日志文件
```

`AddToOpenLog` / `AddLabelToOpenLog` 都遍历 `LogOutput` 逐个写。

**说个场景**：

```
命令行：UnrealEditor.exe MyProject.uproject -FileOpenLog
日志输出（FileOpenLog.txt）：
  "D:/MyProject/Content/Map.umap" 1
  "D:/MyProject/Content/T_Rock.uasset" 2
  # BeginLoadMap
  "D:/MyProject/Content/T_Grass.uasset" 3
```

`# BeginLoadMap` 就是 `FILE_OPEN_ORDER_LABEL("BeginLoadMap")` 写进去的标签。

> 🔑 **一句话**：`IPlatformFileOpenLogWrapper.h` = **文件打开的「顺序记录仪」**，记首次打开顺序 + 阶段标签，用于优化加载。

---

## 二、内部结构全景

```
IPlatformFileOpenLogWrapper.h（286 行）
├─ 第 1~23 行     版权头 + #pragma once + 15 个 include + 3 个前置声明
├─ 第 25~28 行    ① #if !UE_BUILD_SHIPPING + FILE_OPEN_ORDER_LABEL 宏
├─ 第 30~51 行    ② class FLoggingAsyncReadFileHandle final
│    ├─ 第 32~34 行   3 个成员（Owner / Filename / ActualRequest）
│    ├─ 第 36~41 行   构造函数
│    ├─ 第 42~45 行   析构函数（delete ActualRequest）
│    ├─ 第 46~49 行   SizeRequest（转调）
│    └─ 第 50 行      ReadRequest（实现在 .cpp，记录打开）
├─ 第 54~280 行   ③ class FPlatformFileOpenLog : public IPlatformFile
│    ├─ 第 56~63 行   protected：6 个成员
│    ├─ 第 67~76 行   构造 / 析构
│    ├─ 第 78~82 行   4 个 using
│    ├─ 第 84~88 行   ShouldBeUsed（命令行开关）
│    ├─ 第 90 行      Initialize（实现在 .cpp）
│    ├─ 第 92~107 行  GetLowerLevel / SetLowerLevel / GetTypeName / GetName
│    ├─ 第 108~147 行 10 个纯转调
│    ├─ 第 148~156 行 OpenRead（★ 记录打开）
│    ├─ 第 157~160 行 OpenWrite（不记录）
│    ├─ 第 161~220 行 15 个纯转调
│    ├─ 第 221~225 行 OpenAsyncRead（★ 包一层）
│    ├─ 第 226 行     OpenMappedEx（实现在 .cpp）
│    ├─ 第 228~249 行 AddToOpenLog（★ 核心记录逻辑）
│    ├─ 第 251~260 行 AddLabelToOpenLog
│    ├─ 第 263~267 行 AddPackageToOpenLog（旧接口）
│    ├─ 第 269~275 行 AddLabel（静态模板）
│    └─ 第 277~280 行 private：AddLabelInternal
└─ 第 281~286 行   ④ #else 分支：FILE_OPEN_ORDER_LABEL 空宏
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IPlatformFileOpenLogWrapper.h` | **是**（`IPlatformFile` / `IAsyncReadFileHandle`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `Async/AsyncFileHandle.h` | `IAsyncReadFileHandle` / `IAsyncReadRequest` |
| `Containers/Array.h` / `Map.h` | `TArray` / `TMap` |
| `Containers/StringConv.h` | `StringCast<ANSICHAR>` |
| `Containers/UnrealString.h` | `FString` |
| `CoreTypes.h` | `uint8` / `int64` / `TCHAR` |
| `GenericPlatform/GenericPlatformFile.h` | `IPlatformFile` / `IFileHandle` / `FFileStatData` |
| `HAL/CriticalSection.h` | `FCriticalSection` |
| `HAL/PlatformCrt.h` / `PlatformMisc.h` / `PlatformProperties.h` | 平台相关 |
| `Misc/DataDrivenPlatformInfoRegistry.h` | 数据驱动平台信息 |
| `Misc/DateTime.h` | `FDateTime` |
| `Misc/Parse.h` | `FParse::Param` |
| `Misc/Paths.h` | `FPaths` |

**注意**：`EPlatformFileRead` / `EPlatformFileWrite` / `EAsyncIOPriorityAndFlags` / `EOpenReadFlags` / `FOpenMappedResult` / `IFileServerMessageHandler` 都**没有在本文件 include**，靠上面 15 个头间接引入（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\30_IPlatformFileOpenLogWrapper.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `LowLevelMemStats.h`（6566 字节）。