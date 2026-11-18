// 版权所有 Epic Games, Inc. 保留所有权利。

// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint32 / int64 都来自这里。
#include "CoreTypes.h"
// 引入数值极限：MAX_uint32 来自这里。
#include "Math/NumericLimits.h"
// 引入时间跨度：FTimespan / ETimespan 来自这里。
#include "Misc/Timespan.h"
// 引入原子类型：TAtomic 来自这里。
#include "Templates/Atomic.h"
// 引入共享指针：TSharedPtr 来自这里。
#include "Templates/SharedPointer.h"

/**
 * 可等待事件的接口。
 *
 * 本接口有各平台专属实现，用来等待另一个线程发出「我准备好了」的信号。
 * 也可以用来通知一组线程退出。
 * 
 * 
 * 建议改用 FEventRef，更安全也更方便。
 */
// FEvent：可等待事件的抽象接口，各平台有各自实现。
class FEvent
{
// 公有区。
public:

	/**
	 * 创建事件。
	 *
	 * 手动重置事件会一直保持触发状态，直到被重置。
	 * 具名事件共享同一个底层事件。
	 *
	 * @param bIsManualReset 事件是否需要手动重置。
	 * @return 创建成功返回 true，否则返回 false。
	 */
// 标记 Create 已废弃（5.0）：直接 new FEvent 性能不好，请改用 FPlatformProcess::GetSynchEventFromPool。
	UE_DEPRECATED(5.0, "Direct creation of FEvent is discouraged for performance reasons. Please use FPlatformProcess::GetSynchEventFromPool/ReturnSynchEventToPool.")
// 创建事件：bIsManualReset 为 true 表示手动重置（触发后一直保持，直到 Reset）。
	virtual bool Create( bool bIsManualReset = false ) = 0;

	/**
	 * 本事件的触发状态是否需要手动重置。
	 *
	 * @return 需要手动重置返回 true，否则返回 false。
	 * @see Reset
	 */
// 查询本事件是否需要手动重置。
	virtual bool IsManualReset() = 0;

	/**
	 * 触发事件，唤醒所有等待中的线程。
	 *
	 * @see IsManualReset, Reset
	 */
// 触发事件：唤醒所有等待中的线程。
	virtual void Trigger() = 0;

	/**
	 * 把事件重置回未触发（可等待）状态。
	 *
	 * @see IsManualReset, Trigger
	 */
// 把事件重置回未触发（可等待）状态。
	virtual void Reset() = 0;

	/**
	 * 等待事件被触发，最多等指定时长。
	 *
	 * 等待时长传 MAX_uint32 视为无限等待。
	 *
	 * @param WaitTime 等待时长（毫秒）。
	 * @param bIgnoreThreadIdleStats 为 true 时忽略线程空闲统计。
	 * @return 事件被触发返回 true，等待超时返回 false。
	 */
// 等待事件被触发，最多等 WaitTime 毫秒（MAX_uint32 表示无限等）。
	virtual bool Wait( uint32 WaitTime, const bool bIgnoreThreadIdleStats = false ) = 0;

	/**
	 * 无限等待事件被触发。
	 *
	 * @return 事件被触发返回 true。
	 */
// 无限等待的重载：直接转调 Wait(MAX_uint32)。
	bool Wait()
	{
// 传 MAX_uint32 表示无限等待。
		return Wait(MAX_uint32);
	}

	/**
	 * 等待事件被触发，最多等指定时长。
	 *
	 * @param WaitTime 等待时长。
	 * @param bIgnoreThreadIdleStats 为 true 时忽略线程空闲统计。
	 * @return 事件被触发返回 true，等待超时返回 false。
	 */
// 用 FTimespan 指定等待时长的重载。
	bool Wait( const FTimespan& WaitTime, const bool bIgnoreThreadIdleStats = false )
	{
// 断言：等待时长不能为负。
		check(WaitTime.GetTicks() >= 0);
// 把 Ticks 换算成毫秒并夹到 [0, MAX_uint32]，再转调上面的 Wait。
		return Wait((uint32)FMath::Clamp<int64>(WaitTime.GetTicks() / ETimespan::TicksPerMillisecond, 0, MAX_uint32), bIgnoreThreadIdleStats);
	}

	/** 默认构造函数。 */
// 默认构造函数：EventId 和 EventStartCycles 都清零。
	FEvent()
// 事件唯一 ID 初始化为 0。
		: EventId( 0 )
// 等待起始周期初始化为 0。
		, EventStartCycles( 0 )
	{}

	/** 虚析构函数。 */
// 虚析构函数：保证通过基类指针删除时能调到派生类析构。
	virtual ~FEvent() 
	{}

// 警告：下面这几个成员不要改。
	// 不要修改下面这些成员

	/** 推进本事件的统计。用于监控 wait→trigger 历史。 */
// 推进本事件的统计（用于监控 wait→trigger 历史）。
	CORE_API void AdvanceStats();

// 受保护区：只给派生类用。
protected:
	/** 向统计系统发一条特殊消息，编码一次「等待」。 */
// 向统计系统发一条「等待」消息。
	CORE_API void WaitForStats();

	/** 向统计系统发一条特殊消息，编码一次「触发」。 */
// 向统计系统发一条「触发」消息。
	CORE_API void TriggerForStats();

	/** 把起始周期重置为 0。 */
// 把起始周期重置为 0。
	CORE_API void ResetForStats();

	/** 用于给事件生成唯一 ID 的计数器。 */
// 静态计数器：给每个事件生成唯一 ID。
	static CORE_API TAtomic<uint32> EventUniqueId;

	/** 本事件的唯一 ID。 */
// 本事件的唯一 ID。
	uint32 EventId;

	/** 大于 0 表示事件调用过 Wait。 */
// 大于 0 表示事件调用过 Wait（记录等待起始周期）。
	TAtomic<uint32> EventStartCycles;
};

// EEventMode：事件模式，自动重置 / 手动重置。
enum class EEventMode { AutoReset, ManualReset };

/**
 * RAII 风格的池化 FEvent
 *
 * 不可拷贝、不可移动
 */
// FEventRef：RAII 风格的池化 FEvent，不可拷贝、不可移动。
class FEventRef final
{
// 公有区。
public:
// 构造函数：从池里取一个事件，默认自动重置模式。
	CORE_API explicit FEventRef(EEventMode Mode = EEventMode::AutoReset);

// 析构函数：把事件还回池里。
	CORE_API ~FEventRef();

// 删除拷贝构造。
	FEventRef(const FEventRef&) = delete;
// 删除拷贝赋值。
	FEventRef& operator=(const FEventRef&) = delete;
// 删除移动构造。
	FEventRef(FEventRef&& Other) = delete;
// 删除移动赋值。
	FEventRef& operator=(FEventRef&& Other) = delete;

// 箭头运算符：像用指针一样调用 FEvent 的方法。
	FEvent* operator->() const
	{
// 返回内部事件指针。
		return Event;
	}

// 取内部事件指针。
	FEvent* Get()
	{
// 返回内部事件指针。
		return Event;
	}

// 私有区。
private:
// 内部事件指针（由池管理生命周期）。
	FEvent* Event;
};

/**
 * RAII 风格的共享池化 FEvent
 */
// FSharedEventRef：RAII 风格的共享池化 FEvent。
class FSharedEventRef final
{
// 公有区。
public:
// 构造函数：从池里取一个事件，默认自动重置模式。
	CORE_API explicit FSharedEventRef(EEventMode Mode = EEventMode::AutoReset);

// 箭头运算符：像用指针一样调用 FEvent 的方法。
	FEvent* operator->() const
	{
// 从共享指针取裸指针。
		return Ptr.Get();
	}

// 私有区。
private:
// 共享指针：多个 FSharedEventRef 可共享同一个事件。
	TSharedPtr<FEvent> Ptr;
};
