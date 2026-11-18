# 17 — `HAL/DiskUtilizationTracker.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`（**169 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`
> **✅ 本节可独立编译**——只 include `CoreTypes.h`（追踪开启时再加 3 个）。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

加载游戏时磁盘是瓶颈。你想知道「磁盘到底忙不忙、读得多快、有多少时间在空转」，就需要一个**磁盘利用率追踪器**。

`DiskUtilizationTracker.h` 提供：

1. **`FDiskUtilizationTracker`** —— 追踪器本体，统计读取次数、字节数、寻道距离、IO 时间、空闲时间
2. **`UtilizationStats`** —— 一组统计数据 + 8 个计算函数（吞吐率、空闲占比…）
3. **`FScopedDiskUtilizationTracker`** —— RAII 作用域，构造时 `StartRead()`、析构时 `FinishRead()`

**说个场景**：你在异步加载代码里写

```cpp
FScopedDiskUtilizationTracker Tracker(BytesRead, SeekDistance);
// ... 读文件 ...
// 出作用域自动 FinishRead()，统计自动累加
```

**关键设计**：整个文件被 `#if TRACK_DISK_UTILIZATION` 包住。**默认关闭**（第 6 行 `#define TRACK_DISK_UTILIZATION 0`），关闭时 `FScopedDiskUtilizationTracker` 变成**空壳**（第 162~167 行），构造函数什么都不做 —— 零开销。

> 🔑 **一句话**：`DiskUtilizationTracker.h` = **磁盘的「行车记录仪」**，默认不装（零开销），需要时打开开关就能记录读写吞吐。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、开关宏 + include（第 1~9 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 如果外部没定义 TRACK_DISK_UTILIZATION（是否追踪磁盘利用率）。
#ifndef TRACK_DISK_UTILIZATION
// 默认定义为 0，即默认关闭追踪。
#define TRACK_DISK_UTILIZATION 0
// 条件编译结束。
#endif

// 引入基础类型层：uint32 / uint64 / int32 都来自这里。
#include "CoreTypes.h"
```

**本段新元素**

- `TRACK_DISK_UTILIZATION` —— 是否追踪磁盘利用率，默认 0（关闭）
- `#ifndef X` / `#define X 0` / `#endif` —— 「外部没定义就给默认值」的标准写法
- `CoreTypes.h` —— `uint32` / `uint64` / `int32`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~7 行 | TRACK_DISK_UTILIZATION 默认 0 |
| 第 9 行 | 引入基础类型 |

> 类比：像电器的总开关：默认关着，要用才拨上去。

### 二、追踪开启时的额外 include + CSV 分类（第 11~20 行）

```cpp
// 如果开启了磁盘利用率追踪。
#if TRACK_DISK_UTILIZATION
// 引入平台杂项接口：FPlatformTime 等。
#include "HAL/PlatformMisc.h"
// 引入线程安全布尔：bResetShortTermStats 的类型。
#include "HAL/ThreadSafeBool.h"
// 引入 CSV 性能分析器：CSV_DECLARE_CATEGORY_EXTERN 宏。
#include "ProfilingDebugging/CsvProfiler.h"

// 声明一个 CSV 分类 DiskIO，用于在 CSV 里记录磁盘 IO 数据。
CSV_DECLARE_CATEGORY_EXTERN(DiskIO);

// 如果外部没定义 SPEW_DISK_UTILIZATION（是否打印磁盘利用率）。
#ifndef SPEW_DISK_UTILIZATION
// 默认定义为 0，即默认不打印。
#define SPEW_DISK_UTILIZATION 0
// 条件编译结束。
#endif // SPEW_DISK_UTILIZATION（是否打印磁盘利用率）
```

**本段新元素**

- `#if TRACK_DISK_UTILIZATION` —— 只有开启追踪才编译这段
- `HAL/PlatformMisc.h` —— `FPlatformTime`
- `HAL/ThreadSafeBool.h` —— `FThreadSafeBool`
- `ProfilingDebugging/CsvProfiler.h` —— `CSV_DECLARE_CATEGORY_EXTERN`
- `CSV_DECLARE_CATEGORY_EXTERN(DiskIO)` —— 声明 CSV 分类
- `SPEW_DISK_UTILIZATION` —— 是否打印磁盘利用率，默认 0

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 11 行 | 追踪开关判断 |
| 第 12~14 行 | 3 个额外 include |
| 第 16 行 | CSV 分类声明 |
| 第 18~20 行 | SPEW_DISK_UTILIZATION 默认 0 |

> 类比：像给记录仪插上电源和存储卡：只有开机才需要。

### 三、`UtilizationStats` 构造函数（第 22~33 行）

```cpp
// FDiskUtilizationTracker：磁盘利用率追踪器，统计读写次数、字节数、IO 时间、空闲时间。
struct FDiskUtilizationTracker
{
// UtilizationStats：一组利用率统计数据。
	struct UtilizationStats
	{
// 构造函数：所有计数器清零。
		UtilizationStats() :
// 总读取次数清零。
			TotalReads(0),
// 总寻道次数清零。
			TotalSeeks(0),
// 总读取字节数清零。
			TotalBytesRead(0),
// 总寻道距离清零。
			TotalSeekDistance(0),
// 总 IO 时间清零。
			TotalIOTime(0.0),
// 总空闲时间清零。
			TotalIdleTime(0.0)
// 构造函数体开始（空）。
		{}
```

**本段新元素**

- `struct FDiskUtilizationTracker` —— 追踪器本体
- `struct UtilizationStats` —— 一组统计数据
- 初始化列表 `: TotalReads(0), ...` —— 构造时清零

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 22~24 行 | 两个 struct 声明 |
| 第 26~33 行 | 构造函数 + 6 个字段清零 |

> 类比：像新买的秒表：先归零再开始计时。

### 四、4 个吞吐率计算函数（第 35~53 行）

```cpp
// 取整体吞吐率（字节/秒）。
		double GetOverallThroughputBS() const
		{
// 总时间大于 0 时用「总字节 / 总时间」，否则返回 0。
			return (TotalIOTime + TotalIdleTime) > 0.0 ? double(TotalBytesRead) / (TotalIOTime + TotalIdleTime) : 0.0;
		}

// 取整体吞吐率（兆字节/秒）。
		double GetOverallThroughputMBS() const
		{
// 字节/秒 除以 1024*1024 得到 MB/秒。
			return GetOverallThroughputBS() / (1024.0 * 1024.0);
		}

// 取读取吞吐率（字节/秒）。
		double GetReadThrougputBS() const
		{
// IO 时间大于 0 时用「总字节 / IO 时间」，否则返回 0。
			return TotalIOTime > 0.0 ? double(TotalBytesRead) / TotalIOTime : 0.0;
		}

// 取读取吞吐率（兆字节/秒）。
		double GetReadThrougputMBS() const
		{
// 字节/秒 除以 1024*1024 得到 MB/秒。
			return GetReadThrougputBS() / (1024.0 * 1024.0);
		}
```

**本段新元素**

- `GetOverallThroughputBS()` —— 整体吞吐率（字节/秒）
- `GetOverallThroughputMBS()` —— 整体吞吐率（MB/秒）
- `GetReadThrougputBS()` —— 读取吞吐率（字节/秒），注意引擎拼写是 `Througput` 少个 h
- `GetReadThrougputMBS()` —— 读取吞吐率（MB/秒）
- `1024.0 * 1024.0` —— 字节转 MB 的除数
- 三元运算符 `cond ? A : B` —— 防除零

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 35~38 行 | 整体吞吐率 BS |
| 第 40~43 行 | 整体吞吐率 MBS |
| 第 45~48 行 | 读取吞吐率 BS |
| 第 50~53 行 | 读取吞吐率 MBS |

> 类比：像车速表：一个显示「公里/小时」，一个显示「米/秒」，同一份数据两种单位。

### 五、3 个时间/占比计算函数（第 55~70 行）

```cpp
// 取总空闲时间（秒）。
		double GetTotalIdleTimeInSeconds() const
		{
// 直接返回空闲时间。
			return TotalIdleTime;
		}

// 取总 IO 时间（秒）。
		double GetTotalIOTimeInSeconds() const
		{
// 直接返回 IO 时间。
			return TotalIOTime;
		}

// 取空闲时间占比（百分比）。
		double GetPercentTimeIdle() const
		{
// 先算总时间。
			double TotalTime = TotalIOTime + TotalIdleTime;

// 总时间大于 0 时用「100 * 空闲时间 / 总时间」，否则返回 0。
			return TotalTime > 0.0 ? (100.0f * TotalIdleTime) / TotalTime : 0.0;
		}
```

**本段新元素**

- `GetTotalIdleTimeInSeconds()` —— 总空闲时间（秒）
- `GetTotalIOTimeInSeconds()` —— 总 IO 时间（秒）
- `GetPercentTimeIdle()` —— 空闲时间占比（百分比）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 55~58 行 | 总空闲时间 |
| 第 60~63 行 | 总 IO 时间 |
| 第 65~70 行 | 空闲占比 |

> 类比：像统计「一天里有多少小时在干活、多少小时在等」。

### 六、`Reset()` + `Dump()` + 6 个字段（第 72~92 行）

```cpp
// 重置所有计数器。
		void Reset()
		{
// 总读取次数清零。
			TotalReads = 0;
// 总寻道次数清零。
			TotalSeeks = 0;
// 总读取字节数清零。
			TotalBytesRead = 0;
// 总寻道距离清零。
			TotalSeekDistance = 0;
// 总 IO 时间清零。
			TotalIOTime = 0.0;
// 总空闲时间清零。
			TotalIdleTime = 0.0;
		}

// 把统计数据打印出来（实现在 .cpp 里）。
		void Dump() const;

// 总读取次数。
		uint64 TotalReads;
// 总寻道次数。
		uint64 TotalSeeks;

// 总读取字节数。
		uint64 TotalBytesRead;
// 总寻道距离。
		uint64 TotalSeekDistance;

// 总 IO 时间（秒）。
		double TotalIOTime;
// 总空闲时间（秒）。
		double TotalIdleTime;
	};
```

**本段新元素**

- `Reset()` —— 所有计数器清零
- `void Dump() const;` —— 只声明，实现在 `.cpp` 里
- `uint64 TotalReads` / `TotalSeeks` —— 读取/寻道次数
- `uint64 TotalBytesRead` / `TotalSeekDistance` —— 读取字节/寻道距离
- `double TotalIOTime` / `TotalIdleTime` —— IO 时间/空闲时间

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 72~80 行 | `Reset()` 清零 6 个字段 |
| 第 82 行 | `Dump()` 声明 |
| 第 84~91 行 | 6 个数据字段 |

> 类比：像账本：一页记流水（字段），一页写汇总（Dump），还有一块橡皮（Reset）。

### 七、`FDiskUtilizationTracker` 成员变量（第 94~105 行）

```cpp
// 长期统计（整个运行期）。
	UtilizationStats LongTermStats;
// 短期统计（最近一段时间）。
	UtilizationStats ShortTermStats;

// 保护统计数据的临界区（多线程安全）。
	FCriticalSection CriticalSection;

// 空闲开始时的 CPU 周期数。
	uint64 IdleStartCycle;
// 读取开始时的 CPU 周期数。
	uint64 ReadStartCycle;

// 正在飞行中的字节数（已发起但未完成的读取）。
	uint64 InFlightBytes;
// 正在飞行中的读取请求数。
	int32  InFlightReads;

// 是否要重置短期统计（线程安全布尔）。
	FThreadSafeBool bResetShortTermStats;
```

**本段新元素**

- `UtilizationStats LongTermStats` —— 长期统计（整个运行期）
- `UtilizationStats ShortTermStats` —— 短期统计（最近一段时间）
- `FCriticalSection CriticalSection` —— 保护统计数据的临界区
- `uint64 IdleStartCycle` / `ReadStartCycle` —— 空闲/读取开始的 CPU 周期
- `uint64 InFlightBytes` / `int32 InFlightReads` —— 飞行中的字节/请求数
- `FThreadSafeBool bResetShortTermStats` —— 线程安全的重置标志

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 94~95 行 | 长期/短期统计 |
| 第 97 行 | 临界区 |
| 第 99~100 行 | 两个周期起点 |
| 第 102~103 行 | 飞行中计数 |
| 第 105 行 | 重置标志 |

> 类比：像仪表盘上的指针和按钮：长期/短期两块表，一个锁（防止同时读），几个计数器。

### 八、构造函数 + 6 个公开方法（第 107~136 行）

```cpp
// 构造函数：4 个周期/计数成员清零。
	FDiskUtilizationTracker() :
// 空闲开始周期清零。
		IdleStartCycle(0),
// 读取开始周期清零。
		ReadStartCycle(0),
// 飞行中字节数清零。
		InFlightBytes(0),
// 飞行中读取数清零。
		InFlightReads(0)
// 构造函数体开始（空）。
	{
	}

// 开始一次读取：记录字节数和寻道距离。
	void StartRead(uint64 InReadBytes, uint64 InSeekDistance = 0);
// 结束一次读取。
	void FinishRead();

// 取当前未完成的请求数。
	uint32 GetOutstandingRequests() const
	{
// 直接返回飞行中读取数。
		return InFlightReads;
	}

// 取长期统计数据（只读）。
	const struct UtilizationStats& GetLongTermStats() const
	{
// 返回长期统计引用。
		return LongTermStats;
	}

// 取短期统计数据（只读）。
	const struct UtilizationStats& GetShortTermStats() const
	{
// 返回短期统计引用。
		return ShortTermStats;
	}

// 请求重置短期统计（下一帧生效）。
	void ResetShortTermStats()
	{
// 置位重置标志。
		bResetShortTermStats = true;
	}
```

**本段新元素**

- `StartRead(InReadBytes, InSeekDistance = 0)` —— 开始一次读取
- `FinishRead()` —— 结束一次读取
- `GetOutstandingRequests()` —— 取未完成请求数
- `GetLongTermStats()` / `GetShortTermStats()` —— 取统计（只读引用）
- `ResetShortTermStats()` —— 请求重置短期统计（置标志，下一帧生效）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 107~113 行 | 构造函数清零 4 个成员 |
| 第 115~116 行 | `StartRead` / `FinishRead` 声明 |
| 第 118~121 行 | `GetOutstandingRequests` |
| 第 123~131 行 | 两个 Get 统计 |
| 第 133~136 行 | `ResetShortTermStats` |

> 类比：像停车场的进出闸机：进（StartRead）、出（FinishRead）、查还有几辆车（GetOutstandingRequests）。

### 九、私有区（第 138~143 行）

```cpp
// 私有区。
private:
// 取限流速率（MB/秒），静态方法。
	static float GetThrottleRateMBS();
// 打印频率（秒），编译期常量 0.5 秒。
	static constexpr float PrintFrequencySeconds = 0.5f;

// 按需打印统计（内部用）。
	void MaybePrint();
};
```

**本段新元素**

- `static float GetThrottleRateMBS()` —— 取限流速率（MB/秒）
- `static constexpr float PrintFrequencySeconds = 0.5f` —— 打印频率（编译期常量）
- `void MaybePrint()` —— 按需打印

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 138 行 | 私有区开始 |
| 第 139 行 | 限流速率 |
| 第 140 行 | 打印频率常量 |
| 第 142 行 | 按需打印 |

> 类比：像仪表盘背面的调节螺丝：用户看不到，但影响显示。

### 十、全局实例 + 作用域追踪器（真实版）（第 145~158 行）

```cpp
// 全局磁盘利用率追踪器实例。
extern FDiskUtilizationTracker GDiskUtilizationTracker;

// FScopedDiskUtilizationTracker：作用域追踪器，构造时开始读取，析构时结束读取。
struct FScopedDiskUtilizationTracker
{
// 构造函数：记录字节数和寻道距离。
	FScopedDiskUtilizationTracker(uint64 InReadBytes, uint64 InSeekDistance) 
	{
// 通知全局追踪器「开始读取」。
		GDiskUtilizationTracker.StartRead(InReadBytes, InSeekDistance);
	}

// 析构函数：自动结束读取。
	~FScopedDiskUtilizationTracker()
	{
// 通知全局追踪器「结束读取」。
		GDiskUtilizationTracker.FinishRead();
	}
};
```

**本段新元素**

- `extern FDiskUtilizationTracker GDiskUtilizationTracker` —— 全局唯一实例
- `FScopedDiskUtilizationTracker` —— RAII 作用域追踪器
- 构造函数调 `StartRead()`，析构函数调 `FinishRead()`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 145 行 | 全局实例声明 |
| 第 147~152 行 | 构造函数：开始读取 |
| 第 154~157 行 | 析构函数：结束读取 |

> 类比：像自动门：人进来自动开（构造），人走出去自动关（析构）。

### 十一、`#else` 分支：空壳版（第 160~169 行）

```cpp
// 否则（没开启追踪）。
#else

// FScopedDiskUtilizationTracker：空壳版本，什么都不做。
struct FScopedDiskUtilizationTracker
{
// 构造函数：参数被忽略。
	FScopedDiskUtilizationTracker(uint64 Size, uint64 SeekDistance)
	{
	}
};

// 条件编译结束。
#endif // TRACK_DISK_UTILIZATION（是否追踪磁盘利用率）
```

**本段新元素**

- `#else` —— 没开启追踪时走这里
- 空壳 `FScopedDiskUtilizationTracker` —— 构造函数什么都不做
- 参数被忽略 —— 编译器可能给「未使用参数」警告，但零开销

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 160 行 | else 分支开始 |
| 第 162~167 行 | 空壳结构体 |
| 第 169 行 | 条件编译结束 |

> 类比：像没装记录仪的车：位置留着，但什么都不记。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 开关宏 + include | 1~9 | 开关宏 + include |
| 追踪开启时的额外 include + CSV 分类 | 11~20 | 追踪开启时的额外 include + CSV 分类 |
| `UtilizationStats` 构造函数 | 22~33 | `UtilizationStats` 构造函数 |
| 4 个吞吐率计算函数 | 35~53 | 4 个吞吐率计算函数 |
| 3 个时间/占比计算函数 | 55~70 | 3 个时间/占比计算函数 |
| `Reset()` + `Dump()` + 6 个字段 | 72~92 | `Reset()` + `Dump()` + 6 个字段 |
| `FDiskUtilizationTracker` 成员变量 | 94~105 | `FDiskUtilizationTracker` 成员变量 |
| 构造函数 + 6 个公开方法 | 107~136 | 构造函数 + 6 个公开方法 |
| 私有区 | 138~143 | 私有区 |
| 全局实例 + 作用域追踪器（真实版） | 145~158 | 全局实例 + 作用域追踪器（真实版） |
| `#else` 分支：空壳版 | 160~169 | `#else` 分支：空壳版 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#ifndef X` / `#define X 0` / `#endif` | 外部没定义就给默认值 | `#ifndef TRACK_DISK_UTILIZATION` |
| `#if X` / `#else` / `#endif` | 条件编译 | `#if TRACK_DISK_UTILIZATION` |
| `struct X { }` | 结构体（默认 public） | `struct FDiskUtilizationTracker` |
| `X() : a(0), b(0) {}` | 初始化列表 | `UtilizationStats() : TotalReads(0), ...` |
| `cond ? A : B` | 三元运算符 | `TotalIOTime > 0.0 ? ... : 0.0` |
| `static constexpr float X = 0.5f` | 编译期常量 | `static constexpr float PrintFrequencySeconds = 0.5f` |
| `extern T X;` | 声明外部定义的全局量 | `extern FDiskUtilizationTracker GDiskUtilizationTracker;` |
| `~X()` | 析构函数 | `~FScopedDiskUtilizationTracker()` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 169 行 | 259 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 129 行 | 129 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `#endif // 宏名` | 保留宏名 | 保留宏名 + 中文说明 | 宏名是标识符，不翻译 |
| 引擎拼写 `Througput` | 少一个 h | 照抄不改 | 引擎原文如此 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`（259 行）
2. 脚本验收：`codeOurs=129 / codeEngine=129 / diff=0`
3. 确认后下一个：`HAL\Event.h`（4534 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`（169 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`（259 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\04_DiskUtilizationTracker.h_总md.md`