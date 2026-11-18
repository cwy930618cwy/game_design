// 版权所有 Epic Games, Inc. 保留所有权利。

// 如果外部没定义 FMEMORY_INLINE_FUNCTION_DECORATOR（内联函数装饰器）。
#if !defined(FMEMORY_INLINE_FUNCTION_DECORATOR)
// 默认定义为空（即普通函数）。
	#define FMEMORY_INLINE_FUNCTION_DECORATOR 
// 条件编译结束。
#endif

// 如果外部没定义 FMEMORY_INLINE_GMalloc（全局分配器）。
#if !defined(FMEMORY_INLINE_GMalloc)
// 默认指向 UE::Private::GMalloc。
	#define FMEMORY_INLINE_GMalloc UE::Private::GMalloc
// 条件编译结束。
#endif

// 引入低层内存追踪器：LLM_IF_ENABLED 宏。
#include "HAL/LowLevelMemTracker.h"
// 引入 AutoRTFM：事务内存（Transactional Function Memory）支持。
#include "AutoRTFM.h"

// 前置声明 FMemory：本文件实现它的内联函数。
struct FMemory;
// 前置声明 FScopedMallocTimer：分配计时器（统计用）。
struct FScopedMallocTimer;

// FMemory::Malloc：分配内存（Count 字节，Alignment 对齐）。
FMEMORY_INLINE_FUNCTION_DECORATOR void* FMemory::Malloc(SIZE_T Count, uint32 Alignment)
// 函数体开始。
{
	// AutoRTFM：对非事务代码，这些调用全部优化掉，行为与以前完全一致。
	// 对事务代码，我们在 open 阶段就调用分配器（优化），这样就不用追踪
	// 分配器内部数据结构的写入。因为分配器本身已经是事务性的 ——
	// malloc 可以通过调用 free 来回滚。
	// （原文续行）
	// （原文续行）
// AutoRTFM::Open：在事务的「打开」阶段执行分配（优化：避免追踪分配器内部写入）。
	void* Ptr = AutoRTFM::Open([Count, Alignment]
// lambda 体开始。
	{
// 分配结果指针，先置空。
		void* Alloc = nullptr;
// 如果全局分配器为空（未初始化）。
		if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
		{
// 走外部分配（MallocExternal）。
			Alloc = MallocExternal(Count, Alignment);
// 条件体结束。
		}
// 否则（有全局分配器）。
		else
// 否则体开始。
		{
// 调用游戏线程钩子（统计/调试用，参数 0 表示 Malloc）。
			DoGamethreadHook(0);
// 创建分配计时器（参数 0 表示 Malloc）。
			FScopedMallocTimer Timer(0);
// 走全局分配器的 Malloc。
			Alloc = FMEMORY_INLINE_GMalloc->Malloc(Count, Alignment);
// 否则体结束。
		}
		// 可选的「每次分配都追踪」
// LLM_IF_ENABLED：如果启用了低层内存追踪，记录这次分配。
		LLM_IF_ENABLED(FLowLevelMemTracker::Get().OnLowLevelAlloc(ELLMTracker::Default, Alloc, Count, ELLMTag::Untagged, ELLMAllocType::FMalloc));
// 返回分配结果。
		return Alloc;
// lambda 体结束。
	});

	// AutoRTFM：对非事务代码这是空操作。
	// 对事务代码，如果事务中止，就延迟调用 Free，
	// 这样回滚这次分配时会把内存释放掉。
// AutoRTFM::OnAbort：如果事务回滚，就释放这块内存。
	AutoRTFM::OnAbort([Ptr]
// lambda 体开始。
	{
		// 关闭「Free 收到可能为空的指针」这条代码分析警告。
		// Free 本身已经显式处理了这种情况。
// 释放指针（Free 内部已处理空指针）。
		Free(Ptr); //-V575
// lambda 体结束。
	});

// AutoRTFM::DidAllocate：通知事务系统「分配了这块内存」，返回指针。
	return AutoRTFM::DidAllocate(Ptr, Count);
// 函数体结束。
}

// FMemory::Realloc：重新分配内存（Original 原指针，Count 新大小，Alignment 对齐）。
FMEMORY_INLINE_FUNCTION_DECORATOR void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
// 函数体开始。
{
// 如果处于事务的「关闭」状态（即事务代码里）。
	if (AutoRTFM::IsClosed())
// 条件体开始。
	{
		// AutoRTFM：对事务代码，处理 Realloc 要绕个弯。
		// 我们把 realloc 变成 Malloc + Memcpy + Free，绝不调用底层分配器的 realloc。
		// 这是必须的：如果调用真正的 realloc，它可能把旧内存释放掉。
		// 而如果之后事务中止，我们会把指针回滚到那块旧分配上，
		// 却再也拿不回原来那个地址的分配了。
		// 
		// 这里其实有机会：如果原指针就是本事务内分配的，上面说的就不适用，
		// 因为回滚事务会释放那块内存，结果堆也不会指向它。所以
		// 如果我们知道 Original 是本事务内分配的，就可以调用底层 realloc ——
		// 但还要处理 malloc 延迟的那次 free 调用，得把它也擦掉。
		// （原文续行）
		// （原文续行）
		// （原文续行）
		// （原文续行）

// 新指针，先置空。
		void* Ptr = nullptr;

		// 取决于底层实现，这里的 Malloc 即使 Count 为 0 也可能真的分配
		// （这是实现定义的行为）。所以既然 Count 为 0 时返回 null 也可以，
		// 我们就检查这种情况，直接跳过 Malloc 调用。
		// （原文续行）
// 如果新大小大于 0。
		if (Count > 0)
// 条件体开始。
		{
// 先分配新内存。
			Ptr = Malloc(Count, Alignment);

// 如果分配失败。
			if (!Ptr)
// 条件体开始。
			{
// 返回空指针。
				return nullptr;
// 条件体结束。
			}
// 条件体结束。
		}

// 如果原指针非空。
		if (Original)
// 条件体开始。
		{
// 如果新指针也非空。
			if (Ptr)
// 条件体开始。
			{
// 取原内存大小。
				SIZE_T OriginalCount = GetAllocSize(Original);
// 取两者较小值作为拷贝长度（处理新大小更小的情况）。
				SIZE_T CopyCount = FGenericPlatformMath::Min(Count, OriginalCount); // 处理新大小更小的情况

// 拷贝数据。
				Memcpy(Ptr, Original, CopyCount);
// 条件体结束。
			}
			
// 释放原内存。
			Free(Original);
// 条件体结束。
		}

// 返回新指针。
		return Ptr;
// 条件体结束。
	}

	// 可选的追踪 —— Original 为空的 realloc 等价于 malloc，没有要释放的
	// （原文续行）
// LLM_REALLOC_SCOPE：低层内存追踪的 realloc 作用域。
	LLM_REALLOC_SCOPE(Original);
// 如果原指针非空，先记录一次释放。
	LLM_IF_ENABLED(if (Original != nullptr) FLowLevelMemTracker::Get().OnLowLevelFree(ELLMTracker::Default, Original, ELLMAllocType::FMalloc));

// 新指针。
	void* Ptr;
// 如果全局分配器为空。
	if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
	{
// 走外部 realloc。
		Ptr = ReallocExternal(Original, Count, Alignment);
// 条件体结束。
	}
// 否则。
	else
// 否则体开始。
	{
// 调用游戏线程钩子（参数 1 表示 Realloc）。
		DoGamethreadHook(1);
// 创建分配计时器（参数 1 表示 Realloc）。
		FScopedMallocTimer Timer(1);
// 走全局分配器的 Realloc。
		Ptr = FMEMORY_INLINE_GMalloc->Realloc(Original, Count, Alignment);
// 否则体结束。
	}

// 如果新指针和原指针不同（说明内存搬了）。
	if (Ptr != Original)
// 条件体开始。
	{
		// 如果 realloc 返回的是新内存，我们假定旧内存已被释放。
		// （原文续行）
// 通知事务系统「原内存已释放」。
		AutoRTFM::DidFree(Original);
// 条件体结束。
	}

	// 可选的「每次分配都追踪」—— Count 为 0 的 realloc 等价于 free()，
	// 会返回空指针，不需要追踪。如果 realloc 因为其他原因返回空
	// （比如分配失败），也没有理由追踪它。
// 如果新指针非空，记录这次分配。
	LLM_IF_ENABLED(if (Ptr != nullptr) FLowLevelMemTracker::Get().OnLowLevelAlloc(ELLMTracker::Default, Ptr, Count, ELLMTag::Untagged, ELLMAllocType::FMalloc));

// 返回新指针。
	return Ptr;
// 函数体结束。
}

// FMemory::Free：释放内存。
FMEMORY_INLINE_FUNCTION_DECORATOR void FMemory::Free(void* Original)
// 函数体开始。
{
// 如果指针为空。
	if (!Original)
// 条件体开始。
	{
// 创建计时器（参数 3 表示 Free 空指针）。
		FScopedMallocTimer Timer(3);
// 直接返回。
		return;
// 条件体结束。
	}

	// AutoRTFM：对事务代码，为了支持事务中止后「回滚」这次 Free，
	// 我们把真正的释放延迟到提交时。
	// （原文续行）
// UE_AUTORTFM_ONCOMMIT：事务提交时才真正释放（回滚则保留）。
	UE_AUTORTFM_ONCOMMIT(=)
// 代码块开始。
	{
		// 可选的「每次分配都追踪」
// 记录这次释放。
		LLM_IF_ENABLED(FLowLevelMemTracker::Get().OnLowLevelFree(ELLMTracker::Default, Original, ELLMAllocType::FMalloc));

// 如果全局分配器为空。
		if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
		{
// 走外部释放。
			FreeExternal(Original);
// 直接返回。
			return;
// 条件体结束。
		}
// 调用游戏线程钩子（参数 2 表示 Free）。
		DoGamethreadHook(2);
// 创建计时器（参数 2 表示 Free）。
		FScopedMallocTimer Timer(2);
// 走全局分配器的 Free。
		FMEMORY_INLINE_GMalloc->Free(Original);

// 通知事务系统「已释放」。
		AutoRTFM::DidFree(Original);
// 代码块结束。
	};
// 函数体结束。
}

// FMemory::GetAllocSize：取一块内存的实际分配大小。
FMEMORY_INLINE_FUNCTION_DECORATOR SIZE_T FMemory::GetAllocSize(void* Original)
// 函数体开始。
{
// 结果变量。
	SIZE_T Result;
// UE_AUTORTFM_OPEN：在事务打开阶段执行。
	UE_AUTORTFM_OPEN
// 代码块开始。
	{
// 如果全局分配器为空。
		if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
		{
// 走外部实现。
			Result = GetAllocSizeExternal(Original);
// 条件体结束。
		}
// 否则。
		else
// 否则体开始。
		{
// 大小变量，先置 0。
			SIZE_T Size = 0;
// 调分配器取大小，返回是否成功。
			const bool bGotSize = FMEMORY_INLINE_GMalloc->GetAllocationSize(Original, Size);
// 成功就用取到的大小，否则 0。
			Result = bGotSize ? Size : 0;

			// 这段在编译期会被折叠掉，所以检查只在事务代码路径里执行。
			// 检查的目的是确保所用分配器能返回正确的分配大小，
			// 这是 AutoRTFM 能工作的基石要求。
// 如果处于事务代码里。
			if (AutoRTFM::IsClosed())
// 条件体开始。
			{
// 断言：AutoRTFM 要求分配器必须能返回正确大小。
				checkf(bGotSize, TEXT("For AutoRTFM to function it must be able to get the size of an allocation"));
// 条件体结束。
			}
// 否则体结束。
		}
// 代码块结束。
	};

// 返回结果。
	return Result;
// 函数体结束。
}

// FMemory::MallocZeroed：分配并清零内存。
FMEMORY_INLINE_FUNCTION_DECORATOR void* FMemory::MallocZeroed(SIZE_T Count, uint32 Alignment)
// 函数体开始。
{
// 结果指针，先置空（消除静态分析误报）。
	void* Ptr = nullptr; // Silence bogus static analysis warnings.

	// AutoRTFM：对非事务代码，这些调用全部优化掉，行为与以前完全一致。
	// 对事务代码，我们在 open 阶段就调用分配器（优化），这样就不用追踪
	// 分配器内部数据结构的写入。因为分配器本身已经是事务性的 ——
	// malloc 可以通过调用 free 来回滚。
	// （原文续行）
	// （原文续行）
// UE_AUTORTFM_OPEN：在事务打开阶段执行分配。
	UE_AUTORTFM_OPEN
// 代码块开始。
		{
// 如果全局分配器为空。
			if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
			{
// 走外部清零分配。
				Ptr = MallocZeroedExternal(Count, Alignment);
// 条件体结束。
			}
// 否则。
			else
// 否则体开始。
			{
// 调用游戏线程钩子（参数 0 表示 Malloc）。
				DoGamethreadHook(0);
// 创建计时器（参数 0 表示 Malloc）。
				FScopedMallocTimer Timer(0);
// 走全局分配器的 MallocZeroed。
				Ptr = FMEMORY_INLINE_GMalloc->MallocZeroed(Count, Alignment);
// 否则体结束。
			}
	// 可选的「每次分配都追踪」
// 记录这次分配。
	LLM_IF_ENABLED(FLowLevelMemTracker::Get().OnLowLevelAlloc(ELLMTracker::Default, Ptr, Count, ELLMTag::Untagged, ELLMAllocType::FMalloc));
// 代码块结束。
		};

	// AutoRTFM：对非事务代码这是空操作。
	// 对事务代码，如果事务中止，就延迟调用 Free，
	// 这样回滚这次分配时会把内存释放掉。
// AutoRTFM::OnAbort：事务回滚时释放。
	AutoRTFM::OnAbort([Ptr]
// lambda 体开始。
		{
			// 关闭「Free 收到可能为空的指针」这条代码分析警告。
			// Free 本身已经显式处理了这种情况。
// 释放指针。
			Free(Ptr); //-V575
// lambda 体结束。
		});

// 通知事务系统「分配了这块内存」，返回指针。
	return AutoRTFM::DidAllocate(Ptr, Count);
// 函数体结束。
}

// FMemory::QuantizeSize：把请求大小量化成分配器实际会给的大小。
FMEMORY_INLINE_FUNCTION_DECORATOR SIZE_T FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment)
// 函数体开始。
{
// 结果变量。
	SIZE_T Result;
// UE_AUTORTFM_OPEN：在事务打开阶段执行。
	UE_AUTORTFM_OPEN
// 代码块开始。
	{
// 如果全局分配器为空。
		if (!FMEMORY_INLINE_GMalloc)
// 条件体开始。
		{
// 没有分配器就直接返回请求大小。
			Result = Count;
// 条件体结束。
		}
// 否则。
		else
// 否则体开始。
		{
// 问分配器实际会给多少。
			Result = FMEMORY_INLINE_GMalloc->QuantizeSize(Count, Alignment);
// 否则体结束。
		}
// 代码块结束。
	};

// 返回结果。
	return Result;
// 函数体结束。
}

