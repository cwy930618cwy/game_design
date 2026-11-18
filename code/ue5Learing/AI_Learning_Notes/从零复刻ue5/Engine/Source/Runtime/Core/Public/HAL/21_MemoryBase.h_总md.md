# 21 — `HAL/MemoryBase.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\MemoryBase.h`（**294 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\MemoryBase.h`
> **⚠️ 本节不能独立编译**——依赖 `FExec` / `FOutputDevice` / `TAtomic`。

---

## 一、这个文件解决什么问题（为什么）

`FMemory::Malloc(1024)` 最终会调到 `GMalloc->Malloc(1024)`。**`GMalloc` 是什么？** 它是一个 `FMalloc*` 指针，指向当前使用的分配器（`FMallocBinned2` / `FMallocTBB` / `FMallocAnsi` …）。

`MemoryBase.h` 就是**分配器的抽象基类**：定义 `FMalloc` 接口，所有具体分配器都继承它。

**它包含 5 个东西**：

| 类型 | 作用 |
|---|---|
| `DEFAULT_ALIGNMENT` / `MIN_ALIGNMENT` | 2 个对齐常量（0 / 8） |
| `UE::Private::GMalloc` | 全局分配器指针（命名空间版） |
| `GMalloc` | 全局分配器指针（兼容版，**已废弃 5.6**） |
| `GFixedMallocLocationPtr` | 固定分配器指针的地址 |
| `FUseSystemMallocForNew` | 让派生类的 `new`/`delete` 走系统分配器 |
| `FMalloc` | **分配器抽象基类**（核心） |

**核心设计 ①：`FMalloc` 的 3 个纯虚函数**（第 104 / 115 / 127 行）

```cpp
virtual void* Malloc(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT) = 0;
virtual void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT) = 0;
virtual void Free(void* Original) = 0;
```

**这 3 个是必须实现的**，其他 20+ 个方法都有默认实现。

**核心设计 ②：`Try*` 系列（不抛异常）**（第 110 / 122 / 138 行）

```cpp
virtual void* TryMalloc(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
virtual void* TryRealloc(void* Original, SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
virtual void* TryMallocZeroed(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
```

**区别**：`Malloc` 失败会**崩溃**（OOM），`TryMalloc` 失败返回 `nullptr`。用于「可以优雅降级」的场景。

**核心设计 ③：`FUseSystemMallocForNew`**（第 61~93 行）

```cpp
class FUseSystemMallocForNew
{
public:
    CORE_API void* operator new(size_t Size);
    CORE_API void operator delete(void* Ptr);
    void* operator new[](size_t Size);
    void operator delete[](void* Ptr);
};
```

**为什么需要**：`FMalloc` 自己继承它 —— 因为**分配器不能用自己的 `Malloc` 来分配自己的内存**（会死循环）。所以 `FMalloc` 的对象走系统 `malloc`。

**核心设计 ④：`GMalloc` 的双版本**（第 36~47 行）

```cpp
namespace UE::Private
{
    CORE_API extern class FMalloc* GMalloc;          // 新版本
}

UE_DEPRECATED(5.6, "Consider using FMemory::Malloc instead of accessing GMalloc directly.")
CORE_API extern class FMalloc* const& GMalloc;       // 兼容版本
```

**为什么两个**：新代码用 `UE::Private::GMalloc`，老代码用全局 `GMalloc`（`const&` 绑定到前者）。5.6 起全局版标记废弃。

**核心设计 ⑤：TLS 缓存管理**（第 173~196 行）

```cpp
virtual void SetupTLSCachesOnCurrentThread() {}
virtual void MarkTLSCachesAsUsedOnCurrentThread() {}
virtual void MarkTLSCachesAsUnusedOnCurrentThread() {}
virtual void ClearAndDisableTLSCachesOnCurrentThread() {}
```

**为什么需要**：现代分配器（Binned2）每个线程有独立缓存。线程睡眠时缓存可以 `Trim` 掉，醒来时再恢复。

**说个场景**：

```cpp
// 引擎启动时选择分配器
if (FParse::Param(CmdLine, TEXT("binned2")))
    GMalloc = new FMallocBinned2();
else
    GMalloc = new FMallocAnsi();

// 之后所有分配都走它
void* Ptr = FMemory::Malloc(1024);   // → GMalloc->Malloc(1024)
```

> 🔑 **一句话**：`MemoryBase.h` = **分配器的「插座标准」**，定义 `FMalloc` 接口，所有分配器插上去就能用。

---

## 二、内部结构全景

```
MemoryBase.h（294 行）
├─ 第 1~13 行      版权头 + #pragma once + 5 个 include + 2 个前置声明
├─ 第 15~17 行     ① UPDATE_MALLOC_STATS（默认 1）
├─ 第 19~28 行     ② 匿名 enum：DEFAULT_ALIGNMENT / MIN_ALIGNMENT
├─ 第 31~39 行     ③ namespace UE::Private { GMalloc }
├─ 第 41~47 行     ④ GMalloc（兼容版，已废弃 5.6）
├─ 第 49~50 行     ⑤ GFixedMallocLocationPtr
├─ 第 52~53 行     ⑥ FGenericMemoryStats 前置声明
├─ 第 56~93 行     ⑦ class FUseSystemMallocForNew
│    ├─ 第 70 行      operator new
│    ├─ 第 77 行      operator delete
│    ├─ 第 85 行      operator new[]
│    └─ 第 92 行      operator delete[]
├─ 第 95~294 行    ⑧ class FMalloc : public FUseSystemMallocForNew, public FExec
│    ├─ 第 104 行     Malloc（纯虚）
│    ├─ 第 110 行     TryMalloc
│    ├─ 第 115 行     Realloc（纯虚）
│    ├─ 第 122 行     TryRealloc
│    ├─ 第 127 行     Free（纯虚）
│    ├─ 第 132 行     MallocZeroed
│    ├─ 第 138 行     TryMallocZeroed
│    ├─ 第 146 行     QuantizeSize
│    ├─ 第 158 行     GetAllocationSize
│    ├─ 第 166 行     Trim
│    ├─ 第 173~196 行 4 个 TLS 缓存方法
│    ├─ 第 202 行     InitializeStatsMetadata
│    ├─ 第 208 行     Exec（override）
│    ├─ 第 215 行     UpdateStats
│    ├─ 第 218 行     GetAllocatorStats
│    ├─ 第 221 行     DumpAllocatorStats
│    ├─ 第 230 行     IsInternallyThreadSafe
│    ├─ 第 238 行     ValidateHeap
│    ├─ 第 248 行     GetDescriptiveName
│    ├─ 第 256 行     OnMallocInitialized
│    ├─ 第 261 行     OnPreFork
│    ├─ 第 266 行     OnPostFork
│    ├─ 第 271 行     GetImmediatelyFreeableCachedMemorySize
│    ├─ 第 281 行     GetTotalFreeCachedMemorySize
│    ├─ 第 286~287 行 protected + friend
│    └─ 第 289~293 行 MaxSingleAlloc（非 Shipping）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/MemoryBase.h` | **是**（`FExec` / `FOutputDevice` / `TAtomic`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `SIZE_T` / `uint32` / `uint64` / `TCHAR` |
| `HAL/PlatformAtomics.h` | 平台原子操作 |
| `HAL/PlatformCrt.h` | `size_t` |
| `Misc/Exec.h` | **`FExec`（基类）** |
| `Misc/OutputDevice.h` | `FOutputDevice` |
| `Templates/Atomic.h` | `TAtomic` |

**注意**：`UWorld` / `TAtomic` / `FGenericMemoryStats` 都是**前置声明**（第 12~13 / 53 行），本文件只用到指针/引用（照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\34_MemoryBase.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `MemoryMisc.h`（约 3KB）。