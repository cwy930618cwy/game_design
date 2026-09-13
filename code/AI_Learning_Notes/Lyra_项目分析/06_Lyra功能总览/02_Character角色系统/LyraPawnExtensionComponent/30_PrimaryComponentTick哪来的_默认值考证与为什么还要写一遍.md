# 30 — `PrimaryComponentTick` 哪来的？默认是不是都开？为什么还要写一遍？

> **定位**：`LyraPawnExtensionComponent.cpp` 第 25~26 行：
>
> ```cpp
> PrimaryComponentTick.bStartWithTickEnabled = false;
> PrimaryComponentTick.bCanEverTick = false;
> ```
>
> 三个疑问一次答清（这次**有引擎源码证据**，不是猜）：
> 1. `PrimaryComponentTick` 哪来的？
> 2. 默认是都 true（打开）吗？
> 3. 每个组件都要写一遍，太蠢了吧，不能默认关吗？
>
> **衔接**：第 29 篇（构造函数逐行）。这篇专攻 `PrimaryComponentTick` 的来源与默认值真相。

---

## 一、`PrimaryComponentTick` 哪来的？—— 父类 `UActorComponent` 自带

先看它在哪声明。引擎源码 `ActorComponent.h`（L160~162）：

```cpp
	/** Main tick function for the Component */
	UPROPERTY(EditDefaultsOnly, Category="ComponentTick")
	struct FActorComponentTickFunction PrimaryComponentTick;
```

**结论**：`PrimaryComponentTick` 是 **`UActorComponent`（组件的祖宗类）内置的一个成员变量**，你的组件继承父类就自动有了，不需要自己声明。

它类型是 `FActorComponentTickFunction`，继承自 `FTickFunction`（引擎的"Tick 调度器"结构）。每个 ActorComponent 都有**一个**主 Tick 函数，就是它。

> **大白话**：所有组件"能不能每帧被调用"，都靠这一个叫 `PrimaryComponentTick` 的开关结构。它是引擎白送的，你只负责决定开不开。

---

## 二、默认是都 true（打开）吗？—— 一半对，一半错

去引擎源码 `ActorComponent.cpp`（UActorComponent 构造函数，L557~562）看真实默认值：

```cpp
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.bStartWithTickEnabled = true;     // ← ① 默认 TRUE
	PrimaryComponentTick.bCanEverTick = false;             // ← ② 默认 FALSE
	PrimaryComponentTick.bAllowTickBatching = true;
	PrimaryComponentTick.bRunTransactionally = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);     // ← ③ 注册时先禁用
```

**两个开关默认值其实是反的**：

| 开关 | 引擎默认值 | 管什么 |
|---|---|---|
| `bStartWithTickEnabled` | **true**（开） | "如果它能 tick，注册后要不要自动开始" |
| `bCanEverTick` | **false**（关） | "这个组件到底能不能 tick" |

**关键机制**：两个都要 true，组件才会真的每帧被调 `TickComponent`。引擎默认给的是：**能开关先开着（true），但"能不能"是关的（false）→ 结果：默认 tick 不了。**

> 为什么这样设计？看 L562 `SetTickFunctionEnable(false)`——组件默认情况下**不参与每帧 tick**，直到你主动开启。这是引擎故意保守的默认：**绝大多数组件不需要每帧跑，默认不开省性能。**

所以你的直觉"默认都 true 打开"——**`bStartWithTickEnabled` 确实默认 true，但 `bCanEverTick` 默认 false**。两者合起来的净效果是：**组件默认并不会每帧 tick。**

---

## 三、那 Lyra 为什么还要写一遍？（你的吐槽有道理，但有原因）

你问："太蠢了，每个文件都要写一遍，不能默认关吗？"

### 真相 1：默认本来就是"关"的，Lyra 这行**其实是冗余的**

如上考证：`UActorComponent` 构造函数已经把 `bCanEverTick = false` 设好了，组件默认就 tick 不了。所以 Lyra 写这两行，**从"能不能 tick"角度说是重复默认值**——不写效果也一样。

### 真相 2：那为什么 Lyra 还写？（三个真实理由）

**① 明确意图（给自己和队友看的文档）**
```cpp
PrimaryComponentTick.bStartWithTickEnabled = false;
PrimaryComponentTick.bCanEverTick = false;
```
光看这两行，读者立刻知道"**这个总指挥不需要每帧干活，它是事件驱动的**"（呼应第 29 篇）。不写的话，看代码的人得自己去翻引擎默认值才知道"哦它默认不 tick"。**这是把"设计决定"显式写出来，防止后人误开。**

**② 防止继承链/蓝图里被改过**
父类或中间的类有可能把默认改成 true（比如某些基类为了"默认就 tick"重设过）。在**自己的构造函数里显式设 false**，等于"无论祖上改没改，到我这必须是 false"——**锁死设计，不被继承链污染**。

**③ UE 官方模板/惯例就是"用到的成员显式初始化"**
UE 源码风格喜欢**构造函数里把所有要强调的成员写一遍**，即使与默认一致（见上面 ActorComponent 构造函数，它自己都写了一大堆默认值）。显式 > 隐式。

### 一张表总结"写了 vs 不写"

| 情况 | 效果 |
|---|---|
| 不写这两行 | 因为父类默认 `bCanEverTick=false` → 依然不会 tick ✅ |
| 写这两行 | 显式声明"我不要 tick"，并防止父类/蓝图改动 ✅（更稳、更清晰） |
| 继承链某处开了 `bCanEverTick=true` | 你不写就"继承开了"，写了就"强制关闭" ✅（写有实际作用） |

> **大白话回答你的吐槽**：
> - "不能默认关吗？" → **引擎本来默认就是关的**（`bCanEverTick=false`），你不需要写它也不会每帧跑。
> - "那 Lyra 写它干嘛？" → 不是为了让默认关（本来就关），而是**把"我不需要 tick"这个设计决定显式钉在这里**——一是当文档给人看，二是防止父类/蓝图哪天把它打开了你不知道。
> - 所以：**新手可以不写（不写也不 tick）；讲究的人写（锁死设计）。** Lyra 属于后者。

---

## 四、补充：`TickComponent` 要真正跑起来需要什么

理解了默认值，顺便把"组件每帧干活"的完整条件钉死：

```
一个组件的 TickComponent 每帧被调，需要 ALL：
① 重写/实现 UActorComponent::TickComponent()（有活干）
② bCanEverTick = true（允许 tick）
③ bStartWithTickEnabled = true（注册后自动开始）
④ SetTickFunctionEnable(true)（没被手动禁）
⑤ 组件已注册（挂到 Actor 上）
```

`LyraPawnExtensionComponent` 连 ② 都不开，所以①~④ 全部无关——**它彻底告别每帧**，只靠事件驱动（第 29 篇）。

---

## 五、总结一句话

> **`PrimaryComponentTick` 是父类 `UActorComponent` 自带的主 Tick 开关**（引擎 `ActorComponent.h` L160~162），所有组件自动拥有。它的默认值是"**`bStartWithTickEnabled=true` 但 `bCanEverTick=false`**"——所以组件**默认根本不会每帧 tick**（引擎故意保守）。Lyra 那两行**其实是显式重声明确认默认值**：① 当文档表明"我不需要 tick"；② 防止父类/蓝图把 `bCanEverTick` 改成 true 后悄悄开启；③ 符合 UE 显式初始化惯例。**不写也不会 tick，写了是锁死设计**——所以它不蠢，是"以防万一 + 给人看"的工程习惯。

---

## 六、下一步

- 反例学习：找一个需要每帧跑的组件（如 `ULyraHealthComponent` 或某个 TickComponent 非空的类），看它在构造函数怎么开 `bCanEverTick=true`，对照体会"开"和"关"的写法差异。
- 去引擎 `ActorComponent.cpp` L545~565 通读 UActorComponent 构造函数，看它初始化了多少"默认值"——理解 UE"显式初始化"文化。
- 查 `FTickFunction` / `FActorComponentTickFunction` 结构，理解 `TickGroup`、`SetTickFunctionEnable` 等其它字段。
