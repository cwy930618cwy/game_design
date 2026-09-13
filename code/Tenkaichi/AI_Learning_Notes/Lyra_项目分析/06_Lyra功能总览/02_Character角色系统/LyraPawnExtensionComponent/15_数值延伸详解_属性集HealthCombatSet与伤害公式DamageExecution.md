# 15 — 数值延伸详解：属性集（Health/Combat）和伤害公式（DamageExecution）

> **定位**：第 12 篇说的"第三招：数值延伸"。就是两件事——**①把角色的"数值"拆成几个属性集（AttributeSet）来装；②把"怎么算伤害/治疗"抽成一个专门的执行器（Execution）。**
>
> 这篇先把"属性集 + Execution 是什么、为什么官方不直接给"用大白话讲清，**再每个概念紧跟源码**，最后用一个完整的"打一枪掉血"流程把两者串起来。

---

## 〇、先搞懂：伤害是怎么"算出来又落下去"的？一句话总览

在 GAS 里，**你不会直接在代码里写 `血量 -= 伤害`**。而是走一条链路：

```
某个技能命中目标 → 生成一个"伤害效果(GameplayEffect)" 
   → 效果触发一个"计算器(Execution)"算出伤害 = 多少
   → 结果打在目标的"属性集(AttributeSet)"上
   → 属性集再把"受到的伤害"换算成"血量减少"
   → 血量到 0 → 广播"死亡事件"
```

下面每一环都拆开讲 + 贴源码。你会看到一个枪战游戏的"掉血-死亡"在 Lyra 里被拆得清清楚楚。

---

## 一、官方给了什么：属性集 `UAttributeSet` 只是个"空桶"

### 概念
GAS 里角色的数值（血、蓝、攻击力）不是散乱的变量，而是**集中在"属性集"类里管理**。官方 `UAttributeSet` 只是给你打个底，**里面一个数值都没有**——具体存哪些数值，得你自己写子类去加。

### 那为什么不能直接 `int32 Health` 一个普通变量？
因为 Lyra 想让血量能被"效果、公式、网络、事件"全套接管。看官方用什么类型存（`LyraHealthSet.h` 第 74~75 行）：

```cpp
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Lyra|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;
```

`FGameplayAttributeData` 不是普通 float，它自带：
- `ReplicatedUsing` → **能网络同步**（血条在所有人端一致）
- 能被 `UGameplayEffect` 修改、能被"属性捕获"用于公式计算
- 数值变化会触发回调（下面讲）

> **大白话**：`Health` 不是"一个数字"，而是"一个能被效果改、能网络同步、能触发事件通知别人"的受管数值。这就是属性集存在的意义。

---

## 二、Lyra 把官方"空桶"拆成几个业务化子类（`Attributes/` 目录）

官方就一个空壳 `UAttributeSet`，Lyra 在它下面拆了两个有意义的子类（还都用宏 `ATTRIBUTE_ACCESSORS` 自动生成读写函数）：

### 子类 1：`ULyraCombatSet` —— 装"打出去之前"的攻击数值
源码（`LyraCombatSet.h` 第 29~48 行）：
```cpp
class ULyraCombatSet : public ULyraAttributeSet
{
public:
	ATTRIBUTE_ACCESSORS(ULyraCombatSet, BaseDamage);   // 基础伤害
	ATTRIBUTE_ACCESSORS(ULyraCombatSet, BaseHeal);     // 基础治疗
private:
	UPROPERTY(...) FGameplayAttributeData BaseDamage;
	UPROPERTY(...) FGameplayAttributeData BaseHeal;
};
```

> **它管"你这枪理论打多少 / 治疗能回多少"**，是"源头数据"，被后面的伤害公式读取。

### 子类 2：`ULyraHealthSet` —— 装"被打/被奶之后"的血量结果
源码（`LyraHealthSet.h` 第 40~52 行）：
```cpp
	ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health);     // 当前血量
	ATTRIBUTE_ACCESSORS(ULyraHealthSet, MaxHealth);  // 最大血量
	ATTRIBUTE_ACCESSORS(ULyraHealthSet, Healing);    // 收到的治疗（meta，临时中转）
	ATTRIBUTE_ACCESSORS(ULyraHealthSet, Damage);     // 收到的伤害（meta，临时中转）

	// 血量变化时广播（被人打了/被奶了，UI 血条就是听这个）
	mutable FLyraAttributeEvent OnHealthChanged;
	// 最大血量变化时广播（穿加血上限装备时）
	mutable FLyraAttributeEvent OnMaxHealthChanged;
	// 血量到 0 时广播（死亡判定！动画/掉落物/计分都听这个）
	mutable FLyraAttributeEvent OnOutOfHealth;
```

### 为什么分两个 Set？（而不是都塞一个）
> **分工不同**：`CombatSet` 是"**攻击方**带的伤害/治疗值"（源头），`HealthSet` 是"**挨打方**身上的血量"（结果）。技能打你时，从攻击方 `CombatSet` 抓 `BaseDamage`，算完打在挨打方 `HealthSet` 的 `Damage` 上。**一个管输出、一个管承伤，解耦。**（实际用起来 `CombatSet` 常挂在施法者身上、`HealthSet` 挂在目标身上。）

> **教学场景**：你要做"武器 + 装备"系统。武器的攻击力就进 `CombatSet.BaseDamage`，装备加血上限就改 `HealthSet.MaxHealth`。因为都是受管数值，效果/公式/同步自动接手，你不用手写血量同步。

---

## 三、重点：`Health` 有哪些"魔法属性"？先看它俩的构造函数

源码（`LyraHealthSet.cpp` 第 21~28 行）：
```cpp
ULyraHealthSet::ULyraHealthSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	bOutOfHealth = false;
	HealthBeforeAttributeChange = 0.0f;
	MaxHealthBeforeAttributeChange = 0.0f;
}
```

**出生时** `Health=100`、`MaxHealth=100`。构造函数就把初始值定好。

### 血量的两条"规矩"：不能小于 0、不能超过 MaxHealth（`ClampAttribute`）
源码（`LyraHealthSet.cpp` 第 221~233 行）：
```cpp
void ULyraHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());  // 血量夹在 [0, MaxHealth]
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);   // 最大血量最低也 >= 1
	}
}
```
而这个 `ClampAttribute` 被 `PreAttributeChange` / `PreAttributeBaseChange` 调用（第 185~197 行）——**每次血量要被改动前，先卡上限下限**，防止出现负数血或溢出。

> **大白话**：你掉的"伤害"不会让你血变负，加的"血"不会超过血上限——因为改之前先 `Clamp` 一次。

---

## 四、最关键的转换：`Damage`（收到伤害）怎么变成"扣血"？—— `PostGameplayEffectExecute`

这里要解释 HealthSet 里两个怪字段：`Damage` 和 `Healing`。注释说它们是 **Meta Attribute（元属性）**。看字段注释（`LyraHealthSet.h` 第 92~98 行）：
```cpp
	// Incoming healing. This is mapped directly to +Health
	FGameplayAttributeData Healing;   // 收到的治疗 → 转成 +血
	// Incoming damage. This is mapped directly to -Health
	FGameplayAttributeData Damage;    // 收到的伤害 → 转成 -血
```

**为什么不能直接改 Health？** 因为"伤害减免、无敌、作弊 GodMode"这些拦截逻辑，都希望在"改血之前"插一脚。所以先打到 `Damage` 上，处理完再换算成 `-Health`。看核心函数 `PostGameplayEffectExecute`（`LyraHealthSet.cpp` 第 128~156 行）：

```cpp
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// （前面省略：广播伤害消息给其他系统）

		// Convert into -Health and then clamp
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
		SetDamage(0.0f);   // 处理完把"暂存的伤害"清零
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// Convert into +Health and then clamp
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
		SetHealing(0.0f);
	}
```

**大白话**：
- 效果带来的"伤害值"落到 `Damage` 属性。
- `PostGameplayEffectExecute` 一看改的是 `Damage` → **执行 `Health = Health - Damage`**，再 `Clamp` 防越界，最后把 `Damage` 归零（它是临时中转站）。
- 治疗同理：`Health = Health + Healing`。

### 掉血前还能拦截！—— `PreGameplayEffectExecute`
为什么非要绕 `Damage`？因为能在扣血前做"无敌/免疫/作弊检查"。看（第 68~106 行）：
```cpp
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			// 目标带"伤害免疫"标签，且不是自杀伤害 → 伤害归零
			if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
			{
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
			// 开了 GodMode 作弊 → 伤害也归零
			if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) && !bIsDamageFromSelfDestruct)
			{
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
		}
	}
```

> **这就是"无敌/锁血"的实现**：你在目标身上贴个 `DamageImmunity` Tag，任何伤害效果在 `PreGameplayEffectExecute` 里被拦下归零，血一滴不掉。**比在每把枪里写 if 优雅多了**——全局一个 Tag 就搞定"无敌"。

### 血到 0 → 广播"死亡"（游戏逻辑响应的地方）
在 `PostGameplayEffectExecute` 末尾（第 171~182 行）：
```cpp
	// 如果血真的变了 → 广播 OnHealthChanged（血条 UI 就听这个）
	if (GetHealth() != HealthBeforeAttributeChange)
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());

	// 血 <= 0 且之前没死 → 广播 OnOutOfHealth（死亡！）
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());

	bOutOfHealth = (GetHealth() <= 0.0f);
```

> **教学场景**：你要做"死亡通知"。别自己每处扣血都发事件，只要在角色身上监听 `HealthSet.OnOutOfHealth`，它一广播就知道"死了"，然后播死亡动画、掉装备、计击杀分、复活。**一处扣血，全游戏都听同一个死亡信号。**

---

## 五、那"伤害 = 多少"谁算的？—— Execution（`ULyraDamageExecution`）

前面说的是"伤害落下来之后怎么变扣血"。现在反着问：**"这一枪该打多少血"这个数值，是谁算的？**

答案：**一个专门的"伤害执行器" `ULyraDamageExecution`**。它不是自己算完直接扣血，而是算出"伤害值"，再塞进 `HealthSet.Damage`，走上面第四节的扣血流程。**分工：Execution 算多少，HealthSet 负责扣。**

### Execution 在 `GameplayEffect` 里的角色

官方允许你在一个 `UGameplayEffect` 里塞一个"自定义执行器"（`GameplayEffectExecutionCalculation` 的子类），当这个效果生效时，执行器跑一遍、算出要改哪些属性。Lyra 就是给伤害/治疗各写了一个。

### 源码拆解：`ULyraDamageExecution::Execute_Implementation`（`LyraDamageExecution.cpp` 第 36~139 行）

#### ① 声明我要"捕获/读取"哪些属性（构造函数里注册，第 31~34 行）
```cpp
ULyraDamageExecution::ULyraDamageExecution()
{
	// 我要读"攻击方(Source)"的 BaseDamage 属性
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}
```
`BaseDamageDef` 在静态结构体里定义（第 20 行）：从 `ULyraCombatSet` 拿 `BaseDamage`，捕获来源是 `Source`（攻击方）。
```cpp
BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(
	ULyraCombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
```

> **概念：属性捕获（Capture）**。伤害公式要读"攻击方的基础伤害"，得先"声明我要捕获它"，公式执行时引擎把这个值抓给你。`Source` = 谁造成这个效果（开枪的人）。

#### ② 读取攻击方基础伤害（第 50~51 行）
```cpp
	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);
```
从"开枪的人"身上捕获 `BaseDamage` 基础伤害值。

#### ③ 算"命中信息"：打中谁、在哪、距离多远（第 53~114 行，节选）
```cpp
	const FHitResult* HitActorResult = TypedContext->GetHitResult();   // 命中结果
	// ... 从 HitResult 取出 HitActor、ImpactLocation 打击点 ...
	// 没命中 actor 就退回用目标化身
	// 计算 Distance（伤害来源点到打击点的距离，用于"距离衰减"）
```
> 这就是为什么"距离越远伤害越低"能实现——Execution 里算了 `Distance`，后面用它做距离衰减。

#### ④ 阵营/自伤规则（第 90~98 行）
```cpp
	float DamageInteractionAllowedMultiplier = 0.0f;
	if (HitActor)
	{
		ULyraTeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
		DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
	}
```
> 查队伍子系统：**打中队友 → 乘 0（不造成伤害）；打中敌人 → 乘 1**。这就是"友军伤害保护"的统一实现。

#### ⑤ 伤害衰减：材质衰减 × 距离衰减（第 116~131 行，节选）
```cpp
	float PhysicalMaterialAttenuation = 1.0f;   // 材质衰减（打到钢板 vs 打到肉体）
	float DistanceAttenuation = 1.0f;           // 距离衰减（越远越弱）
	if (const ILyraAbilitySourceInterface* AbilitySource = TypedContext->GetAbilitySource())
	{
		if (PhysMat)  PhysicalMaterialAttenuation = AbilitySource->GetPhysicalMaterialAttenuation(PhysMat, ...);
		DistanceAttenuation = AbilitySource->GetDistanceAttenuation(Distance, ...);
	}
	DistanceAttenuation = FMath::Max(DistanceAttenuation, 0.0f);

	// ★ 最终伤害公式 ★
	const float DamageDone = FMath::Max(BaseDamage * DistanceAttenuation * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);
```

**这就是伤害公式本体**，就一行：
```
最终伤害 = BaseDamage（基础伤害）
          × DistanceAttenuation（距离衰减）
          × PhysicalMaterialAttenuation（材质衰减）
          × DamageInteractionAllowedMultiplier（阵营开关：0 或 1）
```

#### ⑥ 把算出的伤害"输出"到目标（第 133~137 行）
```cpp
	if (DamageDone > 0.0f)
	{
		// Apply a damage modifier, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			ULyraHealthSet::GetDamageAttribute(),   // 输出到目标的 Damage 属性
			EGameplayModOp::Additive, DamageDone)); // 加 DamageDone
	}
```
> Execution 不直接扣血，它把伤害 **Add 到目标的 `HealthSet.Damage`**。然后交给第四节的 `PostGameplayEffectExecute` 把 `Damage` 换算成 `-Health`。

---

## 六、把整个"挨一枪"流程完整串起来（一图秒懂）

```
你(攻击方)朝敌人开一枪 → 技能命中 → 生成一个"伤害 GameplayEffect"
                                          │ 该效果带 ULyraDamageExecution 执行器
                                          ▼
       执行器 Execute_Implementation：
        ① 捕获攻击方的 CombatSet.BaseDamage      （这枪基础多少伤害）
        ② 从 HitResult 拿到：打中谁 / 距离多远
        ③ 查队伍：打队友？→ 伤害乘0  打敌人 → 乘1
        ④ 套公式：最终伤害 = BaseDamage × 距离衰减 × 材质衰减 × 阵营开关
        ⑤ 输出：最终伤害 Add 到【敌人】HealthSet.Damage
                                          │
                                          ▼
       敌人 HealthSet：
        PreGameplayEffectExecute：查无敌Tag？GodMode作弊？→ 有就伤害归零
        PostGameplayEffectExecute：Damage → Health -= Damage（clamp 防负血）
                                   若 Health<=0 → 广播 OnOutOfHealth（死亡！）
                                          │
                                          ▼
       血条 UI 听 OnHealthChanged 刷新    死亡系统听 OnOutOfHealth 播死亡
```

**职责划分（记这个就懂整个体系）**：
- `ULyraCombatSet`：攻击方携带的**源头数值**（BaseDamage/BaseHeal）。
- `ULyraDamageExecution`：**伤害计算器**——读源头、套公式（距离/材质/阵营）、算出结果。
- `ULyraHealthSet`：挨打方身上的**血量 + 兜底转换**——把 Execution 算的 Damage 变成 -Health，并管无敌、作弊、扣血、死亡事件。

---

## 七、总结：Lyra"数值延伸"到底解决了什么

| 官方给的 | Lyra 的做法 | 好处 |
|---|---|---|
| 空壳 `UAttributeSet` | 拆成 `CombatSet`（输出）+ `HealthSet`（承伤） | 输出/承伤解耦，各管各的 |
| 伤害公式写在"任意一个 GameplayEffect"里，每把武器一套 | 抽成统一 `ULyraDamageExecution` | **所有武器共用同一公式**，好维护、好调参、好 debug |
| 扣血散落各处 | 统一走 `Damage → Health` + `Pre/PostGameplayEffectExecute` | 无敌/作弊/死亡判定集中一处 |
| 死亡判定各处自己做 | `OnOutOfHealth` 委托广播 | 全游戏只监听一个死亡信号 |

> **一句话**：Lyra 不满足于"GAS 能存数值、能改数值"，而是把数值**按业务拆成几个属性集**、把**伤害/治疗公式抽成标准执行器**，让"数值从哪来、公式怎么算、结果怎么落到血条、死了怎么通知"都有一处明确的归属，而不是散落在每个技能里各写一套。

---

## 八、下一步

- `ULyraCombatSet` 的 `BaseDamage` 是谁填的？——通常武器/装备的 GameplayEffect 或 AbilitySource 会往攻击方 `CombatSet` 里写，追一下"武器伤害"来源。
- `ULyraHealExecution`：和治疗是一套对称逻辑，可对照看。
- 血条 UI 到底谁在听 `OnHealthChanged`？——Lyra 有专门的 `ULyraAttributeSet`→UI 绑定的机制（AttributeSet 与 HUD 的桥）。
- `GameplayEffect` 上怎么"挂"一个 `ULyraDamageExecution`？——去 `UGameplayEffect` 资产里看 `Execution` 数组配置。
