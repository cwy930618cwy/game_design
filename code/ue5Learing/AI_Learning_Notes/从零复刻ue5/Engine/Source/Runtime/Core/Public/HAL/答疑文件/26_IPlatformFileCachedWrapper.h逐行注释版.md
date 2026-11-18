# 26 — `HAL/IPlatformFileCachedWrapper.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`（**468 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`
> **⚠️ 本节不能独立编译**——依赖 `IPlatformFile` / `IFileHandle` / `FOpenMappedResult`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

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

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 14 个 include（第 1~20 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint8 / uint32 / int32 / int64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入断言宏：check。
#include "Misc/AssertionMacros.h"
// 引入内存操作：FMemory::Memcpy。
#include "HAL/UnrealMemory.h"
// 引入模板工具：MoveTemp 等。
#include "Templates/UnrealTemplate.h"
// 引入数学工具：FMath::Min / FMath::Max。
#include "Math/UnrealMathUtility.h"
// 引入 FArchive：序列化基类。
#include "Serialization/Archive.h"
// 引入 FString：字符串类型。
#include "Containers/UnrealString.h"
// 引入命令行解析：FParse::Param。
#include "Misc/Parse.h"
// 引入日志宏：UE_LOG。
#include "Logging/LogMacros.h"
// 引入 FDateTime：文件时间戳类型。
#include "Misc/DateTime.h"
// 引入通用平台文件接口：IPlatformFile / IFileHandle / FFileStatData。
#include "GenericPlatform/GenericPlatformFile.h"
// 引入日志包装器：LogPlatformFile 日志分类。
#include "HAL/IPlatformFileLogWrapper.h"
// 引入 TUniquePtr：独占智能指针。
#include "Templates/UniquePtr.h"
// 引入映射文件句柄：FOpenMappedResult。
#include "Async/MappedFileHandle.h"

// 前置声明 IAsyncReadFileHandle：异步读句柄。
class IAsyncReadFileHandle;
```

**本段新元素**

- `CoreTypes.h` —— `uint8` / `uint32` / `int32` / `int64` / `TCHAR`
- `Misc/AssertionMacros.h` —— `check`
- `HAL/UnrealMemory.h` —— `FMemory::Memcpy`
- `Templates/UnrealTemplate.h` —— `MoveTemp` 等
- `Math/UnrealMathUtility.h` —— `FMath::Min` / `FMath::Max`
- `Serialization/Archive.h` —— `FArchive`
- `Containers/UnrealString.h` —— `FString`
- `Misc/Parse.h` —— `FParse::Param`
- `Logging/LogMacros.h` —— `UE_LOG`
- `Misc/DateTime.h` —— `FDateTime`
- `GenericPlatform/GenericPlatformFile.h` —— `IPlatformFile` / `IFileHandle` / `FFileStatData`
- `HAL/IPlatformFileLogWrapper.h` —— `LogPlatformFile` 日志分类
- `Templates/UniquePtr.h` —— `TUniquePtr<IFileHandle>`
- `Async/MappedFileHandle.h` —— `FOpenMappedResult`
- `class IAsyncReadFileHandle;` —— 前置声明

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~18 行 | 14 个 include |
| 第 20 行 | 前置声明 |

> 类比：像开工前领 14 样工具：类型、断言、内存、模板、数学、归档、字符串、解析、日志、时间、平台文件、日志包装、智能指针、映射文件。

### 二、`FCachedFileHandle` 构造 + 析构（第 22~39 行）

```cpp
// FCachedFileHandle：带缓冲的文件句柄，包住底层句柄做读写缓存。
class FCachedFileHandle : public IFileHandle
{
// 公有区。
public:
// 构造函数：保存底层句柄、读写权限，并初始化位置/大小。
	FCachedFileHandle(IFileHandle* InFileHandle, bool bInReadable, bool bInWritable)
// 初始化列表：保存底层句柄。
		: FileHandle(InFileHandle)
// 初始化列表：文件位置取底层当前 Tell。
		, FilePos(InFileHandle->Tell())
// 初始化列表：底层实际位置也同步。
		, TellPos(FilePos)
// 初始化列表：文件大小取底层 Size。
		, FileSize(InFileHandle->Size())
// 初始化列表：是否可写。
		, bWritable(bInWritable)
// 初始化列表：是否可读。
		, bReadable(bInReadable)
// 初始化列表：当前使用的缓存槽从 0 开始。
		, CurrentCache(0)
// 构造函数体开始。
	{
// 清空缓存（把 CacheStart / CacheEnd 置 -1）。
		FlushCache();
// 构造函数体结束。
	}
	
// 虚析构函数（空，TUniquePtr 自动释放底层句柄）。
	virtual ~FCachedFileHandle()
// 函数体开始。
	{
// 函数体结束。
	}
```

**本段新元素**

- `class FCachedFileHandle : public IFileHandle` —— 带缓存的文件句柄
- `FCachedFileHandle(IFileHandle*, bInReadable, bInWritable)` —— 构造函数
- `FileHandle(InFileHandle)` —— 保存底层句柄
- `FilePos(InFileHandle->Tell())` —— 逻辑位置取底层当前 Tell
- `TellPos(FilePos)` —— 底层实际位置同步
- `FileSize(InFileHandle->Size())` —— 文件大小取底层 Size
- `CurrentCache(0)` —— 当前缓存槽从 0 开始
- `FlushCache()` —— 清空缓存（把 CacheStart / CacheEnd 置 -1）
- `virtual ~FCachedFileHandle()` —— 虚析构（空，`TUniquePtr` 自动释放底层句柄）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 22~24 行 | 类声明 + 公有区 |
| 第 25~35 行 | 构造函数（7 个初始化 + 清缓存） |
| 第 37~39 行 | 虚析构 |

> 类比：像给旧水管装个水箱：进水管（底层句柄）接好，水箱（缓存）先清空。

### 三、`Tell` / `Seek` / `SeekFromEnd`（第 42~60 行）

```cpp
// Tell：返回当前逻辑位置（FilePos，不是底层实际位置）。
	virtual int64		Tell() override
// 函数体开始。
	{
// 返回 FilePos。
		return FilePos;
// 函数体结束。
	}

// Seek：跳转到指定位置。
	virtual bool		Seek(int64 NewPosition) override
// 函数体开始。
	{
// 越界检查：负数或超过文件大小都失败。
		if (NewPosition < 0 || NewPosition > FileSize)
// 条件体开始。
		{
// 返回 false。
			return false;
// 条件体结束。
		}
// 更新逻辑位置。
		FilePos=NewPosition;
// 返回成功。
		return true;
// 函数体结束。
	}

// SeekFromEnd：从文件末尾往前跳。
	virtual bool		SeekFromEnd(int64 NewPositionRelativeToEnd = 0) override
// 函数体开始。
	{
// 换算成绝对位置再调 Seek。
		return Seek(FileSize - NewPositionRelativeToEnd);
// 函数体结束。
	}
```

**本段新元素**

- `Tell()` —— 返回 `FilePos`（逻辑位置，不是底层实际位置）
- `Seek(NewPosition)` —— 越界检查（负数 / 超过文件大小）+ 更新 `FilePos`
- `SeekFromEnd(NewPositionRelativeToEnd)` —— 换算成绝对位置再调 `Seek`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 42~45 行 | `Tell` |
| 第 47~55 行 | `Seek`（越界检查） |
| 第 57~60 行 | `SeekFromEnd` |

> 类比：像看书签：Tell 是「现在第几页」，Seek 是「翻到第 N 页」，SeekFromEnd 是「从最后往前数 N 页」。

### 四、`Read` —— 核心缓存逻辑（第 62~137 行）

```cpp
// Read：读数据（带缓存）。
	virtual bool		Read(uint8* Destination, int64 BytesToRead) override
// 函数体开始。
	{
// 检查：不可读 / 负数 / 越界都失败。
		if (!bReadable || BytesToRead < 0 || (BytesToRead + FilePos > FileSize))
// 条件体开始。
		{
// 返回 false。
			return false;
// 条件体结束。
		}

// 读 0 字节直接成功。
		if (BytesToRead == 0)
// 条件体开始。
		{
// 返回 true。
			return true;
// 条件体结束。
		}

// 结果标志。
		bool Result = false;
// 如果要读的字节数超过缓存大小（大块读）。
		if (BytesToRead > BufferCacheSize) // reading more than we cache
// 条件体开始。
		{
// 说明：如果当前位置在缓存里，先把缓存剩余部分拷出去。
			// if the file position is within the cache, copy out the remainder of the cache
// 取当前位置对应的缓存槽索引。
			int32 CacheIndex=GetCacheIndex(FilePos);
// 如果索引有效（在缓存范围内）。
			if (CacheIndex < CacheCount)
// 条件体开始。
			{
// 缓存里还剩多少字节。
				int64 CopyBytes = CacheEnd[CacheIndex]-FilePos;
// 从缓存拷到目标。
				FMemory::Memcpy(Destination, BufferCache[CacheIndex]+(FilePos-CacheStart[CacheIndex]), CopyBytes);
// 逻辑位置前进。
				FilePos += CopyBytes;
// 剩余待读字节减少。
				BytesToRead -= CopyBytes;
// 目标指针前进。
				Destination += CopyBytes;
// 条件体结束。
			}

// 底层跳到当前位置。
			if (InnerSeek(FilePos))
// 条件体开始。
			{
// 直接从底层读剩余字节。
				Result = InnerRead(Destination, BytesToRead);
// 条件体结束。
			}
// 如果读成功。
			if (Result)
// 条件体开始。
			{
// 逻辑位置前进。
				FilePos += BytesToRead;
// 条件体结束。
			}
// 条件体结束。
		}
// 否则（小块读，走缓存）。
		else
// 否则体开始。
		{
// 先假定成功。
			Result = true;
			
// 循环直到读完或失败。
			while (BytesToRead && Result) 
// 循环体开始。
			{
// 取当前位置对应的缓存槽索引。
				uint32 CacheIndex=GetCacheIndex(FilePos);
// 如果索引超出缓存范围（说明缓存里没有）。
				if (CacheIndex > CacheCount)
// 条件体开始。
				{
// 说明：需要更新缓存。
					// need to update the cache
// 把位置对齐到缓存块边界。
					uint64 AlignedFilePos=FilePos&BufferSizeMask; // Aligned Version
// 要读多少（不超过缓存大小，也不超过文件末尾）。
					uint64 SizeToRead=FMath::Min<uint64>(BufferCacheSize, FileSize-AlignedFilePos);
// 底层跳到对齐位置。
					InnerSeek(AlignedFilePos);
// 读一整块到缓存。
					Result = InnerRead(BufferCache[CurrentCache], SizeToRead);

// 如果读成功。
					if (Result)
// 条件体开始。
					{
// 记录缓存起始位置。
						CacheStart[CurrentCache] = AlignedFilePos;
// 记录缓存结束位置。
						CacheEnd[CurrentCache] = AlignedFilePos+SizeToRead;
// 索引指向刚填的槽。
						CacheIndex = CurrentCache;
// 说明：移到下一个缓存槽（轮转）。
						// move to next cache for update
// 当前槽 +1。
						CurrentCache++;
// 取模回绕（2 个槽轮流用）。
						CurrentCache%=CacheCount;
// 条件体结束。
					}
// 条件体结束。
				}

// 说明：从缓存拷到目标。
				// copy from the cache to the destination
// 如果读成功。
				if (Result)
// 条件体开始。
				{
// 说明：静态分析器看不到这个约束，改代码时务必保证不越界。
					// 静态分析器看不到这个约束 —— 如果这段代码有改动，务必保证不越界！
// 告诉分析器：索引一定小于 CacheCount。
					CA_ASSUME(CacheIndex < CacheCount);
// 实际能拷多少（不超过剩余待读，也不超过缓存末尾）。
					uint64 CorrectedBytesToRead=FMath::Min<uint64>(BytesToRead, CacheEnd[CacheIndex]-FilePos);
// 从缓存拷到目标。
					FMemory::Memcpy(Destination, BufferCache[CacheIndex]+(FilePos-CacheStart[CacheIndex]), CorrectedBytesToRead);
// 逻辑位置前进。
					FilePos += CorrectedBytesToRead;
// 目标指针前进。
					Destination += CorrectedBytesToRead;
// 剩余待读减少。
					BytesToRead -= CorrectedBytesToRead;
// 条件体结束。
				}
// 循环体结束。
			}
// 否则体结束。
		}
// 返回结果。
		return Result;
// 函数体结束。
	}
```

**本段新元素**

- `Read(Destination, BytesToRead)` —— 读数据（带缓存）
- **前置检查**：不可读 / 负数 / 越界 → false
- **读 0 字节** → 直接 true
- **大块读**（`BytesToRead > BufferCacheSize`）：先把缓存剩余部分拷出去，再直读底层
- **小块读**（走缓存）：循环直到读完
- `GetCacheIndex(FilePos)` —— 查位置落在哪个缓存槽
- `CacheIndex > CacheCount` —— 缓存未命中，需要填新块
- `FilePos & BufferSizeMask` —— 把位置对齐到 64KB 边界
- `FMath::Min<uint64>(BufferCacheSize, FileSize-AlignedFilePos)` —— 要读多少（不超过文件末尾）
- `InnerSeek(AlignedFilePos)` + `InnerRead(BufferCache[CurrentCache], SizeToRead)` —— 读一整块到缓存
- `CurrentCache++` / `CurrentCache %= CacheCount` —— 轮转到下一个槽
- `CA_ASSUME(CacheIndex < CacheCount)` —— 告诉静态分析器索引一定有效
- `FMath::Min<uint64>(BytesToRead, CacheEnd[CacheIndex]-FilePos)` —— 实际能拷多少
- `FMemory::Memcpy(...)` —— 从缓存拷到目标

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 62~72 行 | 前置检查 + 0 字节快速返回 |
| 第 74~96 行 | 大块读分支（先拷缓存剩余 + 直读底层） |
| 第 97~135 行 | 小块读分支（循环走缓存） |
| 第 103~121 行 | 缓存未命中 → 填新块 + 轮转 |
| 第 123~133 行 | 从缓存拷到目标 |
| 第 136~137 行 | 返回结果 |

> 类比：像喝水：大杯（大块读）直接从水桶倒，小口（小块读）从杯子（缓存）喝，杯子空了才去水桶续。

### 五、`ReadAt` / `Write` / `Size` / `Flush` / `Truncate` / `ShrinkBuffers`（第 139~201 行）

```cpp
// ReadAt：从指定偏移读（不走缓存）。
	virtual bool ReadAt(uint8* Destination, int64 BytesToRead, int64 Offset) override
// 函数体开始。
	{
// 说明：并发读不会被缓存，直接转给底层。
		// concurrent reads won't be cached yet
// 转调底层 ReadAt。
		return FileHandle->ReadAt(Destination, BytesToRead, Offset);
// 函数体结束。
	}

// Write：写数据。
	virtual bool		Write(const uint8* Source, int64 BytesToWrite) override
// 函数体开始。
	{
// 检查：不可写 / 负数都失败。
		if (!bWritable || BytesToWrite < 0)
// 条件体开始。
		{
// 返回 false。
			return false;
// 条件体结束。
		}

// 写 0 字节直接成功。
		if (BytesToWrite == 0)
// 条件体开始。
		{
// 返回 true。
			return true;
// 条件体结束。
		}

// 底层跳到当前位置。
		InnerSeek(FilePos);
// 调底层写。
		bool Result = FileHandle->Write(Source, BytesToWrite);
// 如果写成功。
		if (Result)
// 条件体开始。
		{
// 逻辑位置前进。
			FilePos += BytesToWrite;
// 文件大小取较大值（可能扩展了文件）。
			FileSize = FMath::Max<int64>(FilePos, FileSize);
// 清空缓存（写后缓存失效）。
			FlushCache();
// 底层实际位置同步。
			TellPos = FilePos;
// 条件体结束。
		}
// 返回结果。
		return Result;
// 函数体结束。
	}

// Size：返回文件大小。
	virtual int64		Size() override
// 函数体开始。
	{
// 返回 FileSize。
		return FileSize;
// 函数体结束。
	}

// Flush：刷新到磁盘。
	virtual bool		Flush(const bool bFullFlush = false) override
// 函数体开始。
	{
// 只有可写才刷。
		if (bWritable)
// 条件体开始。
		{
// 转调底层 Flush。
			return FileHandle->Flush(bFullFlush);
// 条件体结束。
		}
// 不可写返回 false。
		return false;
// 函数体结束。
	}

// Truncate：截断文件到指定大小。
	virtual bool		Truncate(int64 NewSize) override
// 函数体开始。
	{
// 只有可写才截断。
		if (bWritable)
// 条件体开始。
		{
// 调底层截断。
			if (FileHandle->Truncate(NewSize))
// 条件体开始。
			{
// 清空缓存。
				FlushCache();
// 位置和底层实际位置都同步。
				FilePos = TellPos = FileHandle->Tell();
// 文件大小重新取。
				FileSize = FileHandle->Size();
// 返回成功。
				return true;
// 条件体结束。
			}
// 条件体结束。
		}
// 返回失败。
		return false;
// 函数体结束。
	}

// ShrinkBuffers：收缩内部缓冲。
	virtual void ShrinkBuffers() override
// 函数体开始。
	{
// 转调底层。
		FileHandle->ShrinkBuffers();
// 函数体结束。
	}
```

**本段新元素**

- `ReadAt(Destination, BytesToRead, Offset)` —— 从指定偏移读（**不走缓存**，直接转底层）
- `Write(Source, BytesToWrite)` —— 写数据（写后 `FlushCache()` + 更新 `FileSize`）
- `FMath::Max<int64>(FilePos, FileSize)` —— 文件大小取较大值（可能扩展了文件）
- `Size()` —— 返回 `FileSize`
- `Flush(bFullFlush)` —— 只有可写才转调底层
- `Truncate(NewSize)` —— 截断后清缓存 + 重新同步位置和大小
- `ShrinkBuffers()` —— 转调底层

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 139~143 行 | `ReadAt`（不走缓存） |
| 第 145~167 行 | `Write`（写后清缓存） |
| 第 169~172 行 | `Size` |
| 第 174~181 行 | `Flush` |
| 第 183~196 行 | `Truncate` |
| 第 198~201 行 | `ShrinkBuffers` |

> 类比：像写作业：写完要擦掉草稿（清缓存），本子变厚了要更新页数（FileSize）。

### 六、私有常量 + 4 个辅助函数（第 203~248 行）

```cpp
// 私有区。
private:

// BufferCacheSize：每个缓存块 64KB（引擎注释说是性能最佳值）。
	static constexpr uint32 BufferCacheSize = 64 * 1024; // 看起来是性能最佳的经验值
// BufferSizeMask：用于把位置对齐到 64KB 边界的掩码。
	static constexpr uint64 BufferSizeMask  = ~((uint64)BufferCacheSize-1);
// CacheCount：缓存槽数量（2 个，轮转使用）。
	static constexpr uint32	CacheCount		= 2;

// InnerSeek：底层跳转（带位置缓存优化）。
	bool InnerSeek(uint64 Pos)
// 函数体开始。
	{
// 如果目标位置就是底层当前位置。
		if (Pos==TellPos) 
// 条件体开始。
		{
// 直接返回成功（省一次系统调用）。
			return true;
// 条件体结束。
		}
// 调底层 Seek。
		bool bOk=FileHandle->Seek(Pos);
// 如果成功。
		if (bOk)
// 条件体开始。
		{
// 更新底层实际位置。
			TellPos=Pos;
// 条件体结束。
		}
// 返回结果。
		return bOk;
// 函数体结束。
	}
// InnerRead：底层读（带位置更新）。
	bool InnerRead(uint8* Dest, uint64 BytesToRead)
// 函数体开始。
	{
// 调底层 Read。
		if (FileHandle->Read(Dest, BytesToRead))
// 条件体开始。
		{
// 底层实际位置前进。
			TellPos += BytesToRead;
// 返回成功。
			return true;
// 条件体结束。
		}
// 返回失败。
		return false;
// 函数体结束。
	}
// GetCacheIndex：查位置落在哪个缓存槽。
	int32 GetCacheIndex(int64 Pos) const
// 函数体开始。
	{
// 遍历所有缓存槽。
		for (uint32 i=0; i<UE_ARRAY_COUNT(CacheStart); ++i)
// 循环体开始。
		{
// 如果位置在 [CacheStart, CacheEnd) 区间内。
			if (Pos >= CacheStart[i] && Pos < CacheEnd[i]) 
// 条件体开始。
			{
// 返回该槽索引。
				return i;
// 条件体结束。
			}
// 循环体结束。
		}
// 没命中返回 CacheCount+1（表示无效）。
		return CacheCount+1;
// 函数体结束。
	}
// FlushCache：清空所有缓存槽。
	void FlushCache() 
// 函数体开始。
	{
// 遍历所有槽。
		for (uint32 i=0; i<CacheCount; ++i)
// 循环体开始。
		{
// 起止都置 -1（表示空）。
			CacheStart[i] = CacheEnd[i] = -1;
// 循环体结束。
		}
// 函数体结束。
	}
```

**本段新元素**

- `static constexpr uint32 BufferCacheSize = 64 * 1024` —— 每个缓存块 64KB（引擎注释说是性能最佳值）
- `static constexpr uint64 BufferSizeMask = ~((uint64)BufferCacheSize-1)` —— 对齐掩码
- `static constexpr uint32 CacheCount = 2` —— 缓存槽数量（2 个轮转）
- `InnerSeek(Pos)` —— 底层跳转（**带位置缓存优化**：位置相同就跳过系统调用）
- `InnerRead(Dest, BytesToRead)` —— 底层读（带 `TellPos` 更新）
- `GetCacheIndex(Pos)` —— 查位置落在哪个槽（遍历 `CacheStart` / `CacheEnd`）
- `FlushCache()` —— 把所有槽的起止置 -1（表示空）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 203~207 行 | 3 个编译期常量 |
| 第 209~221 行 | `InnerSeek`（位置缓存优化） |
| 第 222~230 行 | `InnerRead` |
| 第 231~241 行 | `GetCacheIndex` |
| 第 242~248 行 | `FlushCache` |

> 类比：像仓库管理员的四个工具：量尺（常量）、找货（InnerSeek）、取货（InnerRead）、查货架（GetCacheIndex）、清仓（FlushCache）。

### 七、10 个成员变量（第 250~260 行）

```cpp
// FileHandle：底层文件句柄（独占指针，自动释放）。
	TUniquePtr<IFileHandle>	FileHandle;
// FilePos：逻辑位置（可能和底层实际位置不同，因为有缓存）。
	int64					FilePos; /* 文件流里的期望位置，因为有缓存，可能和底层实际位置不同 */
// TellPos：底层实际位置（可能和逻辑位置不同）。
	int64					TellPos; /* 文件里的实际位置，可能和逻辑位置不同 */
// FileSize：文件大小。
	int64					FileSize;
// bWritable：是否可写。
	bool					bWritable;
// bReadable：是否可读。
	bool					bReadable;
// BufferCache：2 个 64KB 缓存块。
	uint8					BufferCache[CacheCount][BufferCacheSize];
// CacheStart：每个缓存块的起始文件位置。
	int64					CacheStart[CacheCount];
// CacheEnd：每个缓存块的结束文件位置。
	int64					CacheEnd[CacheCount];
// CurrentCache：当前正在填的缓存槽索引。
	int32					CurrentCache;
// 类结束。
};
```

**本段新元素**

- `TUniquePtr<IFileHandle> FileHandle` —— 底层文件句柄（独占指针，自动释放）
- `int64 FilePos` —— **逻辑位置**（可能和底层实际位置不同，因为有缓存）
- `int64 TellPos` —— **底层实际位置**（可能和逻辑位置不同）
- `int64 FileSize` —— 文件大小
- `bool bWritable` / `bool bReadable` —— 读写权限
- `uint8 BufferCache[2][64KB]` —— 2 个 64KB 缓存块
- `int64 CacheStart[2]` / `int64 CacheEnd[2]` —— 每块的起止文件位置
- `int32 CurrentCache` —— 当前正在填的槽索引

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 250 行 | 底层句柄 |
| 第 251~253 行 | 3 个位置/大小 |
| 第 254~255 行 | 2 个权限标志 |
| 第 256~259 行 | 缓存相关 4 个 |
| 第 260 行 | 类结束 |

> 类比：像水箱的仪表盘：进水管（FileHandle）、水位（FilePos）、实际水位（TellPos）、容量（FileSize）、两个水箱（BufferCache）。

### 八、`FCachedReadPlatformFile` 声明 + 类型名 + 构造（第 262~280 行）

```cpp
// FCachedReadPlatformFile：带读缓存的平台文件包装器。
class FCachedReadPlatformFile : public IPlatformFile
// 类声明开始。
{
// LowerLevel：被包装的下一层平台文件。
	IPlatformFile*		LowerLevel;
// 公有区。
public:
// GetTypeName：返回类型名。
	static const TCHAR* GetTypeName()
// 函数体开始。
	{
// 返回 CachedReadFile。
		return TEXT("CachedReadFile");
// 函数体结束。
	}

// 默认构造函数：LowerLevel 置空。
	FCachedReadPlatformFile()
// 初始化列表：LowerLevel 为空。
		: LowerLevel(nullptr)
// 函数体开始。
	{
// 函数体结束。
	}

// 说明：为了让基类的重载可见（否则会被隐藏）。
	//~ 为了让基类的重载可见（否则会被隐藏）
// 引入基类的 IterateDirectory。
	using IPlatformFile::IterateDirectory;
// 引入基类的 IterateDirectoryRecursively。
	using IPlatformFile::IterateDirectoryRecursively;
// 引入基类的 IterateDirectoryStat。
	using IPlatformFile::IterateDirectoryStat;
// 引入基类的 IterateDirectoryStatRecursively。
	using IPlatformFile::IterateDirectoryStatRecursively;
```

**本段新元素**

- `class FCachedReadPlatformFile : public IPlatformFile` —— 带读缓存的平台文件包装器
- `IPlatformFile* LowerLevel` —— 被包装的下一层平台文件
- `static GetTypeName()` —— 返回 `"CachedReadFile"`
- `FCachedReadPlatformFile()` —— 默认构造（`LowerLevel` 置空）
- `using IPlatformFile::IterateDirectory;` 等 4 个 —— **让基类重载可见**（否则会被隐藏）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 262~265 行 | 类声明 + 成员 |
| 第 266~269 行 | `GetTypeName` |
| 第 271~274 行 | 默认构造 |
| 第 276~280 行 | 4 个 `using` |

> 类比：像给水管装个转接头：外面看还是水管（IPlatformFile），里面多了一层过滤（缓存）。

### 九、`Initialize` + `ShouldBeUsed`（启用条件）（第 282~316 行）

```cpp
// Initialize：初始化包装器。
	virtual bool Initialize(IPlatformFile* Inner, const TCHAR* CommandLineParam) override
// 函数体开始。
	{
// 说明：Inner 是必需的。
		// Inner 是必需的。
// 断言 Inner 非空。
		check(Inner != nullptr);
// 保存下一层。
		LowerLevel = Inner;
// 返回是否成功。
		return !!LowerLevel;
// 函数体结束。
	}
// ShouldBeUsed：判断是否应该启用本包装器。
	virtual bool ShouldBeUsed(IPlatformFile* Inner, const TCHAR* CmdLine) const override
// 函数体开始。
	{
// 如果平台没定义 PLATFORM_PROVIDES_FILE_CACHE。
#ifndef PLATFORM_PROVIDES_FILE_CACHE
// 默认定义为 0。
#define PLATFORM_PROVIDES_FILE_CACHE 0
// 条件编译结束。
#endif
// 说明：平台自己已有文件缓存时默认关闭。
		// 平台自己已有文件缓存时默认关闭
// 平台是否自带文件缓存。
		bool bPlatformProvidesFileCache = PLATFORM_PROVIDES_FILE_CACHE;
// 是否是 Windows 平台。
		bool bPlatformWindows = PLATFORM_WINDOWS;
// 结果：平台无缓存 && 非 Windows && 需要 cook 数据。
		bool bResult = !bPlatformProvidesFileCache && !bPlatformWindows && FPlatformProperties::RequiresCookedData();

// 说明：桌面平台允许在「加载更快」和「内存更少」之间选择。
		// 桌面平台允许在「加载更快」和「内存更少」之间选择。
// 说明：不能放配置里，因为那时配置还没读。
		// 注意：不能放配置里，因为那时配置还没读。
// 如果是桌面平台或平台自带缓存。
#if (PLATFORM_DESKTOP || PLATFORM_PROVIDES_FILE_CACHE)
// 代码块开始。
		{
// 如果命令行有 -NoCachedReadFile。
			if (FParse::Param(CmdLine, TEXT("NoCachedReadFile")))
// 条件体开始。
			{
// 关闭缓存。
				bResult = false;
// 条件体结束。
			}
// 否则如果命令行有 -CachedReadFile。
			else if (FParse::Param(CmdLine, TEXT("CachedReadFile")))
// 条件体开始。
			{
// 开启缓存。
				bResult = true;
// 条件体结束。
			}

// 打日志：是否使用缓存包装器。
			UE_LOG(LogPlatformFile, Verbose, TEXT("%s cached read wrapper"), bResult ? TEXT("Using") : TEXT("Not using"));
// 代码块结束。
		}
// 条件编译结束。
#endif
// 返回结果。
		return bResult;
// 函数体结束。
	}
```

**本段新元素**

- `Initialize(Inner, CommandLineParam)` —— 保存下一层（`Inner` 必需）
- `check(Inner != nullptr)` —— 断言 `Inner` 非空
- `ShouldBeUsed(Inner, CmdLine)` —— 判断是否启用本包装器
- `PLATFORM_PROVIDES_FILE_CACHE` —— 平台是否自带文件缓存（默认 0）
- `PLATFORM_WINDOWS` —— 是否 Windows
- `FPlatformProperties::RequiresCookedData()` —— 是否需要 cook 数据
- `FParse::Param(CmdLine, TEXT("NoCachedReadFile"))` —— 命令行关闭
- `FParse::Param(CmdLine, TEXT("CachedReadFile"))` —— 命令行开启
- `UE_LOG(LogPlatformFile, Verbose, ...)` —— 打日志

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 282~288 行 | `Initialize` |
| 第 289~293 行 | `PLATFORM_PROVIDES_FILE_CACHE` 默认值 |
| 第 294~297 行 | 启用条件计算 |
| 第 299~301 行 | 桌面平台命令行开关 |
| 第 303~313 行 | 解析命令行 + 打日志 |
| 第 315~316 行 | 返回结果 |

> 类比：像门卫判断要不要开空调：平台自己有空调（自带缓存）就不开，Windows 也不开，只有需要 cook 数据时才开。

### 十、`GetLowerLevel` / `SetLowerLevel` / `GetName` + 10 个纯转调（第 317~368 行）

```cpp
// GetLowerLevel：取下一层平台文件。
	IPlatformFile* GetLowerLevel() override
// 函数体开始。
	{
// 返回 LowerLevel。
		return LowerLevel;
// 函数体结束。
	}
// SetLowerLevel：设置下一层平台文件。
	virtual void SetLowerLevel(IPlatformFile* NewLowerLevel) override
// 函数体开始。
	{
// 保存。
		LowerLevel = NewLowerLevel;
// 函数体结束。
	}
// GetName：返回本包装器的名字。
	virtual const TCHAR* GetName() const override
// 函数体开始。
	{
// 返回类型名。
		return FCachedReadPlatformFile::GetTypeName();
// 函数体结束。
	}
// FileExists：转调底层。
	virtual bool		FileExists(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->FileExists(Filename);
// 函数体结束。
	}
// FileSize：转调底层。
	virtual int64		FileSize(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->FileSize(Filename);
// 函数体结束。
	}
// DeleteFile：转调底层。
	virtual bool		DeleteFile(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteFile(Filename);
// 函数体结束。
	}
// IsReadOnly：转调底层。
	virtual bool		IsReadOnly(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IsReadOnly(Filename);
// 函数体结束。
	}
// MoveFile：转调底层。
	virtual bool		MoveFile(const TCHAR* To, const TCHAR* From) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->MoveFile(To, From);
// 函数体结束。
	}
// SetReadOnly：转调底层。
	virtual bool		SetReadOnly(const TCHAR* Filename, bool bNewReadOnlyValue) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->SetReadOnly(Filename, bNewReadOnlyValue);
// 函数体结束。
	}
// GetTimeStamp：转调底层。
	virtual FDateTime	GetTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetTimeStamp(Filename);
// 函数体结束。
	}
// SetTimeStamp：转调底层。
	virtual void		SetTimeStamp(const TCHAR* Filename, FDateTime DateTime) override
// 函数体开始。
	{
// 转调。
		LowerLevel->SetTimeStamp(Filename, DateTime);
// 函数体结束。
	}
// GetAccessTimeStamp：转调底层。
	virtual FDateTime	GetAccessTimeStamp(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetAccessTimeStamp(Filename);
// 函数体结束。
	}
// GetFilenameOnDisk：转调底层。
	virtual FString	GetFilenameOnDisk(const TCHAR* Filename) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetFilenameOnDisk(Filename);
// 函数体结束。
	}
```

**本段新元素**

- `GetLowerLevel()` —— 取下一层平台文件
- `SetLowerLevel(NewLowerLevel)` —— 设置下一层
- `GetName()` —— 返回类型名
- `FileExists` / `FileSize` / `DeleteFile` / `IsReadOnly` / `MoveFile` / `SetReadOnly` —— 6 个纯转调
- `GetTimeStamp` / `SetTimeStamp` / `GetAccessTimeStamp` —— 3 个时间转调
- `GetFilenameOnDisk` —— 1 个路径转调

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 317~328 行 | 3 个访问器 |
| 第 329~352 行 | 6 个文件操作转调 |
| 第 353~364 行 | 3 个时间转调 |
| 第 365~368 行 | `GetFilenameOnDisk` |

> 类比：像传话：外面问什么，直接转给下一层，自己不加料。

### 十一、`OpenRead` / `OpenWrite` —— 包一层缓存句柄（第 369~386 行）

```cpp
// OpenRead：打开文件读（返回带缓存的句柄）。
	virtual IFileHandle*	OpenRead(const TCHAR* Filename, bool bAllowWrite) override
// 函数体开始。
	{
// 先让底层打开。
		IFileHandle* InnerHandle=LowerLevel->OpenRead(Filename, bAllowWrite);
// 如果打开失败。
		if (!InnerHandle)
// 条件体开始。
		{
// 返回空。
			return nullptr;
// 条件体结束。
		}
// 包一层 FCachedFileHandle（可读不可写）。
		return new FCachedFileHandle(InnerHandle, true, false);
// 函数体结束。
	}
// OpenWrite：打开文件写（返回带缓存的句柄）。
	virtual IFileHandle*	OpenWrite(const TCHAR* Filename, bool bAppend = false, bool bAllowRead = false) override
// 函数体开始。
	{
// 先让底层打开。
		IFileHandle* InnerHandle=LowerLevel->OpenWrite(Filename, bAppend, bAllowRead);
// 如果打开失败。
		if (!InnerHandle)
// 条件体开始。
		{
// 返回空。
			return nullptr;
// 条件体结束。
		}
// 包一层 FCachedFileHandle（可写，读权限跟随参数）。
		return new FCachedFileHandle(InnerHandle, bAllowRead, true);
// 函数体结束。
	}
```

**本段新元素**

- `OpenRead(Filename, bAllowWrite)` —— 打开读，返回 `FCachedFileHandle`（可读不可写）
- `OpenWrite(Filename, bAppend, bAllowRead)` —— 打开写，返回 `FCachedFileHandle`（可写，读权限跟随参数）
- `new FCachedFileHandle(InnerHandle, true, false)` —— 包一层（读模式）
- `new FCachedFileHandle(InnerHandle, bAllowRead, true)` —— 包一层（写模式）
- **打开失败返回 `nullptr`** —— 不包空句柄

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 369~377 行 | `OpenRead` |
| 第 378~386 行 | `OpenWrite` |

> 类比：像给水管装水表：打开时自动装上（包一层），之后每次用水都经过它（走缓存）。

### 十二、8 个目录转调 + `FindFiles` ×2（第 387~426 行）

```cpp
// DirectoryExists：转调底层。
	virtual bool		DirectoryExists(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DirectoryExists(Directory);
// 函数体结束。
	}
// CreateDirectory：转调底层。
	virtual bool		CreateDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CreateDirectory(Directory);
// 函数体结束。
	}
// DeleteDirectory：转调底层。
	virtual bool		DeleteDirectory(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteDirectory(Directory);
// 函数体结束。
	}
// GetStatData：转调底层。
	virtual FFileStatData GetStatData(const TCHAR* FilenameOrDirectory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->GetStatData(FilenameOrDirectory);
// 函数体结束。
	}
// IterateDirectory：转调底层。
	virtual bool		IterateDirectory(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectory(Directory, Visitor);
// 函数体结束。
	}
// IterateDirectoryRecursively：转调底层。
	virtual bool		IterateDirectoryRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryRecursively(Directory, Visitor);
// 函数体结束。
	}
// IterateDirectoryStat：转调底层。
	virtual bool		IterateDirectoryStat(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryStat(Directory, Visitor);
// 函数体结束。
	}
// IterateDirectoryStatRecursively：转调底层。
	virtual bool		IterateDirectoryStatRecursively(const TCHAR* Directory, IPlatformFile::FDirectoryStatVisitor& Visitor) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->IterateDirectoryStatRecursively(Directory, Visitor);
// 函数体结束。
	}
// FindFiles：转调底层（注意：没有 override 关键字）。
	virtual void		FindFiles(TArray<FString>& FoundFiles, const TCHAR* Directory, const TCHAR* FileExtension)
// 函数体开始。
	{
// 转调。
		return LowerLevel->FindFiles(FoundFiles, Directory, FileExtension);
// 函数体结束。
	}
// FindFilesRecursively：转调底层（注意：没有 override 关键字）。
	virtual void		FindFilesRecursively(TArray<FString>& FoundFiles, const TCHAR* Directory, const TCHAR* FileExtension)
// 函数体开始。
	{
// 转调。
		return LowerLevel->FindFilesRecursively(FoundFiles, Directory, FileExtension);
// 函数体结束。
	}
```

**本段新元素**

- `DirectoryExists` / `CreateDirectory` / `DeleteDirectory` —— 3 个目录转调
- `GetStatData` —— 元数据转调
- `IterateDirectory` / `IterateDirectoryRecursively` / `IterateDirectoryStat` / `IterateDirectoryStatRecursively` —— 4 个遍历转调
- `FindFiles(FoundFiles, Directory, FileExtension)` —— **没有 `override`**（引擎原文如此）
- `FindFilesRecursively(...)` —— **没有 `override`**（引擎原文如此）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 387~402 行 | 4 个目录/元数据转调 |
| 第 403~418 行 | 4 个遍历转调 |
| 第 419~422 行 | `FindFiles`（无 override） |
| 第 423~426 行 | `FindFilesRecursively`（无 override） |

> 类比：像目录查询台：问什么答什么，全部转给下一层。

### 十三、4 个复制/建树转调 + 3 个路径/消息转调 + 3 个异步转调（第 427~468 行）

```cpp
// DeleteDirectoryRecursively：转调底层。
	virtual bool		DeleteDirectoryRecursively(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->DeleteDirectoryRecursively(Directory);
// 函数体结束。
	}
// CopyFile：转调底层。
	virtual bool		CopyFile(const TCHAR* To, const TCHAR* From, EPlatformFileRead ReadFlags = EPlatformFileRead::None, EPlatformFileWrite WriteFlags = EPlatformFileWrite::None) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CopyFile(To, From, ReadFlags, WriteFlags);
// 函数体结束。
	}
// CreateDirectoryTree：转调底层。
	virtual bool		CreateDirectoryTree(const TCHAR* Directory) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CreateDirectoryTree(Directory);
// 函数体结束。
	}
// CopyDirectoryTree：转调底层。
	virtual bool		CopyDirectoryTree(const TCHAR* DestinationDirectory, const TCHAR* Source, bool bOverwriteAllExisting) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->CopyDirectoryTree(DestinationDirectory, Source, bOverwriteAllExisting);
// 函数体结束。
	}
// ConvertToAbsolutePathForExternalAppForRead：转调底层。
	virtual FString		ConvertToAbsolutePathForExternalAppForRead( const TCHAR* Filename ) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->ConvertToAbsolutePathForExternalAppForRead(Filename);
// 函数体结束。
	}
// ConvertToAbsolutePathForExternalAppForWrite：转调底层。
	virtual FString		ConvertToAbsolutePathForExternalAppForWrite( const TCHAR* Filename ) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->ConvertToAbsolutePathForExternalAppForWrite(Filename);
// 函数体结束。
	}
// SendMessageToServer：转调底层。
	virtual bool		SendMessageToServer(const TCHAR* Message, IFileServerMessageHandler* Handler) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->SendMessageToServer(Message, Handler);
// 函数体结束。
	}
// OpenAsyncRead：转调底层。
	virtual IAsyncReadFileHandle* OpenAsyncRead(const TCHAR* Filename, bool bAllowWrite = false) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->OpenAsyncRead(Filename, bAllowWrite);
// 函数体结束。
	}
// OpenMappedEx：转调底层。
	virtual FOpenMappedResult OpenMappedEx(const TCHAR* Filename, EOpenReadFlags OpenOptions = EOpenReadFlags::None, int64 MaximumSize = 0) override
// 函数体开始。
	{
// 转调。
		return LowerLevel->OpenMappedEx(Filename, OpenOptions, MaximumSize);
// 函数体结束。
	}
// SetAsyncMinimumPriority：转调底层。
	virtual void SetAsyncMinimumPriority(EAsyncIOPriorityAndFlags MinPriority) override
// 函数体开始。
	{
// 转调。
		LowerLevel->SetAsyncMinimumPriority(MinPriority);
// 函数体结束。
	}

// 类结束。
};
```

**本段新元素**

- `DeleteDirectoryRecursively` —— 递归删除转调
- `CopyFile(To, From, ReadFlags, WriteFlags)` —— 复制文件转调
- `CreateDirectoryTree` / `CopyDirectoryTree` —— 建树/复制树转调
- `ConvertToAbsolutePathForExternalAppForRead` / `ForWrite` —— 2 个路径转换转调
- `SendMessageToServer(Message, Handler)` —— 服务器消息转调
- `OpenAsyncRead(Filename, bAllowWrite)` —— 异步读转调
- `OpenMappedEx(Filename, OpenOptions, MaximumSize)` —— 映射文件转调
- `SetAsyncMinimumPriority(MinPriority)` —— 异步优先级转调

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 427~442 行 | 4 个复制/建树转调 |
| 第 443~454 行 | 3 个路径/消息转调 |
| 第 455~466 行 | 3 个异步转调 |
| 第 468 行 | 类结束 |

> 类比：像总机转接：所有电话都转给下一层，自己只负责「读」这一件事。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 14 个 include | 1~20 | 版权头 + 14 个 include |
| `FCachedFileHandle` 构造 + 析构 | 22~39 | `FCachedFileHandle` 构造 + 析构 |
| `Tell` / `Seek` / `SeekFromEnd` | 42~60 | `Tell` / `Seek` / `SeekFromEnd` |
| `Read` —— 核心缓存逻辑 | 62~137 | `Read` —— 核心缓存逻辑 |
| `ReadAt` / `Write` / `Size` / `Flush` / `Truncate` / `ShrinkBuffers` | 139~201 | `ReadAt` / `Write` / `Size` / `Flush` / `Truncate` / `ShrinkBuffers` |
| 私有常量 + 4 个辅助函数 | 203~248 | 私有常量 + 4 个辅助函数 |
| 10 个成员变量 | 250~260 | 10 个成员变量 |
| `FCachedReadPlatformFile` 声明 + 类型名 + 构造 | 262~280 | `FCachedReadPlatformFile` 声明 + 类型名 + 构造 |
| `Initialize` + `ShouldBeUsed`（启用条件） | 282~316 | `Initialize` + `ShouldBeUsed`（启用条件） |
| `GetLowerLevel` / `SetLowerLevel` / `GetName` + 10 个纯转调 | 317~368 | `GetLowerLevel` / `SetLowerLevel` / `GetName` + 10 个纯转调 |
| `OpenRead` / `OpenWrite` —— 包一层缓存句柄 | 369~386 | `OpenRead` / `OpenWrite` —— 包一层缓存句柄 |
| 8 个目录转调 + `FindFiles` ×2 | 387~426 | 8 个目录转调 + `FindFiles` ×2 |
| 4 个复制/建树转调 + 3 个路径/消息转调 + 3 个异步转调 | 427~468 | 4 个复制/建树转调 + 3 个路径/消息转调 + 3 个异步转调 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `class X : public Y` | 继承 | `class FCachedFileHandle : public IFileHandle` |
| `static constexpr uint32 X = 值` | 编译期常量 | `static constexpr uint32 BufferCacheSize = 64 * 1024;` |
| `T X[N][M]` | 二维数组 | `uint8 BufferCache[CacheCount][BufferCacheSize];` |
| `X & Mask` | 位与（对齐） | `FilePos & BufferSizeMask` |
| `X %= N` | 取模回绕 | `CurrentCache %= CacheCount;` |
| `FMath::Min<uint64>(A, B)` | 模板版取小 | `FMath::Min<uint64>(BufferCacheSize, FileSize-AlignedFilePos)` |
| `CA_ASSUME(cond)` | 告诉静态分析器条件成立 | `CA_ASSUME(CacheIndex < CacheCount);` |
| `using Base::Method;` | 让基类重载可见 | `using IPlatformFile::IterateDirectory;` |
| `FParse::Param(CmdLine, TEXT("X"))` | 解析命令行开关 | `FParse::Param(CmdLine, TEXT("CachedReadFile"))` |
| `UE_LOG(Category, Verbosity, Format, ...)` | 打日志 | `UE_LOG(LogPlatformFile, Verbose, TEXT("%s cached read wrapper"), ...)` |
| `TUniquePtr<X>` | 独占智能指针 | `TUniquePtr<IFileHandle> FileHandle;` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 468 行 | 900 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 421 行 | 421 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| 行内块注释 `/* */` | 英文原文 | 中文 | 已译 |
| `FindFiles` 无 `override` | 引擎原文如此 | 照抄不改 | 引擎遗漏，不补 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`（900 行）
2. 脚本验收：`codeOurs=421 / codeEngine=421 / diff=0`
3. 确认后下一个：`HAL\IPlatformFileLogWrapper.h`（22959 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`（468 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileCachedWrapper.h`（900 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\13_IPlatformFileCachedWrapper.h_总md.md`