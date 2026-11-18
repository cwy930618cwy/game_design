# 16 — `HAL/CriticalSection.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`（**59 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`
> **✅ 本节可独立编译**——只 include `HAL/PlatformMutex.h`。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

---

## 一、这个文件解决什么问题（为什么）

多线程同时改同一块数据会出乱子。UE 用**互斥量**（mutex）解决：谁先拿到锁谁改，别人排队等。但不同平台（Windows / Linux / Mac / Android）的互斥量 API 各不相同，所以 UE 在 `PlatformMutex.h` 里定义了统一的 `FPlatformSharedMutex` / `FPlatformRecursiveMutex` / `FPlatformSystemWideMutex`。

`CriticalSection.h` 做两件事：

1. **把 `FPlatformSharedMutex` 包装成读写锁**（`UE::FPlatformRWLock`）—— 因为 `FPlatformSharedMutex` 的方法名是 `Lock` / `LockShared`，而 UE 上层代码期望的是 `WriteLock` / `ReadLock`
2. **给三个常用类型起短别名** —— `FCriticalSection` / `FRWLock` / `FSystemWideCriticalSection`

**说个场景**：你写 `FCriticalSection Lock;` 保护一个 `TArray`。`FCriticalSection` 其实就是 `UE::FPlatformRecursiveMutex` 的别名 —— 同一线程可以重复加锁（递归），不会自己把自己锁死。

> 🔑 **一句话**：`CriticalSection.h` = **互斥量的「统一门面」**，把平台差异藏起来，给上层三个好记的名字。

---

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + include（第 1~5 行）

```cpp
// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入平台互斥量：FPlatformSharedMutex / FPlatformRecursiveMutex / FPlatformSystemWideMutex 都来自这里。
#include "HAL/PlatformMutex.h"
```

**本段新元素**

- `#pragma once` —— 头文件防重复包含
- `HAL/PlatformMutex.h` —— 三个平台互斥量类型的定义来源

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5 行 | 引入平台互斥量 |

> 类比：像开工前先领工具：互斥量是后面所有代码要用的家伙。

### 二、`UE::FPlatformRWLock` 类声明（第 7~15 行）

```cpp
// 命名空间 UE：引擎新版公共 API 都放这里。
namespace UE
{

/**
 * 把 FPlatformSharedMutex 适配成 FRWLock 期望的 API。
 */
// FPlatformRWLock：读写锁适配器，final 表示不允许再被继承。
class FPlatformRWLock final
{
// 公有区：对外暴露 6 个加解锁方法。
public:
```

**本段新元素**

- `namespace UE` —— 引擎新版公共 API 命名空间
- `class FPlatformRWLock final` —— 读写锁适配器，`final` 禁止继承
- `public:` —— 公有区，对外暴露 6 个方法

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 7~8 行 | 命名空间开始 |
| 第 10~12 行 | 类说明注释 |
| 第 13 行 | 类声明 + final |
| 第 15 行 | 公有区开始 |

> 类比：像给旧机器装个新面板：里面还是老零件，外面按钮换了好认的名字。

### 三、写锁三件套（第 16~29 行）

```cpp
// 尝试获取写锁：成功返回 true，失败返回 false（不阻塞）。
	FORCEINLINE bool TryWriteLock()
	{
// 转调底层互斥量的 TryLock。
		return Mutex.TryLock();
	}

// 获取写锁：拿不到就阻塞等待。
	FORCEINLINE void WriteLock()
	{
// 转调底层互斥量的 Lock。
		Mutex.Lock();
	}

// 释放写锁。
	FORCEINLINE void WriteUnlock()
	{
// 转调底层互斥量的 Unlock。
		Mutex.Unlock();
	}
```

**本段新元素**

- `FORCEINLINE` —— 强制内联，零调用开销
- `TryWriteLock()` —— 尝试获取写锁，不阻塞
- `WriteLock()` —— 获取写锁，阻塞等待
- `WriteUnlock()` —— 释放写锁
- `Mutex.TryLock()` / `Mutex.Lock()` / `Mutex.Unlock()` —— 底层平台互斥量的对应方法

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 16~19 行 | `TryWriteLock` → `TryLock` |
| 第 21~24 行 | `WriteLock` → `Lock` |
| 第 26~29 行 | `WriteUnlock` → `Unlock` |

> 类比：像厕所门上的三个动作：敲一下看有没有人（Try）、排队等（Lock）、出来开门（Unlock）。

### 四、读锁三件套（第 31~44 行）

```cpp
// 尝试获取读锁：成功返回 true，失败返回 false（不阻塞）。
	FORCEINLINE bool TryReadLock()
	{
// 转调底层互斥量的 TryLockShared。
		return Mutex.TryLockShared();
	}

// 获取读锁：拿不到就阻塞等待。
	FORCEINLINE void ReadLock()
	{
// 转调底层互斥量的 LockShared。
		Mutex.LockShared();
	}

// 释放读锁。
	FORCEINLINE void ReadUnlock()
	{
// 转调底层互斥量的 UnlockShared。
		Mutex.UnlockShared();
	}
```

**本段新元素**

- `TryReadLock()` —— 尝试获取读锁，不阻塞
- `ReadLock()` —— 获取读锁，阻塞等待
- `ReadUnlock()` —— 释放读锁
- `Mutex.TryLockShared()` / `Mutex.LockShared()` / `Mutex.UnlockShared()` —— 底层共享锁方法

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 31~34 行 | `TryReadLock` → `TryLockShared` |
| 第 36~39 行 | `ReadLock` → `LockShared` |
| 第 41~44 行 | `ReadUnlock` → `UnlockShared` |

> 类比：像图书馆：很多人可以同时看（共享读），但有人要改书就得清场（独占写）。

### 五、私有成员 + 类结束（第 46~50 行）

```cpp
// 私有区：底层互斥量本体。
private:
// FPlatformSharedMutex：平台共享互斥量（读写锁的底层实现）。
	FPlatformSharedMutex Mutex;
};

} // UE（命名空间结束）
```

**本段新元素**

- `private:` —— 私有区
- `FPlatformSharedMutex Mutex` —— 底层平台共享互斥量

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 46 行 | 私有区开始 |
| 第 47 行 | 底层互斥量成员 |
| 第 48 行 | 类结束 |
| 第 50 行 | 命名空间结束 |

> 类比：像发动机藏在车壳里：外面只看到方向盘和油门。

### 六、三个类型别名（第 52~59 行）

```cpp
// 说明：FCriticalSection 的别名定义，支持递归加锁，可能不公平。
/** Alias for a mutex that supports recursive locking and may not be fair. */
// FCriticalSection：递归互斥量别名（同一线程可重复加锁）。
using FCriticalSection = UE::FPlatformRecursiveMutex;

// 说明：FRWLock 的别名定义，不支持递归加锁，可能不公平。
/** Alias for a shared mutex that does not support recursive locking and may not be fair. */
// FRWLock：读写锁别名（多读单写）。
using FRWLock = UE::FPlatformRWLock;

// 说明：FSystemWideCriticalSection 的别名定义，跨进程互斥，不支持递归加锁。
/** Alias for a system-wide (cross-process) mutex that does not support recursive locking and may not be fair. */
// FSystemWideCriticalSection：系统级（跨进程）互斥量别名。
using FSystemWideCriticalSection = UE::FPlatformSystemWideMutex;
```

**本段新元素**

- `using X = Y;` —— C++11 类型别名（等价于 `typedef`）
- `FCriticalSection` —— 递归互斥量别名（同一线程可重复加锁）
- `FRWLock` —— 读写锁别名（多读单写）
- `FSystemWideCriticalSection` —— 跨进程互斥量别名

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 52~53 行 | `FCriticalSection` 别名 |
| 第 55~56 行 | `FRWLock` 别名 |
| 第 58~59 行 | `FSystemWideCriticalSection` 别名 |

> 类比：像给人起小名：正式名字太长，日常都叫小名。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + include | 1~5 | 版权头 + include |
| `UE::FPlatformRWLock` 类声明 | 7~15 | `UE::FPlatformRWLock` 类声明 |
| 写锁三件套 | 16~29 | 写锁三件套 |
| 读锁三件套 | 31~44 | 读锁三件套 |
| 私有成员 + 类结束 | 46~50 | 私有成员 + 类结束 |
| 三个类型别名 | 52~59 | 三个类型别名 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `namespace X { }` | 命名空间 | `namespace UE { }` |
| `class X final` | 类，禁止被继承 | `class FPlatformRWLock final` |
| `FORCEINLINE` | 强制内联 | `FORCEINLINE bool TryWriteLock()` |
| `using A = B;` | 类型别名（C++11） | `using FCriticalSection = UE::FPlatformRecursiveMutex;` |
| `public:` / `private:` | 访问控制 | `private: FPlatformSharedMutex Mutex;` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 59 行 | 89 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 38 行 | 38 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `} // UE` | 保留原文 | 保留原文 + 中文说明 | 命名空间标记，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`（89 行）
2. 脚本验收：`codeOurs=38 / codeEngine=38 / diff=0`
3. 确认后下一个：`HAL\DiskUtilizationTracker.h`（3302 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`（59 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\CriticalSection.h`（89 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\03_CriticalSection.h_总md.md`