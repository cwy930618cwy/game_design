# 05 — `HAL/Event.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`（**175 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`
> **✅ 本节可独立编译**——只 include 5 个 Core 内部头。

---

## 一、这个文件解决什么问题（为什么）

线程 A 干完活，要通知线程 B「可以开始了」。最土的办法是 `while(!bReady) {}` —— 空转烧 CPU。正确做法是**事件（Event）**：线程 B 调 `Wait()` 睡过去，线程 A 干完调 `Trigger()` 把它叫醒。

`Event.h` 定义了三层：

| 层 | 类型 | 作用 |
|---|---|---|
| 接口 | `FEvent` | 抽象基类，5 个纯虚函数（`Create` / `IsManualReset` / `Trigger` / `Reset` / `Wait`） |
| 模式 | `EEventMode` | `AutoReset`（触发后自动复位）/ `ManualReset`（触发后保持） |
| RAII 包装 | `FEventRef` / `FSharedEventRef` | 从池里取事件，析构自动还回去 |

**说个场景**：渲染线程要等游戏线程把场景数据准备好。

```cpp
FEventRef Event;              // 从池里取一个事件（自动重置模式）
// 游戏线程：准备好数据后
Event->Trigger();             // 唤醒等待者
// 渲染线程：
Event->Wait(1000);            // 最多等 1 秒
```

**为什么推荐 `FEventRef` 而不是直接 `new FEvent`**：`FEvent` 的 `Create()` 在 5.0 就被标记废弃了（第 33 行），因为每次 new 都要走系统调用创建内核对象，很慢。`FEventRef` 从**对象池**里取，用完还回去，避免反复创建销毁。

> 🔑 **一句话**：`Event.h` = **线程间的「门铃」**，`Wait()` 是等门铃响，`Trigger()` 是按门铃，`FEventRef` 是「门铃租借服务」。

---

## 二、内部结构全景

```
Event.h（175 行）
├─ 第 1~9 行      版权头 + #pragma once + 5 个 include
├─ 第 11~125 行   ① class FEvent（抽象接口）
│    ├─ 第 24~34 行   Create(bIsManualReset) = 0        ← 已废弃（5.0）
│    ├─ 第 36~42 行   IsManualReset() = 0
│    ├─ 第 44~49 行   Trigger() = 0
│    ├─ 第 51~56 行   Reset() = 0
│    ├─ 第 58~67 行   Wait(WaitTime, bIgnoreThreadIdleStats) = 0
│    ├─ 第 69~77 行   Wait()                             ← 内联，转调 Wait(MAX_uint32)
│    ├─ 第 79~90 行   Wait(FTimespan, ...)               ← 内联，Ticks 转毫秒
│    ├─ 第 92~96 行   构造函数（EventId / EventStartCycles 清零）
│    ├─ 第 98~100 行  虚析构函数
│    ├─ 第 102~105 行 AdvanceStats()
│    ├─ 第 107~115 行 protected：WaitForStats / TriggerForStats / ResetForStats
│    └─ 第 117~124 行 protected：EventUniqueId / EventId / EventStartCycles
├─ 第 127 行      ② enum class EEventMode { AutoReset, ManualReset }
├─ 第 129~158 行  ③ class FEventRef final（RAII 池化）
└─ 第 160~175 行  ④ class FSharedEventRef final（RAII 共享池化）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/Event.h` | 无 | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint32` / `int64` |
| `Math/NumericLimits.h` | `MAX_uint32` |
| `Misc/Timespan.h` | `FTimespan` / `ETimespan::TicksPerMillisecond` |
| `Templates/Atomic.h` | `TAtomic<uint32>` |
| `Templates/SharedPointer.h` | `TSharedPtr<FEvent>` |

**注意**：`FEvent` 是**纯抽象类**（5 个 `= 0`），真正实现在各平台的 `FEventWin` / `FEventLinux` 等类里，通过 `FPlatformProcess::GetSynchEventFromPool()` 创建。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\18_Event.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `ExceptionHandling.h`（2423 字节）。