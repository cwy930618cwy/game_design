# 03 — `HAL/CriticalSection.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`（**59 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`
> **✅ 本节可独立编译**——只 include `HAL/PlatformMutex.h`。

---

## 一、这个文件解决什么问题（为什么）

多线程同时改同一块数据会出乱子。UE 用**互斥量**（mutex）解决：谁先拿到锁谁改，别人排队等。但不同平台（Windows / Linux / Mac / Android）的互斥量 API 各不相同，所以 UE 在 `PlatformMutex.h` 里定义了统一的 `FPlatformSharedMutex` / `FPlatformRecursiveMutex` / `FPlatformSystemWideMutex`。

`CriticalSection.h` 做两件事：

1. **把 `FPlatformSharedMutex` 包装成读写锁**（`UE::FPlatformRWLock`）—— 因为 `FPlatformSharedMutex` 的方法名是 `Lock` / `LockShared`，而 UE 上层代码期望的是 `WriteLock` / `ReadLock`
2. **给三个常用类型起短别名** —— `FCriticalSection` / `FRWLock` / `FSystemWideCriticalSection`

**说个场景**：你写 `FCriticalSection Lock;` 保护一个 `TArray`。`FCriticalSection` 其实就是 `UE::FPlatformRecursiveMutex` 的别名 —— 同一线程可以重复加锁（递归），不会自己把自己锁死。

> 🔑 **一句话**：`CriticalSection.h` = **互斥量的「统一门面」**，把平台差异藏起来，给上层三个好记的名字。

---

## 二、内部结构全景

```
CriticalSection.h（59 行）
├─ 第 1~5 行     版权头 + #pragma once + include PlatformMutex.h
├─ 第 7~50 行    ① namespace UE { class FPlatformRWLock }
│    ├─ 第 16~19 行  TryWriteLock()   → Mutex.TryLock()
│    ├─ 第 21~24 行  WriteLock()      → Mutex.Lock()
│    ├─ 第 26~29 行  WriteUnlock()    → Mutex.Unlock()
│    ├─ 第 31~34 行  TryReadLock()    → Mutex.TryLockShared()
│    ├─ 第 36~39 行  ReadLock()       → Mutex.LockShared()
│    ├─ 第 41~44 行  ReadUnlock()     → Mutex.UnlockShared()
│    └─ 第 46~48 行  FPlatformSharedMutex Mutex（私有成员）
├─ 第 52~53 行    ② using FCriticalSection = UE::FPlatformRecursiveMutex
├─ 第 55~56 行    ③ using FRWLock = UE::FPlatformRWLock
└─ 第 58~59 行    ④ using FSystemWideCriticalSection = UE::FPlatformSystemWideMutex
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/CriticalSection.h` | 无（只 include `HAL/PlatformMutex.h`） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `HAL/PlatformMutex.h` | `FPlatformSharedMutex` / `FPlatformRecursiveMutex` / `FPlatformSystemWideMutex` 的定义 |

**三个别名的区别**：

| 别名 | 底层类型 | 递归？ | 跨进程？ | 用途 |
|---|---|---|---|---|
| `FCriticalSection` | `FPlatformRecursiveMutex` | ✅ 支持 | ❌ | 最常用，保护普通数据 |
| `FRWLock` | `UE::FPlatformRWLock` | ❌ 不支持 | ❌ | 多读单写场景（读多写少） |
| `FSystemWideCriticalSection` | `FPlatformSystemWideMutex` | ❌ 不支持 | ✅ 支持 | 跨进程同步（如单实例检测） |

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\16_CriticalSection.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `DiskUtilizationTracker.h`（3302 字节）。