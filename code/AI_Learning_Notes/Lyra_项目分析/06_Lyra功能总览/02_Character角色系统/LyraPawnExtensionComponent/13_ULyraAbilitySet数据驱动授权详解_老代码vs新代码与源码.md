# 13 — `ULyraAbilitySet` 数据驱动授权详解：老代码 vs 新代码 + 原理源码

> **定位**：上一篇提到 Lyra 的第一招是"数据驱动（AbilitySet）"。这篇把它彻底讲透：**在不用 AbilitySet 的"老项目/官方裸写法"里，授权技能要怎么写一堆代码；Lyra 用 AbilitySet 后变成了什么样；背后的 `GiveToAbilitySystem` 源码到底在干嘛。**
>
> 目标：你以后既看得懂官方老写法，也看得懂 Lyra 的新写法，还能自己抄源码。

---

## 〇、一句话提纲

- **老代码**：你亲自拿 `FGameplayAbilitySpec` 一个一个 new、逐个调 `ASC->GiveAbility()`，还要自己写"输入 Tag 怎么绑"。
- **新代码（AbilitySet）**：把"要授权哪些"填进一张**数据资产**，调用方只写一句 `GiveToAbilitySystem(ASC, Handles)`。
- **本质**：AbilitySet 没有发明新机制，它只是把老代码里"一个个授权 + 记 Handle"的**重复样板逻辑，封装成了可以配置成资产的一次性函数**。

---

## 一、先说底层共用的机制：官方 `GiveAbility` 到底做了什么

不管是老代码还是新代码，最终都调**官方 ASC 的 `GiveAbility`**。它是地基，先讲透它。

### 官方 `GiveAbility` 的作用（源码行为）

```
ASC->GiveAbility(FGameplayAbilitySpec Spec)
   │
   ├─ 用 Spec（技能CDO + 等级 + 自定义Tag）构造一个"待激活条目"
   ├─ 把它插进 ASC 内部的技能台账：ActivatableAbilities.Items   ← 技能从此被"认识"
   ├─ 分配并返回一个 FGameplayAbilitySpecHandle（技能的唯一身份证）
   └─ 之后 ASC 才能 TryActivateAbility 这个技能
```

**关键点**：`GiveAbility` 前，技能只是一份"CDO 蓝图/CDO 类"，**没进 ASC 的台账 = ASC 不认它**。`GiveAbility` 就是"给技能上户口"的动作，返回的 **Handle** 就是户口编号，以后想取消/移除就凭这个 Handle 找它。

---

## 二、老代码长什么样？（官方裸写法 / 大多数普通项目的写法）

### 问题场景
要做一个"职业选择系统"：给玩家的 ASC 授予——3 个技能、1 个加攻速的常驻效果、1 套属性。这是官方场景最常见的需求。

### 老代码（你自己写样板）

```cpp
// 伪代码演示"没有 AbilitySet 时，你得手工写这些"
void UMyCharacterAbilityAdder::GiveKnightAbilities(ULyraAbilitySystemComponent* ASC)
{
	// —— 1. 授予技能：每个技能都要 new 一个 Spec，逐个 GiveAbility ——
	FGameplayAbilitySpec Spec1 = FGameplayAbilitySpec(
		UMySwordSwing::StaticClass()->GetDefaultObject<UGameplayAbility>(), /*等级*/1);
	Spec1.DynamicAbilityTags.AddTag(FGameplayTag::RequestGameplayTag("InputTag.Skill1")); // 手动绑输入Tag
	FGameplayAbilitySpecHandle H1 = ASC->GiveAbility(Spec1);

	FGameplayAbilitySpec Spec2 = ...; // 又重复一遍
	FGameplayAbilitySpecHandle H2 = ASC->GiveAbility(Spec2);

	FGameplayAbilitySpec Spec3 = ...; // 再重复一遍
	FGameplayAbilitySpecHandle H3 = ASC->GiveAbility(Spec3);

	// —— 2. 授予效果：要拿到 CDO 再 Apply ——
	const UGameplayEffect* AttackSpeedGE =
		UMyAttackSpeedGE::StaticClass()->GetDefaultObject<UGameplayEffect>();
	FActiveGameplayEffectHandle EH = ASC->ApplyGameplayEffectToSelf(AttackSpeedGE, 1.0f, ASC->MakeEffectContext());

	// —— 3. 授予属性集：new 出来再 Add ——
	UAttributeSet* MySet = NewObject<UMyCombatSet>(ASC->GetOwner());
	ASC->AddAttributeSetSubobject(MySet);
}
```

### 老代码的痛点（Lyra 想解决的核心问题）

| 痛点 | 说明 |
|---|---|
| **样板代码爆炸** | 每多一个技能/效果，就复制粘贴一坨 `GiveAbility`/`Apply...` |
| **技能和逻辑耦合在 C++ 里** | "骑士带哪几个技能"写死在函数里，想换职业得改代码重新编译 |
| **替换角色痛苦** | 想"卸下旧职业、换上新职业"得把所有 Handle 手动 Clear——漏一个就出 bug |
| **输入 Tag 怎么绑，每个项目各自发明** | 没有统一约定，新手难上手 |

> **教学场景**：想象你在做格斗游戏的角色选择界面。玩家从"火、水、风"三系选一个，每系各带 5 个技能 + 被动。用老代码写，你至少写三份几乎一样的 `GiveXXAbilities()` 函数，每份里重复 5+ 次 `GiveAbility`。想加一个技能，得改动 C++ 重新编译。这套流程又啰嗦又易错——**Lyra 就是来消灭这种重复的。**

---

## 三、新代码长什么样？（Lyra 的 AbilitySet 写法）

### 思路转变：把"给谁、给什么、给几级"从代码里"搬出去"，变成一张资产

**第一步：策划/程序在编辑器里，用数据资产 `ULyraAbilitySet` 做一份"骑士能力包"**
它只是填三个数组（对应 `ULyraAbilitySet.h` 里的三个字段）：

```
ULyraAbilitySet 资产（Knight_AbilitySet）
┌─ GrantedGameplayAbilities（技能清单）
│    能力: UMySwordSwing    等级: 1    InputTag: InputTag.Skill1
│    能力: UMyShieldBlock    等级: 1    InputTag: InputTag.Skill2
│    能力: UMyPassiveHalo    等级: 1    (空)
├─ GrantedGameplayEffects（效果清单）
│    UMyAttackSpeedGE   等级: 1.0
└─ GrantedAttributes（属性集清单）
     UMyCombatSet
```

**第二步：运行时，调用方只写一行**
```cpp
void UMyAbilityGrantComponent::GrantKnight(ULyraAbilitySystemComponent* ASC,
                                            ULyraAbilitySet* KnightSet)
{
	FLyraAbilitySet_GrantedHandles GrantedHandles;        // 装"给了哪些"的收据
	KnightSet->GiveToAbilitySystem(ASC, &GrantedHandles); // 一句话全给上
	// 以后想换职业/卸掉：GrantedHandles.TakeFromAbilitySystem(ASC);
}
```

### 新代码的收益（对比老代码）

| 收益 | 说明 |
|---|---|
| **一行搞定一堆授权** | 技能/效果/属性一股脑 `GiveToAbilitySystem` |
| **换职业 = 换资产** | "骑士"变"法师"只要换一个 `ULyraAbilitySet` 引用，不用改 C++ |
| **输入 Tag 统一约定** | InputTag 写死在资产里，`GiveAbility` 自动把 Tag 挂上技能（给输入系统用） |
| **拿到统一"收据"** | `FLyraAbilitySet_GrantedHandles` 记下所有 Handle，`TakeFromAbilitySystem` 一键卸载 |
| **有合法性检查** | 只在权威端执行、无效资产会报 log（见源码） |

---

## 四、新代码底层源码：`GiveToAbilitySystem` 逐段拆解（重点）

现在看 Lyra 用**多少行代码**实现了"老代码那一大坨"。完整实现在 `LyraAbilitySet.cpp` 第 73~147 行。逐段讲：

### 第 0 段：权威端检查（第 75~81 行）
```cpp
void ULyraAbilitySet::GiveToAbilitySystem(ULyraAbilitySystemComponent* LyraASC,
	FLyraAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(LyraASC);
	if (!LyraASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}
```
> **原理**：授权/撤销必须在**权威端（服务器）**做，否则客户端也能乱给技能 → 作弊/不同步。这和老代码里"无脑调用"的区别：Lyra 把"合法性检查"也包进了封装。

### 第 1 段：授予属性集（第 83~101 行）
```cpp
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FLyraAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];
		if (!IsValid(SetToGrant.AttributeSet))          // 资产没配好 → 报警并跳过
		{
			UE_LOG(LogLyraAbilitySystem, Error, TEXT("...is not valid"));
			continue;
		}
		UAttributeSet* NewSet = NewObject<UAttributeSet>(LyraASC->GetOwner(), SetToGrant.AttributeSet);
		LyraASC->AddAttributeSetSubobject(NewSet);      // 关键：把属性集挂到 ASC 上
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet); // 记进收据
		}
	}
```
> **逐词讲**：
> - `NewObject<UAttributeSet>(Owner, 类)`：**new 一个属性集实例**，挂在 ASC 的 Owner（角色）下。用"资产里配的类"当模板，而不是写死某类。
> - `AddAttributeSetSubobject`：告诉 ASC"这个属性集归你管了"，之后效果才能改它上面的属性。
> - `OutGrantedHandles->AddAttributeSet`：**记一笔**，方便以后撤销。

### 第 2 段：授予技能（第 103~126 行）—— 对应老代码里"最啰嗦"的部分
```cpp
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FLyraAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability)) { /* 无效就报错跳过 */ continue; }

		// 关键1：从资产里的"技能类"拿 CDO（class default object，类默认对象）
		ULyraGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<ULyraGameplayAbility>();

		// 关键2：构造官方 Spec（CDO + 等级），这才是 GiveAbility 要的东西
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;                 // 记录"这技能是谁给的/来源"
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag); // 把输入Tag挂上技能

		// 关键3：核心——调官方 GiveAbility 上户口，拿 Handle
		const FGameplayAbilitySpecHandle AbilitySpecHandle = LyraASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle); // 记收据
		}
	}
```
> **这段就是"老代码里手工重复 N 次"的浓缩**：
> - `GetDefaultObject`：从资产配的 `Ability` 类拿到 CDO（技能模板）。为什么用 CDO？因为技能是**类**，授权时不是 new 实例，而是拿"该类的 CDO"做 Spec 的蓝图模板。
> - `FGameplayAbilitySpec`：官方结构体，装着"CDO + 等级 + 来源 + 动态Tag"。GiveAbility 只认它。
> - `GetDynamicSpecSourceTags().AddTag(InputTag)`：**把资产里配的输入 Tag 动态挂到这份 Spec 上**。后面 `ProcessAbilityInput` 就是靠这个 Tag 找到对应技能的（之前笔记讲过）。
> - `GiveAbility` → 拿到 `Handle`（户口编号）→ 记进收据。

### 第 3 段：授予效果（第 128~146 行）
```cpp
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FLyraAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
		if (!IsValid(EffectToGrant.GameplayEffect)) { /* 无效跳过 */ continue; }

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle =
			LyraASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, LyraASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle); // 记收据
		}
	}
}
```
> 和老代码的"ApplyGameplayEffectToSelf"写法几乎一样，只是：**CDO 从资产里取、等级用资产里配的、Handle 记进收据**。这就是"把重复样板搬进封装"最直白的例子。

### 配对的"撤销"：`TakeFromAbilitySystem`（第 32~66 行）
```cpp
	if (!LyraASC->IsOwnerActorAuthoritative()) { return; }  // 同样要权威端

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
		LyraASC->ClearAbility(Handle);                       // 按户口编号 逐个销户（技能）

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
		LyraASC->RemoveActiveGameplayEffect(Handle);         // 移除效果

	for (UAttributeSet* Set : GrantedAttributeSets)
		LyraASC->RemoveSpawnedAttribute(Set);                // 卸掉属性集

	// 清空收据
	AbilitySpecHandles.Reset(); GameplayEffectHandles.Reset(); GrantedAttributeSets.Reset();
```
> **为什么收据（GrantedHandles）那么重要？** 因为"给出去"的是一堆 Handle，想"收回来"必须记得每个 Handle。`FLyraAbilitySet_GrantedHandles` 就是**一份"我给了哪些"的清单**，`TakeFromAbilitySystem` 顺着清单挨个 `ClearAbility` / `RemoveActiveGameplayEffect` / `RemoveSpawnedAttribute`。**没有这份清单，你就只能自己拿 3 个数组管 3 类 handle，很容易漏。**

---

## 五、新代码 vs 老代码：一张终极对比表

| 维度 | 老代码（官方裸写） | 新代码（ULyraAbilitySet） |
|---|---|---|
| 谁决定"给什么" | 写死在 C++ 函数里 | 填在**数据资产**里 |
| 授予技能的写法 | 手动 new Spec + 逐个 GiveAbility | 资产里列清单，运行时一句话 |
| 输入 Tag 绑定 | 每次手写 `AddTag` | 资产里的 InputTag 字段自动挂 |
| 撤销 | 自己存一堆 Handle 挨个 Clear | `FLyraAbilitySet_GrantedHandles` + `TakeFromAbilitySystem` |
| 换职业/换配置 | 改代码、重新编译 | **换一张资产** |
| 权威端检查 | 自己记得加 or 忘 | 封装里自动 `IsOwnerActorAuthoritative` |
| 底层最终调用 | `GiveAbility` / `ApplyGameplayEffectToSelf` / `AddAttributeSetSubobject` | **完全一样**（没发明新机制） |

> **最关键的一句**：新代码并没有绕过或重写官方 `GiveAbility`。它把老代码里"拿 CDO → 建 Spec → 挂 Tag → GiveAbility → 存 Handle"这套**每个项目都重复的样板，收拢成一份可配置成资产的数据结构 + 一个函数**。**改变的是"组织授权的方式"，不变的是"授权最终调用的引擎 API"。**

---

## 六、总结一句话

> `ULyraAbilitySet` 是**把"给一个角色/ASC 授权一堆技能/效果/属性"从"一堆 C++ 样板代码"升级成"一张可配置的数据资产 + 一次封装调用"**的产物。它的 `GiveToAbilitySystem` 内部只是在权威端安全地循环调用官方 `GiveAbility`/`ApplyGameplayEffectToSelf`/`AddAttributeSetSubobject`，并把所有返回的 Handle 记进 `FLyraAbilitySet_GrantedHandles`，以便随时用 `TakeFromAbilitySystem` 一键回收。

---

## 七、下一步

- 谁在实际调用 `GiveToAbilitySystem`？——通常由 `ULyraPawnExtensionComponent` / 能力授予组件在角色初始化时触发，追一下调用链。
- 输入 Tag 挂上去之后，`ProcessAbilityInput` 怎么靠它把按键翻译成技能激活（回看第 10 篇）。
- `GetDynamicSpecSourceTags()` 和普通 AbilityTags 的区别——为什么输入 Tag 要挂成"动态来源 Tag"。
