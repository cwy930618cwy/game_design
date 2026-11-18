# 10 — `HAL/FMemory.inl` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`（**252 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\FMemory.inl`
> **⚠️ 本节不能独立编译**——`.inl` 是内联实现文件，必须被 `HAL/UnrealMemory.h` include。

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

## 二、内部结构全景

```
FMemory.inl（252 行）
├─ 第 1~15 行     版权头 + 2 个宏默认值 + 2 个 include + 2 个前置声明
├─ 第 17~54 行    ① FMemory::Malloc（AutoRTFM::Open + OnAbort + DidAllocate）
├─ 第 56~137 行   ② FMemory::Realloc（事务里变成 Malloc+Memcpy+Free）
├─ 第 139~166 行  ③ FMemory::Free（UE_AUTORTFM_ONCOMMIT 延迟释放）
├─ 第 168~194 行  ④ FMemory::GetAllocSize（UE_AUTORTFM_OPEN + checkf）
├─ 第 196~233 行  ⑤ FMemory::MallocZeroed（同 Malloc，但清零）
└─ 第 235~252 行  ⑥ FMemory::QuantizeSize（问分配器实际给多少）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/FMemory.inl` | **是**（`FMemory` / `FMalloc` / `AutoRTFM`） | ❌ 不能（`.inl` 必须被 include） | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `HAL/LowLevelMemTracker.h` | `LLM_IF_ENABLED` / `LLM_REALLOC_SCOPE` 宏 |
| `AutoRTFM.h` | `AutoRTFM::Open` / `OnAbort` / `DidAllocate` / `DidFree` / `UE_AUTORTFM_ONCOMMIT` / `UE_AUTORTFM_OPEN` |

**两个可覆盖的宏**：

| 宏 | 默认值 | 作用 |
|---|---|---|
| `FMEMORY_INLINE_FUNCTION_DECORATOR` | 空 | 函数装饰器（可加 `FORCEINLINE`） |
| `FMEMORY_INLINE_GMalloc` | `UE::Private::GMalloc` | 全局分配器指针 |

**注意**：`SIZE_T` / `FGenericPlatformMath` / `Memcpy` / `checkf` / `TEXT` 都**没有在本文件 include**，靠 `UnrealMemory.h` 的上下文（引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\23_FMemory.inl逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `HideTCHAR.h`（1331 字节）。