# 31 — `SetIsReplicatedByDefault(true)` 是干嘛的？组件"出生时申请网络复制"

> **定位**：`LyraPawnExtensionComponent.cpp` 第 28 行：
>
> ```cpp
> SetIsReplicatedByDefault(true);
> ```
>
> 这一行是**组件告诉引擎："我要默认参与网络复制，请把我也同步到客户端。"** 这篇从引擎源码讲透：它到底设了什么、为什么必须在构造函数里调、和 `DOREPLIFETIME`/`ReplicatedUsing` 什么关系。
>
> **衔接**：第 29 篇（构造函数逐行）只概述了这行，这篇深入。配合 LyraPawn/38 篇的"广播站故事"理解复制框架。

---

## 〇、30 秒先给答案

| 问题 | 答案 |
|---|---|
| 它干嘛？ | 把组件的 **`bReplicates` 标志设为 true** → 这个组件跟着 Actor 一起复制到客户端 |
| 在哪调？ | **构造函数里**（引擎注释明确：只能在构造期间调） |
| 为什么叫 ByDefault？ | 它是设"出生默认值"，不是运行时开关（运行时要改走 `SetIsReplicated`） |
| Lyra 为什么开？ | 因为组件身上有要同步的 `PawnData`（配方要发给客户端，第 01 篇） |

---

## 一、引擎源码：它到底设了什么？（先看真实实现）

引擎 `ActorComponent.cpp` L3240~3253：

```cpp
void UActorComponent::SetIsReplicatedByDefault(const bool bNewReplicates)
{
	// Don't bother checking parent here.
	if (LIKELY(NeedsInitialization()))            // ← 还在构造/初始化阶段？
	{
		bReplicates = bNewReplicates;             // ← 直接设标志 = true
		MARK_PROPERTY_DIRTY_FROM_NAME(UActorComponent, bReplicates, this);
	}
	else
	{
		ensureMsgf(false, TEXT("SetIsReplicatedByDefault should only be called during Component Construction..."));
		SetIsReplicated(bNewReplicates);          // ← 否则走运行时版本
	}
}
```

头文件注释（`ActorComponent.h` L1388~1394）：

```cpp
	/**
	 * Sets the value of bReplicates without causing other side effects to this instance.
	 * This should only be called during component construction.
	 *
	 * This method exists only to allow code to directly modify bReplicates ...
	 */
	ENGINE_API void SetIsReplicatedByDefault(const bool bNewReplicates);
```

**结论**：它干的事**非常简单**——把组件内部的一个私有标志 **`bReplicates`** 设成 `true`。附带一个约束：**只能在组件构造期间调用**（`NeedsInitialization()` 为真），否则会 ensure 报错并退化为运行时 `SetIsReplicated`。

> **大白话**：组件内部有个"我要不要跟着 Actor 复制"的私密小开关（`bReplicates`）。这行就是**在组件刚出生时把这个开关拨到"开"**。

---

## 二、为什么组件默认不复制？（bReplicates 默认 false）

回顾第 29 篇读过的 `UActorComponent` 构造函数（ActorComponent.cpp L545~585）——通篇**没有把 `bReplicates` 设为 true**，所以组件的 `bReplicates` **默认是 false**。

**为什么默认关？** 引擎的设计哲学（呼应第 30 篇的关 Tick）：
- 不是所有组件都要同步（纯本地的摄像机、特效辅助组件等不需要）。
- 复制**有带宽成本**：每帧扫描、打包、发送。
- 所以引擎**默认不复制**，需要同步的组件**自己申请**——`SetIsReplicatedByDefault(true)` 就是"申请"动作。

> **场景**：你给角色挂个"小地图标记组件"（纯本地 UI 用）→ 不调这行，它不复制，省带宽；你挂个"血量组件"（所有人都要看）→ 调这行，让它跟着复制。

---

## 三、为什么"必须"在构造函数里调？

引擎实现里用 `NeedsInitialization()` 判断：**还在构造阶段才能直接设标志**；运行后再调就 ensure 报错、改走 `SetIsReplicated`。

为什么构造期间特殊？看两段代码的差异：

| 函数 | 什么时候用 | 行为 |
|---|---|---|
| `SetIsReplicatedByDefault` | **构造函数**（组件还没挂到 Actor 上） | 只设标志，无副作用 |
| `SetIsReplicated` | 运行时 | 设标志 + `UpdateReplicatedComponent`（**通知 Actor 重新登记**，有副作用、有开销） |

**逻辑**：组件刚 new 出来还没挂到 Actor 时，改 `bReplicates` 不需要"通知 Actor 重算"（Actor 还没拿到它）；等注册时引擎会**根据 bReplicates 决定这个组件要不要进 Actor 的复制列表**。所以**在构造函数里设 = 零副作用、一次到位**。

> **类比**：员工入职时在表格上勾选"需要配电脑"（构造时声明）——人事部门（Actor）之后统一按表格配；如果入职后才喊"我要电脑"（运行时），就得走加急流程、额外协调（`UpdateReplicatedComponent`）。

---

## 四、和 Actor 的 `bReplicates` 区别（别混）

Actor（Pawn/Character）也有复制标志——是 **`AActor::SetReplicates(true)`**，管"这个 Actor 要不要同步到客户端"。组件这行是**另一个**更细的开关：

```
AActor::SetReplicates(true)          ← Actor 级：整个角色要不要复制
UActorComponent::SetIsReplicatedByDefault(true)  ← 组件级：这个组件要不要复制
```

**两者的关系（层层授权）**：

```
服务器要把角色同步给客户端
  │ 需要 Actor.bReplicates = true     （角色整体愿意复制）
  ▼
还要 Actor 上的组件 bReplicates = true  （这个组件愿意跟着复制）
  ▼
还要该组件里具体属性被 DOREPLIFETIME 登记（属性在复制花名册里）
```

> 三个条件**逐层满足**，属性才真的同步：**Actor 愿意复制 → 组件愿意复制 → 属性登记了**。`SetIsReplicatedByDefault(true)` 管中间那一层（组件级）。

---

## 五、它 vs `DOREPLIFETIME` vs `ReplicatedUsing`：三者的分工

这是最容易混的一组，放一起排清楚（角色 LyraPawn/38 篇的广播站故事继续）：

| 代码 | 在哪 | 管哪层 | 广播站类比 |
|---|---|---|---|
| `AActor::SetReplicates(true)` | Actor 构造 | **Actor 整体**复制 | "我这栋楼要不要装广播" |
| `SetIsReplicatedByDefault(true)` | **组件构造**（本篇） | **组件**复制 | "我这层楼要不要也装分机" |
| `.h` 属性 `ReplicatedUsing = OnRep_X` | 属性声明 | 到达后**调谁** | "收到广播敲哪面锣" |
| `.cpp` `DOREPLIFETIME` | GetLifetimeReplicatedProps | 属性进**同步名单** | "这面锣的内容值不值得播" |

**具体到 LyraPawnExtensionComponent**：
- 组件整体：`SetIsReplicatedByDefault(true)` → "我这个组件要复制"
- 组件内属性 `PawnData`：`.h` `ReplicatedUsing = OnRep_PawnData` + `.cpp` 构造函数里 `GetLifetimeReplicatedProps` 的 `DOREPLIFETIME` → "PawnData 这个属性在名单里、到了调 OnRep"

```
SetIsReplicatedByDefault(true)    ← ① 组件层开关（本篇）
PawnData ReplicatedUsing + DOREPLIFETIME   ← ② 属性层登记
→ PawnData 才能在服务器→客户端同步（第 01 篇客户端靠 OnRep_PawnData 推进初始化）
```

> **记忆**：`SetIsReplicatedByDefault(true)` = **给整个组件申请"复制资格"**；`DOREPLIFETIME` = 组件里有资格后，**具体哪个属性值得同步**再逐个登记。一个是"组件级入场券"，一个是"属性级名单"。

---

## 六、那组件里所有属性都自动复制吗？（重要澄清）

**不是。** `SetIsReplicatedByDefault(true)` 只是让"**这个组件**"进入复制流程，但**组件里每个属性是否同步，仍要单独登记**（`GetLifetimeReplicatedProps` + `DOREPLIFETIME`）。

对照 LyraPawnExtensionComponent：
- `PawnData` → 登记了（复制到客户端）
- `AbilitySystemComponent` → `.h` 标了 `Transient`（瞬态、不持久），也不在复制名单里（它只是本地缓存指针）

> **所以两行配合缺一不可**：
> - 只调 `SetIsReplicatedByDefault(true)` 不登记属性 → 组件复制了但没内容可同步；
> - 只登记属性不调 `SetIsReplicatedByDefault` → 组件整体不复制，属性登记也白搭。

---

## 七、总结一句话

> **`SetIsReplicatedByDefault(true)` 是组件在构造函数里申请"整个组件默认参与网络复制"**：它把组件内部的 `bReplicates` 标志设为 true（引擎源码 `ActorComponent.cpp` L3240 实锤）。必须在构造期间调（零副作用）；运行时改要走 `SetIsReplicated`。它管的是**组件级**的复制资格，和 **Actor 级**（`AActor::SetReplicates`）、**属性级**（`.h` 的 `ReplicatedUsing` + `.cpp` 的 `DOREPLIFETIME`）是层层递进的三个开关。Lyra 的 PawnExtension 开它，是为了让身上的 `PawnData`（配方）能同步给客户端、触发初始化（第 01 篇）。

---

## 八、下一步

- 追 `AActor::SetReplicates` 与组件 `bReplicates` 在 `UpdateReplicatedComponent` 里怎么联动（组件注册时如何被决定进不进复制列表）。
- 对照引擎 `ActorComponent.cpp` L2915~2928：注意 `bReplicates` 本身也被复制到客户端，理解"组件复制标志也要同步"的设计。
- 回看第 01 篇 PawnData 的 `OnRep_PawnData` 客户端流程，把"组件复制 → 属性复制 → 触发初始化"整条链串起来。
