# 16 — UE5 官方 GAS 底层怎么改属性？和 Lyra 的实现有什么区别？

> **定位**：第 15 篇讲完了 Lyra 的"属性集 + 伤害公式"。但你会有个疑问：**这套东西，到底是 UE5.6 官方 GAS 本来就有的，还是 Lyra 自己发明的？官方的 AttributeSet / Execution 底层是怎么工作的？Lyra 跟官方到底差在哪？**
>
> 这篇就回答这个。讲清楚：**官方引擎"出厂自带"到哪一步，Lyra 从哪一步开始"接手干活"，以及两者本质区别。**

---

## 〇、结论先放这儿：Lyra 几乎没碰官方引擎，它只是"把官方给你的钩子填满"

> **官方 GAS 的机制原样在跑，Lyra 一个引擎代码都没改。** Lyra 做的全是"**在官方留好的回调钩子里填入游戏逻辑 + 包一层业务封装**"。

一个类比：
- **官方 = 给你一台有自动化流水线的机器**：你把"原料（效果）"丢进去，它自动完成"混合（计算）→ 灌装（改属性）"，还留了几个"检修窗口（回调钩子）"。
- **Lyra = 在这台机器的检修窗口里，装上了你们工厂的质检员**：血条不许负、无敌角色不掉血、血到 0 就拉警报（死亡事件）……机器没换，是"谁在窗口里干活"变了。

下面分开看官方底层和 Lyra 扩展。

---

## 一、官方 UE5.6 GAS：属性到底是怎么被"改"的？（底层机制）

### 机制 1：属性是"有反射的受管值"，不是裸变量

官方要求你的数值必须用 `FGameplayAttributeData` 存，并且**用 UPROPERTY 暴露给反射系统**。为什么？

因为 ASC 不知道你有几个属性、属性叫啥，它靠 **UHT 反射**去扫描你的 `UAttributeSet` 子类，把所有属性登记进它的系统。看官方标准的声明方式（官方要求你每个属性都这么写）：

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Health")
FGameplayAttributeData Health;
```

- `FGameplayAttributeData`：存数值的容器（`float` 的封装 + 可同步标记）。
- `ReplicatedUsing`：血要网络同步，变了一次就把 `OnRep_Health` 同步给别的端。
- `UPROPERTY`：让**反射系统能看到它**，ASC 才能遍历、才能被 `GameplayEffect` 锁定修改。

> **底层真相**：官方"改属性"不是 `Health -= 10` 这种 C++ 直接赋值，而是 **ASC 通过反射找到这个属性 → 计算好要改成多少 → 通过属性系统的内部接口写进去**。你用 `SetHealth()` 只是走了一层"请求"。

### 机制 2：效果的数值，由 ASC 的"聚合器"统一算

官方对"一个属性当前是多少"维护着一堆**聚合器（Aggregator）**。一个 `GameplayEffect` 带来的数值修改叫 **Modifier**。效果叠上去时，聚合器把所有 Modifier 按优先级叠算，得出当前值。

（这是官方核心，但**你自己写代码时通常不用碰**——它是引擎内部。）

### 机制 3：官方给你留的"检修窗口"（回调钩子）

官方 `UAttributeSet` 留了一堆 `virtual` 虚函数，让你在"改属性"的前前后后插自己的逻辑。这是 Lyra 大量使用的部分。官方签名大致如下（机制准确，行号属官方内部不作标注）：

```cpp
// 改属性"之前"触发（还能阻止本次修改/拦截数值）
virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data);

// 改属性"之后"触发（通常在这里处理"收到伤害该干嘛"）
virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data);

// 属性基础值要变之前（钳制血上限等）
virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const;

// 属性当前值要变之前
virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);
```

### 机制 4：官方也给了"自定义计算器" Execution

官方 `GameplayEffectExecutionCalculation`（Execution）**本来就有**——它允许你在一个 `GameplayEffect` 里挂一个子类，生效时跑自定义计算，算出要改哪些属性。Lyra 的 `ULyraDamageExecution` 就继承自它。

> **到这里你就明白了**：**属性怎么存、效果怎么叠算、伤害能不能自定义算——这些官方底层全都有了。** Lyra 要做的只是：**决定"用这些钩子具体跑什么游戏规则"。**

---

## 二、那 Lyra 到底"额外做了"什么？（对比官方）

把官方机制和 Lyra 的做法摆一起看区别：

| 环节 | 官方 UE5.6 GAS（出厂） | Lyra 的实现 | 区别本质 |
|---|---|---|---|
| 属性怎么存 | 只给 `UAttributeSet` 空壳，你自建 | 建 `ULyraAttributeSet` 基类 + `HealthSet`/`CombatSet` | **业务分层**：Lyra 把空壳拆成有含义的子类 |
| 伤害数值类型 | 官方无约定 | `Health` + 一个 **meta 中转 `Damage`** | **Lyra 加了中间层**（见下） |
| 改属性后的处理 | 官方只触发 `PostGameplayEffectExecute`，空着 | 在里面填"Damage→扣血、Healing→加血、死亡判定" | Lyra **填满了钩子** |
| 无敌/免疫 | 官方无这套 | `PreGameplayEffectExecute` 里查 `DamageImmunity`/`GodMode` Tag | Lyra **用钩子做规则** |
| 伤害公式 | 官方只给 Execution 框架，算啥你自己写 | 统一 `ULyraDamageExecution`：距离×材质×阵营 | Lyra **规范化公式** |
| 死亡通知 | 官方没有现成事件 | 自定义 `OnOutOfHealth` 委托 | Lyra **补了事件层** |
| 改属性能不能出负血/超上限 | 官方不自动管 | `ClampAttribute` + `PreAttributeChange` 强制钳制 | Lyra **加了保护层** |

**核心结论**：官方给你的是"**能动的部件**"（存属性的反射系统、叠效果的聚合器、改前后的钩子、Execution 框架）；Lyra 给你的是一套"**已经想好怎么用这些部件的游戏逻辑**"。**Lyra 不是重写了 GAS，是给官方 GAS 填了一份高质量的"标准用法"。**

---

## 三、Lyra 和官方最"反直觉"的一处差异：为什么要绕 `Damage` 中转？

这是最能体现"Lyra 在官方基础上加了层思考"的地方，值得单讲。

### 官方最朴素的思路（很多普通项目这么做）
直接在"扣血效果"里把伤害写成**直接改 Health**，`PostGameplayEffectExecute` 里 `SetHealth(Health - 10)`。简单直接。

### Lyra 的思路：先打在 `Damage` 上，再由 `Damage` 转成扣血
看第 15 篇的 `PostGameplayEffectExecute`：
```cpp
	// 效果把伤害值 Add 到 HealthSet.Damage
	// HealthSet 收到后在这里统一处理：
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));  // Damage → -Health
		SetDamage(0.0f);
	}
```

### 为什么要多这一层？三个理由

**① 让"改血之前的拦截"有统一的落脚点**
如果技能 A 直接 `-10 Health`、技能 B 直接 `-20 Health`，你想做"无敌时不掉血"，就得**改每一把枪/每个技能**。Lyra 统一先打到 `Damage`，那无敌检查只需要写在 `PreGameplayEffectExecute` 一处，管住 `Damage` 就管住所有伤害。源码（`LyraHealthSet.cpp` 第 76~99 行）：
```cpp
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
		{
			Data.EvaluatedData.Magnitude = 0.0f;   // 无敌 → 把要打的伤害归零
			return false;
		}
	}
```

**② 想加"额外伤害来源"好扩展**
以后要做"中毒每秒掉血、流血、灼烧"，它们全都往 `Damage` 上叠，`PostGameplayEffectExecute` 统一扣。不用每种状态各写一套扣血。

**③ 数值类型能区分"是收到伤害"还是"本来血量"**
官方属性系统里 `Health` 和 `Damage` 是**不同属性**。效果想表达"造成伤害"就 Add 到 `Damage`，想表达"直接扣上限外的血"才碰 `Health`。语义清晰。

> **大白话总结这一层**：官方让你"直接给血做减法"。Lyra 说"**你先写张'收到伤害'的条子（Damage），我统一在柜台（PostGameplayEffectExecute）核销**"——这样"无敌能不能核销、有没有流血条子混进来"，都在柜台一处把关。

---

## 四、实现场景（带代码）：做一个"敌人掉血 → 无敌 → 死亡"来对比两者写法

用同一个需求，分别看"官方最朴素写法"和"Lyra 写法"，你就彻底明白差异了。

**需求**：玩家 A 用武器打敌人 B，造成 50 伤害；B 若带"无敌"标记则不掉血；血掉到 0 触发死亡逻辑。

### 写法一：官方最朴素（很多普通 GAS 项目这么干）

直接在效果回调里改 Health + 自己判断无敌：
```cpp
// —— 敌人 A 被打时 ——
void UMyEnemy::ApplyDamage_Naive(float Damage)
{
	// 无敌检查得自己写在"每个会扣血的地方"
	if (HasMatchingGameplayTag(TAG_DamageImmunity))
	{
		return;                     // 无敌 → 啥都不干
	}
	// 血 = 血 - 伤害（如果还想 clamp，也得自己写）
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	// 死亡判定也得自己在这里写
	if (Health <= 0.f)
	{
		OnDeath();                  // 播死亡
	}
}
```
**问题**：如果还有"中毒、灼烧、敌人打你"多处扣血，每处都得复制"无敌检查 + clamp + 死亡判定"——规则散落、易漏。

### 写法二：Lyra（官方机制 + 在钩子里填规则）

你不用自己每处判断，效果机制帮你统一走钩子：

```cpp
// ① 武器命中时，只做一件事：往目标 Damage 属性上加"要打的伤害"
void UMyShootAbility::DealDamage(ULyraAbilitySystemComponent* TargetASC, float Amount)
{
	// （通常通过一个带 Execution 的 GameplayEffect 触发，此处示意"打伤害"意图）
	// Execution 里算出 Amount，最终 Add 到目标的 HealthSet.Damage
}

// ② 无敌检查：写在 PreGameplayEffectExecute（一处，管所有伤害）
bool ULyraHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity))
		{
			Data.EvaluatedData.Magnitude = 0.0f;   // 无敌 → 全部伤害归零
			return false;
		}
	}
	return true;
}

// ③ 扣血 + 死亡：写在 PostGameplayEffectExecute（一处，管所有伤害/治疗）
void ULyraHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), 0.f, GetMaxHealth())); // 扣血+clamp
		SetDamage(0.0f);
	}
	if ((GetHealth() <= 0.f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(/*...*/);   // 死亡信号：死亡系统监听这个
	}
}
```

**对比结论**：
- 朴素写法 = 规则**跟着每个扣血点走**，逻辑散落、重复。
- Lyra 写法 = 规则**集中在官方钩子里**（`Pre/PostGameplayEffectExecute`），"无敌""扣血""死亡"各归一处，任何伤害效果进来都自动走这套。**想给全游戏加"无敌"，贴一个 Tag 就行，不用改任何武器。**

> **这个对比就是 Lyra 相对官方的全部价值**：官方给钩子但不替你决定怎么用；Lyra 用钩子把"掉血/无敌/死亡"这些通用游戏规则**沉淀成标准实现**，你抄它的就行，不用每次从零发明。

---

## 五、一张图：官方给到哪、Lyra 接到哪

```
┌──────────────────── 官方 UE5.6 GAS 出厂自带 ────────────────────┐
│   FGameplayAttributeData（受管值）+ UPROPERTY 反射                │
│   ASC 聚合器：效果Modifier 怎么叠算一个属性的当前值                │
│   UAttributeSet 虚函数钩子：Pre/PostGameplayEffectExecute 等      │
│   GameplayEffectExecutionCalculation（Execution 框架）            │
└───────────────────────────────────────────────────────────────────┘
                              │  官方只提供"能动的部件"，里面是空的
                              ▼
┌──────────────────── Lyra 补上的"标准玩法层" ────────────────────┐
│   把空壳 UAttributeSet → 拆成 HealthSet / CombatSet（业务分层）    │
│   发明 meta 中转 Damage/Healing（让拦截/扩展有统一落脚点）        │
│   填满 Pre/PostGameplayEffectExecute（无敌/扣血/死亡判定）        │
│   统一伤害公式 ULyraDamageExecution（距离×材质×阵营）             │
│   自定义委托 OnOutOfHealth / OnHealthChanged（血条&死亡响应）     │
│   加 ClampAttribute（血不出负、不超上限）的保护                   │
└───────────────────────────────────────────────────────────────────┘
  Lyra 不碰官方引擎代码，只把官方的"钩子和框架"填满成一套通用游戏规则
```

---

## 六、总结一句话

> **官方 UE5.6 GAS = 提供"能存受管属性、能叠效果、能在改前后回调、能自定义 Execution"的一套引擎机制，但具体怎么用是空的。Lyra = 不改引擎，只在这套机制的回调钩子里，把"血量钳制、无敌免疫、伤害转扣血、死亡通知"这些通用玩法逻辑做成规范实现，并多包了一层"拆属性集 + Damage meta 中转 + 自定义事件"的业务封装。** 所以两者不是"两套系统"，而是"官方给机器，Lyra 给这套机器灌进了经过验证的游戏规则"。

---

## 七、下一步

- 深入官方 `FGameplayAttributeData` 与反射机制：ASC 到底怎么扫描你的属性、怎么序列化同步的。
- 深入官方 Execution 与聚合器：`AttemptCalculateCapturedAttributeMagnitude` 这个"捕获"底层怎么从聚合器取值。
- 想真正摸清官方底层，去 UE 引擎 `Source/Runtime/GameplayAbilities` 目录读 `UAttributeSet` / `UGameplayAbilitySystemComponent` 源码（以上机制描述可对照核实）。
