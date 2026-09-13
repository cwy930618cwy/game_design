# 01 - GameplayAbilities（GAS 技能系统）

> 路径：`c:\Program Files\Epic Games\UE_5.6\Engine\Plugins\Runtime\GameplayAbilities\`
> 规模：**621 文件**（437 .h + 161 .cpp）—— Runtime 里最大的单个游戏逻辑插件
> Lyra 使用度：⭐⭐⭐ **核心中的核心**

---

## 一、这是什么？

**GAS (Gameplay Ability System)** 是 UE 官方的**技能/属性/效果框架**，专门用于：
- RPG 技能系统
- MOBA 英雄技能
- 射击游戏武器/道具
- 任何需要"属性 + 技能 + Buff/Debuff"的游戏

**Lyra 用它实现了**：跳跃、冲刺、近战、远程射击、生命值、伤害计算……几乎所有战斗相关逻辑。

---

## 二、四大核心概念（必须记住）

> ⚠️ 这一节是 GAS 的**地基**，务必理解透。如果第一次看晕，多看两遍比喻，再回来对照代码。

### 🎮 先用一个比喻建立全局印象

把 GAS 想象成医院的**"病人状态管理系统"**：

| GAS 概念 | 比喻 | 本质 |
|---------|------|------|
| **AttributeSet（属性集）** | 病人的**体检指标表** | 存数值：血量=100、蓝量=50、攻击=20 |
| **GE（GameplayEffect）** | 一剂**药 / 治疗操作** | 改变数值：吃补血药 → 血量+30 |
| **GA（GameplayAbility）** | 一个**技能动作** | 可执行的行为：放火球、跳跃、射击 |
| **ASC（AbilitySystemComponent）** | 医院的**中央监控台** | 管理一切：记账、发药、触发技能 |

**最关键的一句话**：前三个（AttributeSet / GE / GA）都是**"数据 / 定义"**，它们自己不会动；只有 **ASC 是唯一的"执行者 / 管理器"**，所有操作都要通过它发起。

```
        ┌─────────────────────────────────┐
        │           ASC（监控台）           │  ← 唯一执行者
        │   管账 / 发药 / 触发技能          │
        └───────┬───────────┬──────────────┘
                │           │
        读取/修改│    施加   │   激活
                ▼           ▼
   ┌─────────────────┐  ┌─────────────────┐
   │  AttributeSet   │  │      GE         │  ← 被管理的"数据"
   │ （体检指标表）    │  │   （药方）       │
   └─────────────────┘  └────────┬────────┘
                                 │ 由谁执行？
                                 ▼
                        ┌─────────────────┐
                        │      GA         │  ← 技能动作
                        │   （释放火球）    │
                        └─────────────────┘
```

---

### 2.1 AttributeSet（属性集）— 数值容器

**是什么**：一堆数值的集合，记录"这个角色有哪些数字属性"。

**Lyra 里的例子**：
```
ULyraHealthSet（生命值属性集）：
  ├── Health（当前血量）      = 100.0
  ├── MaxHealth（最大血量）   = 100.0
  └── DeathState（死亡状态）  = false

ULyraCombatSet（战斗属性集）：
  ├── AttackPower（攻击力）   = 20.0
  ├── Defense（防御力）       = 10.0
  └── CritChance（暴击率）    = 0.1
```

**关键点**：
- 它**只存数据**，不做任何逻辑（不计算、不判断）
- 每个属性叫 `FGameplayAttributeData`，包含：基础值(Base)、当前值(Current)、最大值(Max)
- 支持**网络自动复制**（多人游戏里血量自动同步给所有客户端）
- 数值变化时会**触发回调**（血量变了 → 通知 UI 刷新血条）

```cpp
// Lyra 的两个属性集
ULyraHealthSet     // 生命值、最大生命值、死亡状态
ULyraCombatSet     // 攻击力、防御力、暴击率
```

> 💡 **一句话记忆**：AttributeSet = 一张表格，记录角色的所有数字属性。

---

### 2.2 GE（GameplayEffect）— 修改数值的手段

**是什么**：对属性做修改的"操作指令"。**每次想改一个属性值，都必须通过 GE**，不能直接改。

**为什么不直接改数值？** 因为 GE 提供了**统一、安全、可追踪**的修改方式：
- **谁改的？**（来源追踪，方便调试）
- **改了多久？**（持续 / 永久）
- **能不能叠加？**（多次中毒是否叠在一起）

**三种类型**：

| 类型 | 含义 | 例子 |
|------|------|------|
| **Instantaneous（瞬时）** | 立即生效，改完就消失 | 中一枪 -30 血 |
| **Duration（持续）** | 持续一段时间 | 中毒 10 秒，每秒 -5 血 |
| **Infinite（无限）** | 一直存在直到手动移除 | 穿上盔甲 +20 防御力 |

**Lyra 里的例子**：
```
GE_LyraDamage（伤害）    → 瞬时型，Health -= 30
GE_LyraHeal（治疗）      → 瞬时型，Health += 50
GE_LyraCooldown（冷却）  → 持续型，持续 5 秒不能再用技能
GE_LyraCost（消耗）      → 瞬时型，Mana -= 20
```

**关键点**：
- GE **不能自己运行**，必须由 ASC 来施加：`ASC->ApplyGameplayEffect()`
- GE 可以附带 **ExecutionCalculation**（伤害公式，如"攻击力 × 暴击率"，见第四节）
- GE 可以附带 **GameplayTag**（标记这个效果是什么类型，用于筛选/免疫）

> 💡 **一句话记忆**：GE = 一剂药方，规定了"改哪个属性、改多少、持续多久"。

---

### 2.3 GA（GameplayAbility）— 可执行的技能

**是什么**：一个"技能 / 能力"，玩家激活它来执行某个动作。

**Lyra 里的例子**：
```cpp
ULyraGameplayAbility_Jump          // 跳跃
ULyraGameplayAbility_Dash          // 冲刺
ULyraGameplayAbility_Melee         // 近战攻击
ULyraGameplayAbility_RangedWeapon  // 远程射击
```

**每个 GA 包含什么**：
```
GA_火球术：
  ├── ActivationPolicy（激活策略）→ 按 Q 键激活
  ├── Cost（消耗）           → 耗蓝 20（这本身就是一个 GE）
  ├── Cooldown（冷却）       → 冷却 5 秒（这也是一个 GE）
  ├── Tags（标签）           → Ability.Skill.Fire
  └── 执行逻辑              → 生成火球、播放动画、造成伤害
```

**关键点**：
- GA **需要被"激活"**（`ASC->TryActivateAbility()`），不是自动运行的
- 激活时会**自动检查**：够不够蓝？在不在冷却？有没有被眩晕（Tag 阻止）？
- 激活成功后会走一个固定流程：`Activate → 执行逻辑 → Commit（结算消耗/冷却）→ End`
- GA 里可以挂 **GameplayCue**（触发特效/音效）和 **GameplayEffect**（造成伤害）

> 💡 **一句话记忆**：GA = 一个可释放的技能，定义了"怎么触发、花多少代价、产生什么效果"。

---

### 2.4 ASC（AbilitySystemComponent）— 中央管理器

**是什么**：GAS 的**总管家**，挂在 Actor 上（**Lyra 挂在 PlayerState 上**，而不是角色身上）。它是唯一能"操作"前三者的类。

**它负责三件事**：
```cpp
UAbilitySystemComponent* ASC = GetASC();

ASC->TryActivateAbility(AbilityHandle);      // 1. 激活技能（触发 GA）
ASC->ApplyGameplayEffect(GEClass, ...);      // 2. 施加效果（用 GE 改属性）
ASC->GetGameplayAttributeValue(HealthAttr);  // 3. 读取属性（查 AttributeSet 的数值）
```

**为什么挂在 PlayerState 而不是 Character？**
- PlayerState **生命周期更长**（角色死亡重生时 PlayerState 还在）
- 技能的状态（冷却、Buff）不应因角色死亡而丢失
- 方便网络同步（PlayerState 本来就对所有客户端可见）

**它还管理**：
- 所有已授予的 GA 列表
- 所有正在生效的 GE（Buff/Debuff）
- 所有 Tag（当前处于什么状态：眩晕/中毒/无敌…）
- 客户端预测（让技能本地先跑起来，不等服务器）

> 💡 **一句话记忆**：ASC = 中央监控台，你想做任何事（读属性、放技能、加 Buff）都得找它。

---

### 🔗 四者关系总结（背下这张图）

```
玩家按下技能键
      │
      ▼
   ┌──────┐  检查冷却/消耗/状态   ┌──────────┐
   │ ASC  │ ───────────────────▶ │   GA     │  （技能被激活）
   │监控台 │                       │  火球术   │
   └──┬───┘                       └────┬─────┘
      │                                │
      │ ①读取蓝量                 ②消耗蓝量 │ ③造成火焰伤害
      ▼                                ▼        ▼
 ┌──────────────┐               ┌──────────────────┐
 │ AttributeSet │◀──────────────│       GE         │
 │  Mana = 100  │   修改数值      │ 消耗GE/伤害GE     │
 └──────────────┘               └──────────────────┘
```

**一句话串起来**：
> **AttributeSet** 存数值 → **GE** 负责改数值 → **GA** 是携带 GE 的技能动作 → **ASC** 是唯一把它们串起来运转的管理器。

---

## 二·补、GAS 高频方法清单（实战必背）⭐

> 这一节列出**实际开发中最常用的方法**，按"谁调用"分类。几乎每个 Lyra 功能都会用到这些。
> 标记 ⭐ 的是"天天用"级别，务必记住。

### 2.1 ASC 上的方法（最常调用）

#### 🔹 授予 / 移除技能
```cpp
// ⭐ 授予一个 GA（角色初始化时）
FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(
    FGameplayAbilitySpec(AbilityClass, Level, SourceObject));

// ⭐ 激活技能（玩家按键时）
ASC->TryActivateAbility(Handle);

// 按类激活（更常用，不用先拿 Handle）
ASC->TryActivateAbilityByClass(AbilityClass);

// 检查能否激活（通常不用手动调，内部会自动检查）
bool bCan = ASC->CanActivateAbility(Handle, CurrentActivationInfo);

// 移除技能（角色死亡/切换模式时）
ASC->ClearAbility(Handle);
ASC->ClearAllAbilities();   // 清掉所有
```

> 📖 **不知道这三个参数一开始怎么定？往下看"如何确定三个参数" ↓**
>
> <details>
> <summary><b>🧠 点击展开：我一开始怎么知道填什么？</b></summary>
>
> **核心思路转换**：不是"运行时猜参数"，而是"设计阶段就定好答案，参数只是把答案填进去"。
>
> | 你要回答的问题 | 对应参数 | 答案从哪来 |
> |---------------|---------|-----------|
> | **1. 给什么技能？** | `AbilityClass` | 你写的 GA 类（如 `ULyraGA_Jump`） |
> | **2. 技能几级？** | `Level` | 你的数值设计（初级=1，满级=5） |
> | **3. 谁授予的？** | `SourceObject` | 通常是调用者自己（`this`） |
>
> ### 1️⃣ AbilityClass —— 想给什么能力就填什么
> ```cpp
> // 想让角色会跳跃 → 填 Jump 的类
> FGameplayAbilitySpec(ULyraGameplayAbility_Jump::StaticClass(), ...)
> // ::StaticClass() 是 UE 固定写法，拿到一个类的"类型信息"
> ```
>
> ### 2️⃣ Level —— 来自数值策划设计
> - 影响 GE 里 `Level × 系数` 的伤害/冷却计算
> - 没有明确等级系统时，直接填 `1` 即可
> ```cpp
> FGameplayAbilitySpec(FireballClass, 1, this)   // 初级
> FGameplayAbilitySpec(FireballClass, 5, this)   // 满级
> ```
>
> ### 3️⃣ SourceObject —— 最容易晕，其实是"谁调用就填谁"
> **本质**：溯源标记，记录"这个技能从哪来的"，用于调试和上下文读取。
>
> | 场景 | 填什么 |
> |------|--------|
> | AbilitySet 批量授予 | `this`（AbilitySet 自己） |
> | PawnData 授予 | PawnData 对象 |
> | 装备授予的技能 | 那件装备的 Instance |
>
> > 💡 **简单记忆**：谁调用 GiveAbility，SourceObject 就填谁（`this`），90% 情况如此。
> > ```cpp
> > // Lyra 的 AbilitySet 里：
> > ASC->GiveAbility(FGameplayAbilitySpec(Entry.Ability, Level, this));
> > //                                                         ↑ 填 this
> > ```
>
> ### 🎯 写代码时的思考顺序
> ```
> 1. 要给角色什么能力？→ 跳跃 → ULyraGA_Jump::StaticClass()  (AbilityClass)
> 2. 什么强度/等级？    → 基础等级 1 → 1                      (Level)
> 3. 谁给的？          → 当前组件给的 → this                  (SourceObject)
>
> 组合：FGameplayAbilitySpec(ULyraGA_Jump::StaticClass(), 1, this)
> ```
>
> ### 🔑 关键认知转变
> 这三个参数**不需要运行时才知道**，在你写代码那一刻就已确定：
> - `AbilityClass` → 取决于你想赋予什么能力
> - `Level` → 取决于你的数值设计
> - `SourceObject` → 就是调用者自己（`this`）
>
> Lyra 把这些值配在 **AbilitySet/PawnData 数据资产**里，初始化时读出来填进去——这就是数据驱动。
>
> </details>
>
> 📖 **"写死了吗？以后怎么取？" —— 类 vs 实例的关键区分 ↓**
>
> <details>
> <summary><b>🎯 点击展开：是写死的吗？以后怎么取值？</b></summary>
>
> ### 分两层看"写死"
>
> | 层 | 内容 | 是否写死 |
> |----|------|---------|
> | **技能类 (UClass)** | `ULyraGA_Jump::StaticClass()` | ✅ 编译期固定（"跳跃"这个概念本身） |
> | **技能实例 (Spec)** | `FGameplayAbilitySpec(JumpClass, Level, this)` | ❌ 运行时动态（谁拥有、几级都不同） |
>
> ### 🔑 类 vs 实例（理解的核心）
>
> | | 技能类 (UClass) | 技能实例 (Spec/Handle) |
> |---|---|---|
> | **是什么** | "跳跃"的模板/概念 | 某角色身上的具体跳跃能力 |
> | **数量** | 全游戏只有 1 个 | 每个角色各有一份 |
> | **何时确定** | 写代码时固定 | 运行时授予时创建 |
> | **类比** | 饼干的**模具** | 用模具压出的**饼干** |
> | **例子** | `ULyraGA_Jump::StaticClass()` | `FGameplayAbilitySpec(JumpClass, 1, this)` |
>
> > 💡 同一个 JumpClass，角色 A 授予等级1、Boss 授予等级5 → **类相同，实例不同**。
>
> ### 授予后如何"取回来"？（三种方式）
>
> ```cpp
> // 方式一：用授予时返回的 Handle（需自己保存）
> FGameplayAbilitySpecHandle JumpHandle = ASC->GiveAbility(...);
> ASC->TryActivateAbility(JumpHandle);   // 激活
> ASC->ClearAbility(JumpHandle);         // 移除
>
> // 方式二：用类反查（更常用，不用存 Handle）⭐
> ASC->TryActivateAbilityByClass(JumpClass);           // 激活
> ASC->GetAbilityInstanceFromAbilityObject(JumpClass); // 取实例
>
> // 方式三：遍历 ASC 所有已授予技能
> for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) {
>     UE_LOG(LogTemp, Log, TEXT("技能: %s, 等级: %d"),
>         *Spec.Ability->GetName(), Spec.Level);
> }
> ```
>
> ### 🎮 Lyra 的真实做法（不写死在角色代码里）
>
> ```
> PawnData（数据资产，策划配）
>    └── 配置：这个角色要哪些技能 + 什么等级
>          ↓
> 角色初始化读取 PawnData
>          ↓
> 遍历技能列表，逐个 GiveAbility（程序只写一次通用逻辑）
>          ↓
> 每个实例登记到 ASC
> ```
>
> - **技能类** → 配在 PawnData/AbilitySet 数据资产里
> - **授予时机** → 角色初始化时自动完成
> - **取值方式** → 通过 ASC 的 Handle 或按类反查
>
> ### 📊 完整生命周期
>
> ```
> 【编译期】JumpClass 定义（模具，唯一）
>                │
> 【运行时】角色A出生 → GiveAbility(JumpClass, Lv1) → 创建 Spec_A → 返回 Handle_A
>                │
>          玩家按键 → TryActivateAbility(Handle_A) → 执行跳跃
>                │
>          角色A死亡 → ClearAbility(Handle_A) → 回收实例
>                │
>          （同一份 JumpClass 还能给角色B用，等级可以不同）
> ```
>
> ### ⚠️ 关于命名：JumpClass / AbilityClass 到底是什么？
>
> **没有改名！** `JumpClass` 不是引擎内置的，只是示例里的**变量别名**，代表"跳跃技能这个类"。
>
> ```cpp
> // 你定义的技能类（全名）
> class ULyraGameplayAbility_Jump : public ULyraGameplayAbility { };
>
> // JumpClass 是一个变量，存了这个类的类型信息
> TSubclassOf<UGameplayAbility> JumpClass = ULyraGameplayAbility_Jump::StaticClass();
>
> // 传给 ASC
> ASC->TryActivateAbilityByClass(JumpClass);
> ```
>
> **三个名字指同一个东西**：
>
> | 名字 | 语境 | 含义 |
> |------|------|------|
> | `ULyraGameplayAbility_Jump::StaticClass()` | 定义时 | 真实的类类型信息 |
> | `JumpClass` | 示例变量 | 指向跳跃类的别名（短，好懂） |
> | `AbilityClass` | GiveAbility 参数 | 泛指"任意技能类" |
>
> > 💡 完整写法对比：
> > ```cpp
> > // ❌ 不能直接写字面量
> > ASC->TryActivateAbilityByClass(ULyraGA_Jump);
> >
> > // ✅ 先存变量再用
> > TSubclassOf<UGameplayAbility> JumpClass = ULyraGameplayAbility_Jump::StaticClass();
> > ASC->TryActivateAbilityByClass(JumpClass);
> >
> > // ✅ 或内联写
> > ASC->TryActivateAbilityByClass(ULyraGameplayAbility_Jump::StaticClass());
> > ```
>
> **Lyra 真实做法**：不手写类名，通过 InputConfig 数据资产自动映射 InputTag → GA 类，全程数据驱动。
>
> </details>
>
> 📖 **看不懂上面那行 GiveAbility？再往下有逐词拆解 ↓**
>
> <details>
> <summary><b>🔍 点击展开：GiveAbility 逐词拆解</b></summary>
>
> **核心概念**："授予 GA" = 把一张"技能卡片"从卡池（UClass 模板）复印一份登记到角色身上。
>
> ```cpp
> FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(
>     FGameplayAbilitySpec(AbilityClass, Level, SourceObject));
> ```
>
> **逐词拆解**：
>
> | 部分 | 含义 | 例子 |
> |------|------|------|
> | `FGameplayAbilitySpec(...)` | 构造一份"技能实例档案" | 要复印的技能 + 等级 + 来源 |
> | `AbilityClass` | **哪个技能**（技能的类类型） | `ULyraGA_Jump::StaticClass()` |
> | `Level` | **技能等级**（影响伤害/冷却强度） | `1`（初级）、`5`（满级） |
> | `SourceObject` | **谁授予的**（通常填 PawnData/AbilitySet） | `this` |
> | `ASC->GiveAbility()` | 让中央监控台执行"登记入库" | — |
> | `FGameplayAbilitySpecHandle Handle` | 接收返回的**句柄**（取餐小票） | 后续激活/移除都靠它 |
>
> **Handle 是什么？**
> - 一个轻量"取号凭证"，内部只存一个 `int32` ID
> - 类似**取餐小票**：你点餐（授予）→ 店员给小票（Handle）→ 凭小票取餐（激活）/退餐（移除）
> - 为什么不直接存 Spec？因为 Spec 存在 ASC 内部数组，外部持有会悬空；Handle 是安全引用
>
> **完整流程**：
> ```cpp
> // 初始化：授予，拿到句柄
> FGameplayAbilitySpecHandle JumpHandle = ASC->GiveAbility(
>     FGameplayAbilitySpec(ULyraGameplayAbility_Jump::StaticClass(), 1, this));
>
> // 玩家按键：激活
> ASC->TryActivateAbility(JumpHandle);
> // 或更简洁：按类激活
> ASC->TryActivateAbilityByClass(ULyraGameplayAbility_Jump::StaticClass());
>
> // 角色死亡：移除
> ASC->ClearAbility(JumpHandle);
> ```
>
> **Lyra 的真实写法**：不手写单条 GiveAbility，而是通过 **AbilitySet 批量授予**（数据驱动）：
> ```cpp
> // ULyraAbilitySet::GiveToAbilitySystem 里批量授予
> for (const auto& Entry : Abilities) {
>     ASC->GiveAbility(FGameplayAbilitySpec(Entry.Ability, Level, this));
>     ASC->ApplyGameplayEffect(Entry.InitialEffects, ...);
> }
> ```
>
> **一图总结**：
> ```
> UClass(模具) ──GiveAbility(复印登记)──▶ FGameplayAbilitySpec(产品)
>                                               │
>                                       返回 Handle(取餐小票)
>                                               │
>                     ┌─────────TryActivate─────┼────Clear────────┐
>                     ▼                         ▼                 ▼
>                  激活技能                   移除技能          查询能否激活
> ```
>
> </details>

#### 🔹 施加 / 移除效果
```cpp
// ⭐ 施加 GE（造成伤害/治疗/Buff 的核心方法）
FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
Context.AddSourceObject(this);   // 记录是谁造成的

FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, Level, Context);
Spec->SetByCallerMagnitude(DamageTag, 30.0f);   // 动态设置伤害值

ASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);  // 施加给目标

// 简化版：直接施加（Lyra 里大量使用）
ASC->ApplyGameplayEffect(GEClass, Level, Context);

// 移除 GE（Buff 到期手动移除时用）
ASC->RemoveActiveGameplayEffect(Handle);
```

#### 🔹 读取属性
```cpp
// ⭐ 读取某个属性的当前值
float Health = ASC->GetNumericAttribute(ULyraAttributeSet::GetHealthAttribute());

// 读取基础值（不含 Buff 加成）
float BaseAtk = ASC->GetNumericAttributeBase(ULyraAttributeSet::GetAttackPowerAttribute());

// 获取属性变化委托（监听血量变化 → 更新血条 UI）
ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr)
   ->AddUObject(this, &ThisClass::OnHealthChanged);
```

#### 🔹 Tag 相关
```cpp
// ⭐ 检查是否拥有某 Tag（判断是否眩晕/无敌等）
bool bStunned = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Debuff.Stun"));

// 获取所有拥有的 Tag
FGameplayTagContainer OwnedTags;
ASC->GetOwnedGameplayTags(OwnedTags);

// 添加/移除 Tag（一般通过 GE 的 GrantedTags 自动管理，少手动调）
ASC->AddLooseGameplayTag(Tag);
ASC->RemoveLooseGameplayTag(Tag);
```

#### 🔹 GameplayCue（表现层）
```cpp
// ⭐ 触发特效/音效（击中爆炸、受击闪光等）
FGameplayCueParameters Params;
Params.Location = HitLocation;
Params.Instigator = this;
ASC->ExecuteGameplayCue(HitCueTag, Params);

// 持续型 Cue（如燃烧粒子一直挂着）
ASC->AddGameplayCue(HitCueTag, Params);
ASC->RemoveGameplayCue(HitCueTag);
```

---

### 2.2 GA 里的方法（写技能时用）

```cpp
// ⭐ 生命周期函数（重写这些）
virtual void ActivateAbility(...);        // 技能激活时（播放动画、生成子弹）
virtual void EndAbility(...);             // 技能结束时（清理）
virtual bool CanActivateAbility(...);     // 能否激活（自定义条件）
virtual void CommitAbility(...);          // 结算消耗/冷却（通常在技能末尾调）

// ⭐ 提交消耗和冷却（官方推荐用这个一站式方法）
CommitAbilityCooldown();   // 只结算冷却
CommitAbilityCost();       // 只结算消耗
CommitAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());  // 两者一起

// 结束技能
EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

// ⭐ 施加 GE（在技能内部造成伤害）
const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(GE_Damage, AbilityLevel, EffectContext);
ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);

// 播放蒙太奇（攻击动画）
PlayMontage(AttackMontage, PlayRate);

// 等待动画结束（异步任务）
UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    this, NAME_None, AttackMontage);
Task->OnBlendOut.AddDynamic(this, &ThisClass::EndAbility);
Task->ReadyForActivation();
```

---

### 2.3 AttributeSet 里的方法（定义属性时用）

```cpp
// ⭐ 构造函数里声明属性
UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health)
FGameplayAttributeData Health;
REPLATED_ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health);   // 自动生成 Get/Set/Init

// 初始化属性（角色出生时设初始值）
virtual void PostAttributeInitialize(const FGameplayAttribute& Attribute, float MaxValue);

// ⭐ 属性变化回调（每次被 GE 修改后触发，伤害计算的关键位置）
virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data);

// 预属性变化（可在这里拦截/钳制，如血量不能超过上限）
virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);

// 预属性基础值变化（钳制基础值，如攻击力不能为负）
virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const;
```

> 💡 **关键区别**：
> - `PostGameplayEffectExecute` — GE 执行**之后**触发，适合处理"血量归零→死亡"这类逻辑
> - `PreAttributeChange` — GE 执行**之前**触发，适合"钳制数值不超过上限"

---

### 2.4 GE 相关（配置伤害公式时用）

```cpp
// ExecutionCalculation（伤害公式，见第四节）
// 在 GE 的 Modifier 里指定 MagnitudeCalculationClass = ULyraDamageExecution

// 动态设置伤害值（代码里传具体数字）
Spec->SetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), 30.0f);

// 用属性作为伤害来源（如"攻击力 × 倍率"）
Spec->SetSetByCallerMagnitude(Tag, Value);
```

---

### 2.5 高频方法速查表

| 场景 | 方法 | 重要度 |
|------|------|--------|
| 角色初始化授予技能 | `ASC->GiveAbility()` | ⭐⭐⭐ |
| 玩家按键释放技能 | `ASC->TryActivateAbilityByClass()` | ⭐⭐⭐ |
| 造成伤害/治疗 | `ASC->ApplyGameplayEffect()` | ⭐⭐⭐ |
| 读取血量/蓝量 | `ASC->GetNumericAttribute()` | ⭐⭐⭐ |
| 监听血量变化更新 UI | `ASC->GetGameplayAttributeValueChangeDelegate()` | ⭐⭐⭐ |
| 触发击中特效 | `ASC->ExecuteGameplayCue()` | ⭐⭐ |
| 检查是否眩晕/无敌 | `ASC->HasMatchingGameplayTag()` | ⭐⭐ |
| 技能激活逻辑 | `GA::ActivateAbility()` | ⭐⭐⭐ |
| 结算消耗+冷却 | `GA::CommitAbility()` | ⭐⭐⭐ |
| 播放攻击动画 | `GA::PlayMontage()` + `AbilityTask` | ⭐⭐ |
| 定义属性 | `AttributeSet` 构造 + `REPLATED_ATTRIBUTE_ACCESSORS` | ⭐⭐ |
| 属性变化处理死亡 | `AttributeSet::PostGameplayEffectExecute()` | ⭐⭐⭐ |
| 钳制血量上限 | `AttributeSet::PreAttributeChange()` | ⭐⭐ |

---

### 2.6 一个完整流程串联（射击技能的典型调用链）

```cpp
// 1. 玩家按射击键 → InputConfig 映射到 GA_RangedWeapon
// 2. ASC 激活技能
ASC->TryActivateAbilityByClass(ULyraGameplayAbility_RangedWeapon::StaticClass());

// 3. GA 内部：ActivateAbility()
void ULyraGameplayAbility_RangedWeapon::ActivateAbility(...)
{
    // 播放开火动画
    PlayMontage(FireMontage);
    
    // 等待动画到特定帧（AnimNotify 触发）
    // ...
}

// 4. AnimNotify 触发 → 生成子弹、施加伤害 GE
void OnFireAnimNotify()
{
    SpawnProjectile();
    
    // 对目标施加伤害 GE
    FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(GE_LyraDamage, AbilityLevel, Context);
    Spec->SetByCallerMagnitude(DamageTag, WeaponDamage);
    ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
    
    // 触发枪口火焰 Cue
    ExecuteGameplayCue(MuzzleFlashTag, Params);
}

// 5. 目标 AttributeSet 的 PostGameplayEffectExecute 被调用
void ULyraHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Health <= 0) {
        // 触发死亡
        DeathState = true;
    }
}
```

---

## 三、目录结构

```
GameplayAbilities/
├── Source/
│   ├── GameplayAbilities/       ← 主模块（C++ 核心）
│   │   ├── Public/
│   │   │   ├── AbilitySystemComponent.h      ← ASC
│   │   │   ├── GameplayAbility.h             ← GA 基类
│   │   │   ├── GameplayEffect.h              ← GE
│   │   │   ├── GameplayEffectTypes.h         ← GE 类型
│   │   │   ├── AttributeSet.h                ← 属性集基类
│   │   │   ├── GameplayTagContainer.h        ← Tag 容器
│   │   │   └── ...
│   │   └── Private/
│   ├── GameplayAbilitiesEditor/ ← 编辑器扩展
│   └── GameplayAbilityTests/    ← 测试
├── Content/                     ← 蓝图资产
└── GameplayAbilities.uplugin
```

---

## 四、关键设计思想

### 4.1 GameplayTag 驱动一切
每个技能/效果/属性都用 **GameplayTag** 标识：

```cpp
// 技能标签
Ability.Attack.Melee
Ability.Attack.Ranged
Ability.Movement.Jump

// 状态标签
State.Debuff.Stun
State.Buff.Shield

// 事件标签
Event.Damage.Dealt
Event.Health.Death
```

Tag 用于：
- 技能的激活条件（需要/阻止某些 Tag）
- 效果的过滤（只对特定 Tag 生效）
- 动画/特效的触发（GameplayCue）

### 4.2 预测系统 (Prediction)
GAS 内置**客户端预测**，让技能在服务器确认前就能本地表现：

```cpp
FPredictionKey PredictionKey;
ASC->TryActivateAbilityByClass(AbilityClass, true, &PredictionKey);
```

这对射击/动作游戏至关重要（否则会有明显延迟感）。

### 4.3 GameplayCue（表现层）
把"逻辑"和"表现"分离：
- **GA/GE** 负责逻辑（扣血、加 Buff）
- **GameplayCue** 负责表现（特效、音效、震动）

```cpp
// 触发一个 Cue（如击中爆炸特效）
ASC->ExecuteGameplayCue(HitCueTag, params);
```

### 4.4 ExecutionCalculation（伤害公式）
自定义伤害计算逻辑：

```cpp
// Lyra 的 LyraDamageExecution
// 读取攻击力、防御力、暴击率 → 算出最终伤害
class ULyraDamageExecution : public UGameplayEffectExecutionCalculation;
```

---

## 五、Lyra 中的用法示例

### 5.1 生命值组件如何与 GAS 交互

```cpp
// ULyraHealthComponent 监听 GAS 属性变化
void ULyraHealthComponent::PostInitializeComponents()
{
    // 拿到 ASC 的生命值属性
    HealthAttribute = ULyraAttributeSet::GetHealthAttribute();
    
    // 监听变化
    ASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute)
       ->AddUObject(this, &ThisClass::HandleHealthChanged);
}

void ULyraHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    // 血量变化 → 更新 UI / 判断死亡
    if (NewValue <= 0) { OnDeath(); }
}
```

### 5.2 技能激活流程

```
1. 玩家按攻击键
2. InputConfig 映射到 AbilityInputTag
3. ASC 收到输入 → 查找对应 GA
4. CanActivateAbility? (检查冷却/消耗/Tag)
5. ActivateAbility → 播放动画/Montage
6. AnimNotify 触发 → 生成子弹/判定伤害
7. ApplyGameplayEffect → 扣血
8. GameplayCue → 播放特效/音效
9. CommitAbility → 扣除消耗、进入冷却
```

---

## 六、学习建议

1. **先理解四大概念** — ASC/GA/GE/AttributeSet 的关系
2. **看懂 LyraHealthSet** — 最简单的属性集示例
3. **跟踪一个完整技能** — 如 LyraGameplayAbility_Jump
4. **动手实践** — 跟着教程做一个新 GA（如护盾）

## 七、下一步

- [02_CommonUI与UMG](./02_CommonUI与UMG.md) — UI 框架
- [03_ModularGameplay组件化](./03_ModularGameplay组件化.md) — 角色组件化
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
