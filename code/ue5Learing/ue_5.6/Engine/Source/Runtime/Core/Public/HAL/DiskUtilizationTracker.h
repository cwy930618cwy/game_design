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

// 私有区。
private:
// 取限流速率（MB/秒），静态方法。
	static float GetThrottleRateMBS();
// 打印频率（秒），编译期常量 0.5 秒。
	static constexpr float PrintFrequencySeconds = 0.5f;

// 按需打印统计（内部用）。
	void MaybePrint();
};

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