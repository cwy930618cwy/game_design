# 04 — `HAL/DiskUtilizationTracker.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`（**169 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\DiskUtilizationTracker.h`
> **✅ 本节可独立编译**——只 include `CoreTypes.h`（追踪开启时再加 3 个）。

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

## 二、内部结构全景

```
DiskUtilizationTracker.h（169 行）
├─ 第 1~9 行      版权头 + TRACK_DISK_UTILIZATION 默认 0 + include CoreTypes.h
├─ 第 11~20 行    追踪开启时：3 个 include + CSV 分类 + SPEW_DISK_UTILIZATION 默认 0
├─ 第 22~143 行   ① struct FDiskUtilizationTracker
│    ├─ 第 24~92 行   struct UtilizationStats（8 个计算函数 + 6 个字段）
│    │    ├─ GetOverallThroughputBS / MBS      整体吞吐率
│    │    ├─ GetReadThrougputBS / MBS          读取吞吐率（注意引擎拼写 Througput）
│    │    ├─ GetTotalIdleTimeInSeconds         总空闲时间
│    │    ├─ GetTotalIOTimeInSeconds           总 IO 时间
│    │    ├─ GetPercentTimeIdle                空闲占比
│    │    ├─ Reset()                           清零
│    │    └─ Dump()                            打印（实现在 .cpp）
│    ├─ 第 94~105 行  6 个成员（长期/短期统计 + 临界区 + 周期 + 飞行中计数）
│    ├─ 第 107~113 行 构造函数
│    ├─ 第 115~136 行 6 个公开方法（StartRead / FinishRead / GetOutstandingRequests / GetLongTermStats / GetShortTermStats / ResetShortTermStats）
│    └─ 第 138~142 行 私有区（GetThrottleRateMBS / PrintFrequencySeconds / MaybePrint）
├─ 第 145 行      ② extern FDiskUtilizationTracker GDiskUtilizationTracker
├─ 第 147~158 行  ③ struct FScopedDiskUtilizationTracker（真实版）
├─ 第 160~167 行  ④ #else 分支：FScopedDiskUtilizationTracker 空壳版
└─ 第 169 行      #endif
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/DiskUtilizationTracker.h` | 无 | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 何时引入 | 作用 |
|---|---|---|
| `CoreTypes.h` | 总是 | `uint32` / `uint64` / `int32` |
| `HAL/PlatformMisc.h` | 追踪开启时 | `FPlatformTime` |
| `HAL/ThreadSafeBool.h` | 追踪开启时 | `FThreadSafeBool` |
| `ProfilingDebugging/CsvProfiler.h` | 追踪开启时 | `CSV_DECLARE_CATEGORY_EXTERN` |

**注意**：`FCriticalSection` 在第 97 行用到，但本文件**没有 include** `HAL/CriticalSection.h` —— 依赖调用方已经 include 过（这是引擎的隐式依赖，照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\17_DiskUtilizationTracker.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `Event.h`（4534 字节）。