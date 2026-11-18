# 18 — `HAL/Event.h` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`（**175 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`
> **✅ 本节可独立编译**——只 include 5 个 Core 内部头。
> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。

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

## 二、逐段代码（每行上方都有中文注释）

### 一、版权头 + 5 个 include（第 1~9 行）

```cpp
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
```

**本段新元素**

- `CoreTypes.h` —— `uint32` / `int64`
- `Math/NumericLimits.h` —— `MAX_uint32`
- `Misc/Timespan.h` —— `FTimespan` / `ETimespan::TicksPerMillisecond`
- `Templates/Atomic.h` —— `TAtomic<uint32>`
- `Templates/SharedPointer.h` —— `TSharedPtr<FEvent>`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 1 行 | 版权头 |
| 第 3 行 | 头文件保护 |
| 第 5~9 行 | 5 个 include |

> 类比：像出门前备齐五样工具：类型、极限值、时间、原子、共享指针。

### 二、`FEvent` 类说明 + 公有区（第 11~22 行）

```cpp
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
```

**本段新元素**

- `class FEvent` —— 可等待事件的抽象接口
- 说明：各平台有专属实现
- 说明：建议改用 `FEventRef`

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 11~19 行 | 类说明注释 |
| 第 20 行 | 类声明 |
| 第 22 行 | 公有区开始 |

> 类比：像插座标准：形状定好了，各国自己造插座。

### 三、`Create()` —— 已废弃（第 24~34 行）

```cpp
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
```

**本段新元素**

- `UE_DEPRECATED(5.0, "...")` —— 标记 5.0 起废弃
- `virtual bool Create(bool bIsManualReset = false) = 0` —— 纯虚函数
- `= 0` —— 纯虚，派生类必须实现
- `bIsManualReset` —— 手动重置 / 自动重置

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 24~32 行 | 函数说明注释 |
| 第 33 行 | 废弃标记 |
| 第 34 行 | 纯虚函数声明 |

> 类比：像老式电话：还能用，但门口贴了「请用手机」的告示。

### 四、`IsManualReset()` / `Trigger()` / `Reset()`（第 36~56 行）

```cpp
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
```

**本段新元素**

- `IsManualReset()` —— 查询是否需要手动重置
- `Trigger()` —— 触发事件，唤醒等待线程
- `Reset()` —— 重置回未触发状态
- 三个都是 `= 0` 纯虚函数

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 36~42 行 | `IsManualReset` |
| 第 44~49 行 | `Trigger` |
| 第 51~56 行 | `Reset` |

> 类比：像门铃的三个动作：查是不是自锁式（IsManualReset）、按一下（Trigger）、复位（Reset）。

### 五、`Wait()` 三个重载（第 58~90 行）

```cpp
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
```

**本段新元素**

- `Wait(uint32 WaitTime, bool bIgnoreThreadIdleStats = false) = 0` —— 纯虚，带超时
- `Wait()` —— 内联重载，转调 `Wait(MAX_uint32)` 无限等
- `Wait(const FTimespan&, bool)` —— 内联重载，Ticks 转毫秒
- `check(...)` —— 断言宏
- `FMath::Clamp<int64>(值, 0, MAX_uint32)` —— 夹到区间内
- `ETimespan::TicksPerMillisecond` —— 每毫秒多少 Ticks

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 58~67 行 | 带超时的纯虚 `Wait` |
| 第 69~77 行 | 无限等待重载 |
| 第 79~90 行 | `FTimespan` 重载 |

> 类比：像等快递：可以「等到天荒地老」（无限）、「最多等 1 小时」（毫秒）、「最多等 1 天 2 小时」（Timespan）。

### 六、构造 / 析构 / `AdvanceStats`（第 92~105 行）

```cpp
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
```

**本段新元素**

- `FEvent() : EventId(0), EventStartCycles(0) {}` —— 初始化列表
- `virtual ~FEvent() {}` —— 虚析构（保证派生类析构被调用）
- `CORE_API void AdvanceStats()` —— 跨模块导出
- `// DO NOT MODIFY THESE` —— 引擎警告注释

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 92~96 行 | 默认构造函数 |
| 第 98~100 行 | 虚析构函数 |
| 第 102 行 | 警告注释 |
| 第 104~105 行 | `AdvanceStats` |

> 类比：像给设备上电（构造）和断电（析构），中间还能查运行统计。

### 七、protected 区：统计方法 + 成员（第 107~125 行）

```cpp
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
```

**本段新元素**

- `protected:` —— 只给派生类访问
- `WaitForStats()` / `TriggerForStats()` / `ResetForStats()` —— 三个统计辅助函数
- `static CORE_API TAtomic<uint32> EventUniqueId` —— 静态原子计数器
- `uint32 EventId` —— 本事件唯一 ID
- `TAtomic<uint32> EventStartCycles` —— 等待起始周期（原子）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 107 行 | protected 开始 |
| 第 108~115 行 | 3 个统计函数 |
| 第 117~118 行 | 静态唯一 ID 计数器 |
| 第 120~124 行 | 2 个成员变量 |

> 类比：像设备的内部编号牌：外面看不到，但每个设备都有唯一编号。

### 八、`EEventMode` 枚举（第 127~127 行）

```cpp
// EEventMode：事件模式，自动重置 / 手动重置。
enum class EEventMode { AutoReset, ManualReset };
```

**本段新元素**

- `enum class EEventMode` —— 强类型枚举
- `AutoReset` —— 触发后自动复位（一次唤醒一个）
- `ManualReset` —— 触发后保持（唤醒所有，直到手动 Reset）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 127 行 | 枚举定义（2 个值） |

> 类比：像两种门铃：按一下就自动弹回（AutoReset），或者按下去一直响（ManualReset）。

### 九、`FEventRef` —— RAII 池化（第 129~158 行）

```cpp
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
```

**本段新元素**

- `class FEventRef final` —— 不可继承
- `explicit FEventRef(EEventMode Mode = EEventMode::AutoReset)` —— 显式构造，默认自动重置
- `~FEventRef()` —— 析构时还回池
- 4 个 `= delete` —— 禁拷贝 + 禁移动
- `FEvent* operator->() const` —— 箭头运算符重载
- `FEvent* Get()` —— 取裸指针
- `FEvent* Event` —— 私有成员

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 129~134 行 | 类声明 + 说明 |
| 第 137~139 行 | 构造 / 析构 |
| 第 141~144 行 | 4 个 delete |
| 第 146~154 行 | `operator->` + `Get` |
| 第 156~157 行 | 私有成员 |

> 类比：像租车：租的时候给你钥匙（构造），还车时自动结算（析构），不能转租给别人（禁拷贝）。

### 十、`FSharedEventRef` —— RAII 共享池化（第 160~175 行）

```cpp
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
```

**本段新元素**

- `class FSharedEventRef final` —— 不可继承
- `explicit FSharedEventRef(EEventMode Mode = EEventMode::AutoReset)` —— 构造
- `FEvent* operator->() const` —— 箭头运算符
- `TSharedPtr<FEvent> ptr` —— 共享指针（多个 FSharedEventRef 可共享同一事件）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| 第 160~163 行 | 类声明 + 说明 |
| 第 166 行 | 构造函数 |
| 第 168~171 行 | `operator->` |
| 第 173~174 行 | 私有共享指针 |

> 类比：像共享单车：多个人可以骑同一辆（共享指针），最后一个还车时才真正回收。

---

## 三、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 版权头 + 5 个 include | 1~9 | 版权头 + 5 个 include |
| `FEvent` 类说明 + 公有区 | 11~22 | `FEvent` 类说明 + 公有区 |
| `Create()` —— 已废弃 | 24~34 | `Create()` —— 已废弃 |
| `IsManualReset()` / `Trigger()` / `Reset()` | 36~56 | `IsManualReset()` / `Trigger()` / `Reset()` |
| `Wait()` 三个重载 | 58~90 | `Wait()` 三个重载 |
| 构造 / 析构 / `AdvanceStats` | 92~105 | 构造 / 析构 / `AdvanceStats` |
| protected 区：统计方法 + 成员 | 107~125 | protected 区：统计方法 + 成员 |
| `EEventMode` 枚举 | 127~127 | `EEventMode` 枚举 |
| `FEventRef` —— RAII 池化 | 129~158 | `FEventRef` —— RAII 池化 |
| `FSharedEventRef` —— RAII 共享池化 | 160~175 | `FSharedEventRef` —— RAII 共享池化 |

---

## 四、C++ 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `virtual T F() = 0;` | 纯虚函数，派生类必须实现 | `virtual void Trigger() = 0;` |
| `virtual ~X()` | 虚析构函数 | `virtual ~FEvent() {}` |
| `UE_DEPRECATED(v, "msg")` | 标记废弃 | `UE_DEPRECATED(5.0, "...")` |
| `X() : a(0), b(0) {}` | 初始化列表 | `FEvent() : EventId(0), EventStartCycles(0) {}` |
| `explicit X(...)` | 禁止隐式转换 | `explicit FEventRef(EEventMode Mode = ...)` |
| `X(const X&) = delete;` | 删除拷贝构造 | `FEventRef(const FEventRef&) = delete;` |
| `T* operator->() const` | 箭头运算符重载 | `FEvent* operator->() const` |
| `static CORE_API TAtomic<uint32> X` | 静态原子成员 | `static CORE_API TAtomic<uint32> EventUniqueId;` |
| `enum class X { A, B }` | 强类型枚举 | `enum class EEventMode { AutoReset, ManualReset };` |
| `check(cond)` | 断言宏 | `check(WaitTime.GetTicks() >= 0);` |

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎 | 我们 | 说明 |
|---|---|---|---|
| 代码行 | 175 行 | 230 行 | 多出的是每行上方的中文注释 |
| 非注释代码行 | 71 行 | 71 行 | **逐字一致** |
| 注释语言 | 英文 | 中文 | 全部译成中文 |
| `UE_DEPRECATED` 字符串 | 英文原文 | 保留英文 | API 契约，不翻译 |

---

## 待确认

1. 还原文件已生成：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`（230 行）
2. 脚本验收：`codeOurs=71 / codeEngine=71 / diff=0`
3. 确认后下一个：`HAL\ExceptionHandling.h`（2423 字节）

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`（175 行）
- 还原：`E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\Event.h`（230 行）
- 总 md：`AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\05_Event.h_总md.md`