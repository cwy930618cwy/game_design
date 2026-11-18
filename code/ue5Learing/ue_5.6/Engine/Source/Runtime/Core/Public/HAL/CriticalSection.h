// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入平台互斥量：FPlatformSharedMutex / FPlatformRecursiveMutex / FPlatformSystemWideMutex 都来自这里。
#include "HAL/PlatformMutex.h"

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

// 私有区：底层互斥量本体。
private:
// FPlatformSharedMutex：平台共享互斥量（读写锁的底层实现）。
	FPlatformSharedMutex Mutex;
};

} // UE（命名空间结束）

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
