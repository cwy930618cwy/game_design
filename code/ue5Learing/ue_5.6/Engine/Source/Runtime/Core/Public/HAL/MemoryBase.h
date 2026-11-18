// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：SIZE_T / uint32 / uint64 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入平台原子操作。
#include "HAL/PlatformAtomics.h"
// 引入平台 C 运行时：size_t。
#include "HAL/PlatformCrt.h"
// 引入 FExec：可执行控制台命令的接口（本文件 FMalloc 的基类之一）。
#include "Misc/Exec.h"
// 引入 FOutputDevice：日志输出设备。
#include "Misc/OutputDevice.h"
// 引入 TAtomic：原子变量模板。
#include "Templates/Atomic.h"

// 前置声明 UWorld。
class UWorld;
// 前置声明 TAtomic 模板。
template <typename T> class TAtomic;

// 如果没定义 UPDATE_MALLOC_STATS（是否更新分配器统计）。
#ifndef UPDATE_MALLOC_STATS
// 默认 1（启用）。
	#define UPDATE_MALLOC_STATS 1
// 条件编译结束。
#endif

// 匿名枚举：对齐常量。
enum
// 枚举体开始。
{
	// 默认分配器对齐。指定默认值时分配器按引擎规则处理。
	// >= 16 字节的块按 16 字节对齐，< 16 字节的块按 8 字节对齐。如果分配器
	// 不支持对齐，则忽略对齐参数。
// DEFAULT_ALIGNMENT：默认对齐（0 表示按引擎规则）。
	DEFAULT_ALIGNMENT = 0,

	// 最小分配器对齐
// MIN_ALIGNMENT：最小对齐（8 字节）。
	MIN_ALIGNMENT = 8,
// 枚举体结束。
};

	
// 全局内存分配器。 大多数调用方应该用 FMemory::Malloc 而不是直接访问 GMalloc。 FMemory 包装了 GMalloc，还提供低层内存追踪和 AutoRTFM 支持。
// 命名空间 UE::Private：引擎私有实现。
namespace UE::Private
// 命名空间开始。
{
// GMalloc：全局分配器指针（命名空间版）。
	CORE_API extern class FMalloc* GMalloc;
// 命名空间结束。
}

// 不带命名空间的 GMalloc 为向后兼容而保留。 大多数调用方应该用 FMemory::Malloc 而不是直接访问 GMalloc。 FMemory 包装了 GMalloc，还提供低层内存追踪和 AutoRTFM 支持。
UE_DEPRECATED(5.6, "Consider using FMemory::Malloc instead of accessing GMalloc directly.")
// GMalloc：全局分配器指针（兼容版，const 引用）。
CORE_API extern class FMalloc* const& GMalloc;

/** PLATFORM_USES_FIXED_GMalloc_CLASS 为 true 时，分配器指针的存放位置。 */
// GFixedMallocLocationPtr：固定分配器指针的地址。
CORE_API extern class FMalloc** GFixedMallocLocationPtr;

/** 存放通用内存统计，内部用 map 实现。 */
// 前置声明 FGenericMemoryStats。
struct FGenericMemoryStats;


// 继承 FUseSystemMallocForNew 可让你的对象用系统 malloc 分配，绕过 GMalloc。 例如 FMalloc 自己就用了这个。
// FUseSystemMallocForNew：让派生类的 new/delete 走系统分配器。
class FUseSystemMallocForNew
// 类体开始。
{
// 公有区。
public:
// 重载 new 运算符，使用系统分配器。 @param	Size	要分配的内存大小（字节） @return		指向大小为 Size 的内存块的指针，或 nullptr
// operator new：用系统分配器分配。
	CORE_API void* operator new(size_t Size);

// 重载 delete 运算符，使用系统分配器 @param	Ptr		要删除的指针
// operator delete：用系统分配器释放。
	CORE_API void operator delete(void* Ptr);

// 重载数组 new 运算符，使用系统分配器。 @param	Size	要分配的内存大小（字节） @return		指向大小为 Size 的内存块的指针，或 nullptr
// operator new[]：用系统分配器分配数组。
	void* operator new[](size_t Size);

// 重载数组 delete 运算符，使用系统分配器 @param	Ptr		要删除的指针
// operator delete[]：用系统分配器释放数组。
	void operator delete[](void* Ptr);
// 类结束。
};

/** 全局内存分配器的接口。 */
// FMalloc：所有分配器的基类。
class FMalloc  : 
// 继承 FUseSystemMallocForNew：FMalloc 自己的内存走系统分配器。
	public FUseSystemMallocForNew,
// 继承 FExec：支持控制台命令。
	public FExec
// 类体开始。
{
// 公有区。
public:
// 分配内存
// Malloc：纯虚，分配 Count 字节（按 Alignment 对齐）。
	virtual void* Malloc( SIZE_T Count, uint32 Alignment=DEFAULT_ALIGNMENT ) = 0;

// TryMalloc —— 类似 Malloc()，但分配请求无法满足时可能返回 nullptr。
// TryMalloc：默认实现转调 Malloc。
	CORE_API virtual void* TryMalloc( SIZE_T Count, uint32 Alignment=DEFAULT_ALIGNMENT );

// 重新分配内存
// Realloc：纯虚，把 Original 重新分配到 Count 字节。
	virtual void* Realloc( void* Original, SIZE_T Count, uint32 Alignment=DEFAULT_ALIGNMENT ) = 0;

// TryRealloc —— 类似 Realloc()，但分配请求无法满足时可能返回 nullptr。 注意这种情况下 Original 指向的内存仍然有效。
// TryRealloc：默认实现转调 Realloc。
	CORE_API virtual void* TryRealloc(void* Original, SIZE_T Count, uint32 Alignment=DEFAULT_ALIGNMENT);

// 释放内存
// Free：纯虚，释放 Original。
	virtual void Free( void* Original ) = 0;

// 分配并清零内存
// MallocZeroed：默认实现转调 Malloc + 清零。
	CORE_API virtual void* MallocZeroed(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);

// TryMallocZeroed —— 类似 MallocZeroed()，但分配请求无法满足时可能返回 nullptr。
// TryMallocZeroed：默认实现转调 TryMalloc + 清零。
	CORE_API virtual void* TryMallocZeroed(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
		
// 对某些分配器，返回为消除内部碎片应该请求的实际大小。返回值总是 >= Count。 可用于把容器扩缩到最优大小。 这个调用总是很快且线程安全，无需加锁。
// QuantizeSize：默认实现直接返回 Count。
	virtual SIZE_T QuantizeSize(SIZE_T Count, uint32 Alignment)
// 函数体开始。
	{
		return Count; // 默认实现无法确定这个值
// 函数体结束。
	}

// 如果可能，确定给定地址上分配的内存大小 @param Original - 要检查大小的内存指针 @param SizeOut - 如果可能，设为传入指针的大小 @return 成功返回 true
// GetAllocationSize：默认实现返回 false。
	virtual bool GetAllocationSize(void *Original, SIZE_T &SizeOut)
// 函数体开始。
	{
		return false; // 默认实现无法确定这个值
// 函数体结束。
	}

// 尽可能释放内存。必须从主线程调用。
// Trim：默认实现什么都不做。
	virtual void Trim(bool bTrimThreadCaches)
// 函数体开始。
	{
// 空实现。
	}
// 函数体结束。

// 在当前线程建立 TLS 缓存。这些线程是我们能 trim 的。
// SetupTLSCachesOnCurrentThread：默认空实现。
	virtual void SetupTLSCachesOnCurrentThread()
// 函数体开始。
	{
// 空实现。
	}
// 函数体结束。

// 把当前线程的 TLS 缓存标记为已使用。线程醒来要处理事情，需要它的 TLS 缓存。
// MarkTLSCachesAsUsedOnCurrentThread：默认空实现。
	virtual void MarkTLSCachesAsUsedOnCurrentThread()
// 函数体开始。
	{
// 空实现。
	}
// 函数体结束。

// 把当前线程的 TLS 缓存标记为未使用。通常在进入睡眠前。这些线程可以在不唤醒它们的情况下被 trim。
// MarkTLSCachesAsUnusedOnCurrentThread：默认空实现。
	virtual void MarkTLSCachesAsUnusedOnCurrentThread()
// 函数体开始。
	{
// 空实现。
	}
// 函数体结束。

// 清除当前线程的 TLS 缓存并禁用后续缓存。
// ClearAndDisableTLSCachesOnCurrentThread：默认空实现。
	virtual void ClearAndDisableTLSCachesOnCurrentThread()
// 函数体开始。
	{
// 空实现。
	}
// 函数体结束。

// 初始化统计元数据。需要尽早做，但不能在构造函数里做， 因为依赖 FName::StaticInit
// InitializeStatsMetadata：默认空实现。
	CORE_API virtual void InitializeStatsMetadata();

// 如果允许 Exec 命令。
#if UE_ALLOW_EXEC_COMMANDS
// 处理命令行传入的任何命令
// Exec：默认返回 false（不处理）。
	virtual bool Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar ) override
// 函数体开始。
	{ 
// 默认不处理。
		return false; 
// 函数体结束。
	}
// 条件编译结束。
#endif // UE_ALLOW_EXEC_COMMANDS

	/** 每帧调用一次，收集并设置所有分配器统计到对应 stat。必须线程安全。 */
// UpdateStats：默认空实现。
	CORE_API virtual void UpdateStats();

	/** 把上次更新的分配器统计写入指定目标。 */
// GetAllocatorStats：默认空实现。
	CORE_API virtual void GetAllocatorStats( FGenericMemoryStats& out_Stats );

	/** 把当前分配器统计转储到日志。 */
// DumpAllocatorStats：默认打印「未实现」。
	virtual void DumpAllocatorStats( class FOutputDevice& Ar )
// 函数体开始。
	{
		Ar.Logf( TEXT("Allocator Stats for %s: (not implemented)" ), GetDescriptiveName() );
// 函数体结束。
	}

// 返回分配器是否保证线程安全，因此不需要在外面套多余的线程安全包装。
// IsInternallyThreadSafe：默认返回 false。
	virtual bool IsInternallyThreadSafe() const 
// 函数体开始。
	{ 
// 默认不保证。
		return false; 
// 函数体结束。
	}

// 校验分配器的堆
// ValidateHeap：默认返回 true。
	virtual bool ValidateHeap()
// 函数体开始。
	{
// 默认认为有效。
		return( true );
// 函数体结束。
	}

// 取用于日志的描述性名字。 @return 人类可读的 malloc 名字指针
// GetDescriptiveName：默认返回「未指定分配器」。
	virtual const TCHAR* GetDescriptiveName()
// 函数体开始。
	{
		return TEXT("Unspecified allocator");
// 函数体结束。
	}

// 通知 malloc 实现 GMalloc 里所有分配器初始化完成，可以安全初始化需要「常规」分配的任何额外功能了
// OnMallocInitialized：默认空实现。
	virtual void OnMallocInitialized() {}

// 通知 malloc 实现进程即将 fork。可用于 trim 缓存等。
// OnPreFork：默认空实现。
	virtual void OnPreFork() {}

// 通知 malloc 实现进程已 fork，以便尽量避免弄脏 fork 前的页。
// OnPostFork：默认空实现。
	virtual void OnPostFork() {}

// 返回分配器缓存的、在内存紧张时可归还系统的空闲内存量
// GetImmediatelyFreeableCachedMemorySize：默认返回 0。
	virtual uint64 GetImmediatelyFreeableCachedMemorySize() const
// 函数体开始。
	{
// 默认 0。
		return 0;
// 函数体结束。
	}

// 返回分配器缓存的总空闲内存量。 包括内存紧张时可归还系统的（见 GetImmediatelyFreeableCachedMemorySize()）， 以及无法归还内核但能满足部分分配需求的内存。
// GetTotalFreeCachedMemorySize：默认返回 0。
	virtual uint64 GetTotalFreeCachedMemorySize() const
// 函数体开始。
	{
// 默认 0。
		return 0;
// 函数体结束。
	}

// 受保护区。
protected:
// 友元：FCurrentFrameCalls 可访问受保护成员。
	friend struct FCurrentFrameCalls;

// 如果不是 Shipping 构建。
#if !UE_BUILD_SHIPPING
// 公有区。
public:
	/** 调试用，限制单次分配的最大字节数 */
// MaxSingleAlloc：单次分配上限（原子变量）。
	static CORE_API TAtomic<uint64> MaxSingleAlloc;
// 条件编译结束。
#endif
// 类结束。
};
