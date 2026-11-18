# 23 — `HAL/FMemory.inl` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`（**252 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`
> **⚠️ 本节不能独立编译**——`.inl` 是内联实现文件，必须被 `HAL/UnrealMemory.h` include。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

`FMemory` 是 UE 的**内存分配门面**：`FMemory::Malloc` / `Realloc` / `Free` 是所有 `new` / `delete` 的最终落点。但它的**声明**在 `UnrealMemory.h`，**实现**在这个 `.inl` 文件里。

**为什么用 `.inl` 而不是 `.cpp`？** 因为要**内联**。`Malloc` 被调用几百万次，函数调用开销不能忍。`.inl` 文件被 `UnrealMemory.h` 在末尾 include，编译器就能在每个编译单元里内联展开。

**它实现了 6 个函数**：

| 函数 | 作用 |
|---|---|
| `Malloc(Count, Alignment)` | 分配内存 |
| `Realloc(Original, Count, Alignment)` | 重新分配 |
| `Free(Original)` | 释放内存 |
| `GetAllocSize(Original)` | 取实际分配大小 |
| `MallocZeroed(Count, Alignment)` | 分配并清零 |
| `QuantizeSize(Count, Alignment)` | 把请求大小量化成分配器实际给的大小 |

**两个关键设计**：

**① 双路径**（每个函数都有）：

```cpp
if (!FMEMORY_INLINE_GMalloc)   // 分配器还没初始化
    Alloc = MallocExternal(...);   // 走外部实现（通常是系统 malloc）
else
    Alloc = FMEMORY_INLINE_GMalloc->Malloc(...);  // 走 UE 的分配器
```

**② AutoRTFM 事务内存支持**（这是本文件最复杂的地方）：

| 宏 | 作用 |
|---|---|
| `AutoRTFM::Open(lambda)` | 在事务「打开」阶段执行（分配器本身是事务性的，提前调用可避免追踪内部写入） |
| `AutoRTFM::OnAbort(lambda)` | 事务回滚时执行（释放刚分配的内存） |
| `AutoRTFM::DidAllocate(Ptr, Count)` | 通知事务系统「分配了这块内存」 |
| `AutoRTFM::DidFree(Ptr)` | 通知事务系统「释放了这块内存」 |
| `UE_AUTORTFM_ONCOMMIT` | 事务**提交**时才执行（Free 延迟到提交） |
| `UE_AUTORTFM_OPEN` | 事务打开阶段执行 |

> 🔑 **一句话**：`FMemory.inl` = **内存分配的「内联实现」**，双路径（外部/UE 分配器）+ AutoRTFM 事务支持。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 2 个宏默认值 + include（第 1~15 行）

```cpp
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
```

**本段新元素**

- `FMEMORY_INLINE_FUNCTION_DECORATOR` —— 函数装饰器，默认空（可加 `FORCEINLINE`）
- `FMEMORY_INLINE_GMalloc` —— 全局分配器，默认 `UE::Private::GMalloc`
- `HAL/LowLevelMemTracker.h` —— `LLM_IF_ENABLED` / `LLM_REALLOC_SCOPE`
- `AutoRTFM.h` —— 事务内存支持
- `struct FMemory;` / `struct FScopedMallocTimer;` —— 前置声明

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3~5 行 | 函数装饰器宏 |
| 第 7~9 行 | 全局分配器宏 |
| 第 11~12 行 | 2 个 include |
| 第 14~15 行 | 2 个前置声明 |

> 类比：像开工前定两个可调参数：函数怎么装饰、用哪个分配器。

### 二、`FMemory::Malloc`（第 17~54 行）

```cpp
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
```

**本段新元素**

- `AutoRTFM::Open([Count, Alignment] { ... })` —— 在事务打开阶段执行分配
- `FMEMORY_INLINE_GMalloc` —— 全局分配器指针（判空）
- `MallocExternal(Count, Alignment)` —— 外部分配（分配器未初始化时）
- `DoGamethreadHook(0)` —— 游戏线程钩子（参数 0 = Malloc）
- `FScopedMallocTimer Timer(0)` —— 分配计时器（参数 0 = Malloc）
- `LLM_IF_ENABLED(...)` —— 低层内存追踪（编译期可关）
- `FLowLevelMemTracker::Get().OnLowLevelAlloc(...)` —— 记录分配
- `AutoRTFM::OnAbort([Ptr] { Free(Ptr); })` —— 事务回滚时释放
- `AutoRTFM::DidAllocate(Ptr, Count)` —— 通知事务系统并返回指针

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 17~18 行 | 函数签名 + 开始 |
| 第 19~24 行 | AutoRTFM 说明注释 |
| 第 25~41 行 | `AutoRTFM::Open` lambda（双路径分配） |
| 第 43~51 行 | `AutoRTFM::OnAbort` lambda（回滚释放） |
| 第 53 行 | `DidAllocate` 返回 |
| 第 54 行 | 函数结束 |

> 类比：像借钱：先记一笔（Open），如果对方反悔就还回去（OnAbort），最后登记入账（DidAllocate）。

### 三、`FMemory::Realloc` 事务分支（第 56~105 行）

```cpp
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
```

**本段新元素**

- `AutoRTFM::IsClosed()` —— 是否处于事务代码里
- **事务里 realloc 变成 Malloc + Memcpy + Free** —— 不能调底层 realloc（会释放旧内存，回滚后拿不回来）
- `Count > 0` 才 Malloc —— 避免 Count=0 时实现定义行为
- `GetAllocSize(Original)` —— 取原内存大小
- `FGenericPlatformMath::Min(Count, OriginalCount)` —— 取较小值（新大小可能更小）
- `Memcpy(Ptr, Original, CopyCount)` —— 拷贝数据
- `Free(Original)` —— 释放原内存

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 56~58 行 | 函数签名 + 事务判断 |
| 第 60~73 行 | 为什么不能调底层 realloc（长注释） |
| 第 75~89 行 | Count > 0 才 Malloc |
| 第 91~102 行 | 拷贝 + 释放原内存 |
| 第 104~105 行 | 返回新指针 |

> 类比：像换房子：不能直接改造旧房（realloc 会拆掉），只能买新房（Malloc）+ 搬东西（Memcpy）+ 卖旧房（Free）。

### 四、`FMemory::Realloc` 非事务分支（第 107~137 行）

```cpp
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
```

**本段新元素**

- `LLM_REALLOC_SCOPE(Original)` —— LLM 的 realloc 作用域
- `OnLowLevelFree(...)` —— 先记录释放旧内存
- `ReallocExternal(...)` —— 外部 realloc
- `FMEMORY_INLINE_GMalloc->Realloc(...)` —— UE 分配器的 realloc
- `DoGamethreadHook(1)` / `FScopedMallocTimer Timer(1)` —— 参数 1 = Realloc
- `AutoRTFM::DidFree(Original)` —— 指针变了说明旧内存已释放
- `OnLowLevelAlloc(...)` —— 记录新分配

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 107~110 行 | LLM 追踪 |
| 第 112~122 行 | 双路径 realloc |
| 第 124~129 行 | 指针变了 → 通知事务系统 |
| 第 131~134 行 | 记录新分配 |
| 第 136~137 行 | 返回 + 结束 |

> 类比：像正常换房：直接找中介（realloc），一次搞定。

### 五、`FMemory::Free`（第 139~166 行）

```cpp
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
```

**本段新元素**

- `if (!Original)` —— 空指针直接返回（但先记一次计时器）
- `FScopedMallocTimer Timer(3)` —— 参数 3 = Free 空指针
- `UE_AUTORTFM_ONCOMMIT` —— **事务提交时才真正释放**（回滚则保留）
- `OnLowLevelFree(...)` —— 记录释放
- `FreeExternal(Original)` —— 外部释放
- `DoGamethreadHook(2)` / `FScopedMallocTimer Timer(2)` —— 参数 2 = Free
- `AutoRTFM::DidFree(Original)` —— 通知事务系统

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 139~145 行 | 空指针快速返回 |
| 第 147~150 行 | AutoRTFM 说明 + ONCOMMIT |
| 第 152~159 行 | LLM 追踪 + 外部释放 |
| 第 160~165 行 | UE 分配器释放 + 通知 |

> 类比：像退租：不是立刻搬走，而是等合同正式生效（提交）才搬。

### 六、`FMemory::GetAllocSize`（第 168~194 行）

```cpp
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
```

**本段新元素**

- `UE_AUTORTFM_OPEN { ... }` —— 事务打开阶段执行
- `GetAllocSizeExternal(Original)` —— 外部实现
- `FMEMORY_INLINE_GMalloc->GetAllocationSize(Original, Size)` —— 返回是否成功
- `bGotSize ? Size : 0` —— 成功用大小，失败用 0
- `checkf(bGotSize, TEXT("..."))` —— 断言（只在事务代码里执行）
- **为什么断言** —— AutoRTFM 要求分配器必须能返回正确大小

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 168~172 行 | 函数签名 + OPEN 块 |
| 第 173~176 行 | 外部实现 |
| 第 177~182 行 | UE 分配器实现 |
| 第 183~189 行 | 事务里的断言 |
| 第 193~194 行 | 返回 + 结束 |

> 类比：像问房东「这房子实际多大」：必须能答出来，否则事务系统没法工作。

### 七、`FMemory::MallocZeroed`（第 196~233 行）

```cpp
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
```

**本段新元素**

- `void* Ptr = nullptr;` —— 先置空（消除静态分析误报）
- `UE_AUTORTFM_OPEN { ... }` —— 事务打开阶段
- `MallocZeroedExternal(...)` —— 外部清零分配
- `FMEMORY_INLINE_GMalloc->MallocZeroed(...)` —— UE 分配器清零分配
- `AutoRTFM::OnAbort([Ptr] { Free(Ptr); })` —— 回滚释放
- `AutoRTFM::DidAllocate(Ptr, Count)` —— 通知并返回

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 196~198 行 | 函数签名 + 置空 |
| 第 200~220 行 | OPEN 块（双路径清零分配） |
| 第 222~230 行 | OnAbort 块 |
| 第 232~233 行 | DidAllocate + 结束 |

> 类比：像租新房还带保洁：Malloc 是租空房，MallocZeroed 是租完还帮你打扫干净。

### 八、`FMemory::QuantizeSize`（第 235~252 行）

```cpp
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

```

**本段新元素**

- `QuantizeSize(Count, Alignment)` —— 把请求大小量化成分配器实际给的大小
- `UE_AUTORTFM_OPEN { ... }` —— 事务打开阶段
- `Result = Count` —— 没有分配器就直接返回请求大小
- `FMEMORY_INLINE_GMalloc->QuantizeSize(...)` —— 问分配器实际给多少

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 235~239 行 | 函数签名 + OPEN 块 |
| 第 240~243 行 | 无分配器：直接返回 |
| 第 244~247 行 | 有分配器：问它 |
| 第 250~251 行 | 返回 + 结束 |

> 类比：像问「我要 100 块，你实际给我多少」：分配器按块大小给，可能给 128。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 2 个宏默认值 + include | 1~15 | 版权头 + 2 个宏默认值 + include |
| `FMemory::Malloc` | 17~54 | `FMemory::Malloc` |
| `FMemory::Realloc` 事务分支 | 56~105 | `FMemory::Realloc` 事务分支 |
| `FMemory::Realloc` 非事务分支 | 107~137 | `FMemory::Realloc` 非事务分支 |
| `FMemory::Free` | 139~166 | `FMemory::Free` |
| `FMemory::GetAllocSize` | 168~194 | `FMemory::GetAllocSize` |
| `FMemory::MallocZeroed` | 196~233 | `FMemory::MallocZeroed` |
| `FMemory::QuantizeSize` | 235~252 | `FMemory::QuantizeSize` |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `#if !defined(X)` / `#define X 值` / `#endif` | 外部没定义就给默认值 | `#if !defined(FMEMORY_INLINE_GMalloc)` |
| `[捕获列表] { ... }` | lambda 表达式 | `AutoRTFM::Open([Count, Alignment] { ... })` |
| `X ? A : B` | 三元运算符 | `bGotSize ? Size : 0` |
| `checkf(cond, msg)` | 带消息的断言 | `checkf(bGotSize, TEXT("..."))` |
| `UE_AUTORTFM_ONCOMMIT { ... };` | 事务提交时执行 | `UE_AUTORTFM_ONCOMMIT(=) { ... };` |
| `UE_AUTORTFM_OPEN { ... };` | 事务打开时执行 | `UE_AUTORTFM_OPEN { ... };` |
| `//-V575` | PVS-Studio 静态分析抑制注释 | `Free(Ptr); //-V575` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 252 行 | 414 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 161 行 | 161 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `//-V575` | 保留原文 | 保留原文 | 静态分析工具指令，不翻译 |
| `TEXT("...")` 字符串 | 英文原文 | 保留英文 | 运行时字符串，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`（414 行）
2. 脚本验收：`codeOurs=161 / codeEngine=161 / diff=0`
3. 确认后下一个：`HAL\HideTCHAR.h`（1331 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`（252 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`（414 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\10_FMemory.inl_总md.md`