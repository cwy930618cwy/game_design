# AbilitySystem 目录总览：都是干嘛的

> 结论先说：**这个目录 = 引擎 GAS 的"Lyra 定制层"。**
>
> 引擎已经给了你一套 GAS（技能能打、有属性、有特效）。Lyra 在这个目录里做的事只有一件：
> **在引擎给的东西上，各盖一层自己的子类，加上项目需要的能力。**

📌 阅读顺序：**本篇（看地图）→ 分区单篇 → 具体文件 .h 笔记**

---

## 一、目录长相

```
AbilitySystem/
├── (10 组文件直接躺在根目录)   ← 定制的 GAS 底座
├── Abilities/                  ← "技能"
├── Attributes/                 ← "数值"
├── Executions/                 ← "算数的地方"
└── Phases/                     ← "游戏阶段"
```

| 区 | 文件 | 一句话 |
|---|---|---|
| **根目录** | 10 组 `.h/.cpp` | Lyra 定制的 GAS 底座（ASC、能力礼包、Cue 管理、GE 上下文…） |
| **Abilities/** | 9 组 | 项目的技能基类 + 花费系统 + 3 个具体技能 |
| **Attributes/** | 3 组 | 血量、战斗数值 |
| **Executions/** | 2 组 | 伤害/治疗的**实际数学** |
| **Phases/** | 3 组 | 比赛当前处于哪个阶段 |

---

## 二、根目录：10 个"改装的引擎零件"

这 10 个文件基本是**一对一"盖在引擎某个类上面"**：

| 文件 | 盖在谁上面 | 加了什么 |
|---|---|---|
| `LyraAbilitySystemComponent` | `UAbilitySystemComponent` | **①输入缓冲 ②激活组 ③标签关系** |
| `LyraAbilitySystemGlobals` | `UAbilitySystemGlobals` | 告诉引擎"请用我们的 EffectContext"（**配置入口**） |
| `LyraGameplayEffectContext` | `FGameplayEffectContext` | 多带：命中结果、物理材质、`CartridgeID`、伤害来源对象 |
| `LyraGameplayAbilityTargetData_SingleTargetHit` | `FGameplayAbilityTargetData_SingleTargetHit` | 加 `CartridgeID`（识别"同一发子弹的多颗弹丸"） |
| `LyraAbilitySourceInterface` | 接口（不继承谁） | **伤害来源自己算衰减**：距离衰减、物理材质衰减 |
| `LyraAbilityTagRelationshipMapping` | `UDataAsset` | 一张配置表：**什么技能挡住/取消什么技能** |
| `LyraAbilitySet` | `UPrimaryDataAsset` | **能力礼包**：一次发放一堆技能+效果+属性集，并返回可回收的 handles |
| `LyraGlobalAbilitySystem` | `UWorldSubsystem` | 给**全场所有人**统一发/收技能和效果 |
| `LyraGameplayCueManager` | `UGameplayCueManager` | 管特效的加载，避免开局把所有特效都读进来 |
| `LyraTaggedActor` | `AActor` | 一个最简 Actor，身上挂一组静态 GameplayTag |

**其中三个最该先认识：**

**① `LyraAbilitySystemComponent`（ASC）** —— 整个 GAS 的中枢。
Lyra 在它身上加了三样东西，这也是和引擎版最大的区别：

- **输入缓冲**：按键不直接激活技能，而是先记在三个列表里（按下/松开/按住），**每帧统一处理一次**（`ProcessAbilityInput`）。
- **激活组**：技能分三档互斥等级 —— `Independent`（互不干扰）、`Exclusive_Replaceable`（会被顶掉，比如开火）、`Exclusive_Blocking`（谁都别想插队，比如处决）。ASC 维护每种组的运行计数，来判断能不能进。
- **标签关系映射**：技能本身只写"我是什么标签"，**"我挡住谁、我被谁挡"写在配置表里**，策划改表不用改代码。

**② `LyraAbilitySet`** —— 发放用的"礼包"。
角色初始化、捡到武器、装备道具时，都需要"一口气给我一堆技能和效果"。这个 DataAsset 就是干这个的，给完还返回 handles，方便以后**收回**（比如丢枪时把枪的技能收掉）。

**③ `LyraGameplayCueManager`** —— 只负责"表现"。
GameplayCue 是技能的特效/音效层。它不参与逻辑，只管加载策略（异步、延迟、常驻）。

---

## 三、Abilities/：技能

| 文件 | 干嘛的 |
|---|---|
| `LyraGameplayAbility` | **项目的技能基类**（所有技能都继承它） |
| `LyraAbilityCost` | **花费抽象基类**：激活技能要付什么代价 |
| `LyraAbilityCost_InventoryItem` | 花费 = 消耗一定数量的某个物品（子弹） |
| `LyraAbilityCost_ItemTagStack` | 花费 = 消耗**物品身上**某个标签的层数（弹匣余量） |
| `LyraAbilityCost_PlayerTagStack` | 花费 = 消耗**玩家身上**某个标签的层数（体力/充能） |
| `LyraAbilitySimpleFailureMessage` | "技能失败了"的消息结构体（给 UI 用） |
| `LyraGameplayAbility_Death` | 死亡技能（由 `GameplayEvent.Death` 自动触发） |
| `LyraGameplayAbility_Jump` | 跳跃技能 |
| `LyraGameplayAbility_Reset` | 把玩家重置回出生状态（仅服务器） |

### 基类 `LyraGameplayAbility` 加了什么

这是全目录最该精读的一个类。它主要加了四件事：

**1. 激活策略（ActivationPolicy）—— 决定"技能什么时候能放"**

| 策略 | 含义 | 例子 |
|---|---|---|
| `OnInputTriggered` | 按下才激活 | 开火（半自动） |
| `WhileInputActive` | 按着就一直尝试激活 | 全自动开火、蓄力 |
| `OnSpawn` | 一被挂到角色上就自动放 | 被动技能、初始化 |

**2. 激活组（ActivationGroup）** —— 和上面 ASC 的组配套，决定技能间的互斥关系。

**3. 可插拔花费（AdditionalCosts）** —— 技能不用自己写"检查弹药"，而是挂一个 `ULyraAbilityCost` 实例：
- `CheckCost()` 检查够不够（不够就填一个失败标签，UI 据此提示）
- `ApplyCost()` 扣钱
- 还有个 `bOnlyApplyCostOnHit`：**命中才扣**（打空了不扣子弹）

**4. 失败反馈** —— 技能失败时不是干瞪眼：
- `FailureTagToUserFacingMessages`：失败标签 → 给玩家看的文字
- `FailureTagToAnimMontage`：失败标签 → 播个动画（比如"没子弹"时抖一下枪）

另外还提供了 `SetCameraMode()` —— **技能可以临时换相机**（开镜就是靠这个）。

---

## 四、Attributes/：数值

| 文件 | 干嘛的 |
|---|---|
| `LyraAttributeSet` | 属性集基类 + 一个省事宏 `ATTRIBUTE_ACCESSORS` + 六参数事件委托 |
| `LyraCombatSet` | **攻击方**的数值：`BaseDamage`、`BaseHeal` |
| `LyraHealthSet` | **受击方**的数值：`Health`、`MaxHealth`、`Healing`、`Damage` |

**关键区分（很多人在这里绕晕）：**

```
LyraCombatSet（在开枪的人身上）      LyraHealthSet（在被打的人身上）
    BaseDamage = 30      ──开火──▶      Damage = 30    ← 临时值！
                                        Health: 100 → 70
                                        Damage: 30 → 0 ← 用完清零
```

`Damage` 和 `Healing` **不是真实数值，是"过路值"（meta attribute）**：

- 它们只是"这一下要打进去多少"
- 在 `PostGameplayEffectExecute` 里被换算成 `-Health` / `+Health`，**然后立刻归零**
- 而 `Health` 标了 `HideFromModifiers`，意思就是：**只有 Execution 能改它**，普通 GE 修改器碰不到

`LyraHealthSet` 还广播三个事件，用来通知别处：

| 事件 | 何时 |
|---|---|
| `OnHealthChanged` | 血量变了 |
| `OnMaxHealthChanged` | 上限变了 |
| `OnOutOfHealth` | 血量**第一次掉到 0**（注意：有 `bOutOfHealth` 防重复触发） |

---

## 五、Executions/：真正算数的地方

| 文件 | 干嘛的 |
|---|---|
| `LyraDamageExecution` | 伤害计算 |
| `LyraHealExecution` | 治疗计算 |

它们是 `UGameplayEffectExecutionCalculation` 子类，也就是"**在技能和属性之间的那道计算工序**"。

**`LyraDamageExecution` 的计算公式（源码里是一条乘法）：**

```
最终伤害 = BaseDamage
         × 距离衰减        （子弹飞得越远越弱）
         × 物理材质衰减    （打头/打甲不一样）
         × 队伍关系        （队友误伤 = 0）
```

几个细节：
- 距离衰减和材质衰减**不是它自己算的**，而是问 `ILyraAbilitySourceInterface`（也就是**开枪那把武器**）要
- 队伍关系问 `ULyraTeamSubsystem::CanCauseDamage()`，所以**友伤屏蔽在这里统一处理**
- 它输出的**不是"扣血"**，而是往目标身上写一个 `Damage` 值 —— 剩下的交给 `LyraHealthSet`（见上一节）
- 整个算数包在 `#if WITH_SERVER_CODE` 里，**只在服务器算**

---

## 六、Phases/：游戏阶段

| 文件 | 干嘛的 |
|---|---|
| `LyraGamePhaseAbility` | 一个"技能"，但它代表的是**游戏阶段**（不是一个动作） |
| `LyraGamePhaseSubsystem` | 管理阶段的世界子系统 |
| `LyraGamePhaseLog` | 日志频道，很小 |

**核心设计：阶段用"标签的层级"表示嵌套关系。**

```
Game.Playing                ← 父阶段
Game.Playing.WarmUp         ← 子阶段，可以和父共存
Game.Playing.CaptureTheFlag ← 和 WarmUp 是"兄弟"，不能共存
```

规则一句话：**启动新阶段时，把所有"不是它祖先"的活跃阶段结束掉。**

- 从 `WarmUp` 切到 `CaptureTheFlag`：`WarmUp` 被结束，`Game.Playing` 保留
- 别人想"阶段开始时提醒我"，就注册 `WhenPhaseStartsOrIsActive()`，支持精确匹配和前缀匹配两种

---

## 七、把五个区串起来：开一枪的完整流程

这是理解整个目录最好的方法——**一次开火，五个区全都出场**：

```
① 玩家按左键
   └─ HeroComponent 打一个 InputTag → 传到 ASC
                          【根目录·LyraAbilitySystemComponent】
                             记进"按下"列表，本帧稍后统一处理

② ASC 每帧处理输入（ProcessAbilityInput）
   └─ 找到带这个标签的技能 → 看它的 ActivationPolicy
                          【Abilities·LyraGameplayAbility】

③ 要激活？先过三关：
   ├─ 激活组允许吗      【根目录·ASC 的 ActivationGroupCounts】
   ├─ 标签关系允许吗    【根目录·LyraAbilityTagRelationshipMapping】
   └─ 花费付得起吗      【Abilities·LyraAbilityCost_*】

④ 技能执行：打射线 → 生成命中数据
                          【根目录·..._SingleTargetHit，带 CartridgeID】

⑤ 应用一个 GameplayEffect，用伤害计算工序
                          【Executions·LyraDamageExecution】
   └─ 读开枪者的 BaseDamage  【Attributes·LyraCombatSet】
   └─ 问武器算衰减           【根目录·LyraAbilitySourceInterface】
   └─ 问队伍系统能不能打     【Teams·LyraTeamSubsystem】
   └─ 输出 Damage 值

⑥ 目标身上的属性集收到
                          【Attributes·LyraHealthSet】
   ├─ PreGameplayEffectExecute：检查免疫 / GodMode → 可能直接归零
   ├─ PostGameplayEffectExecute：Damage → -Health，clamp，然后 Damage 清零
   ├─ 广播 OnHealthChanged
   ├─ 广播一条 Lyra.Damage.Message 消息（给别人听）
   └─ 血量到 0 → 广播 OnOutOfHealth → 触发死亡技能
                          【Abilities·LyraGameplayAbility_Death】

⑦ 全程：命中特效由 Cue 系统负责
                          【根目录·LyraGameplayCueManager】
```

再加一个全局视角：如果作弊器要"让全场所有人受伤"或"开局给所有人加技能"，走的是
`LyraGlobalAbilitySystem`（**根目录**）—— 它是唯一一个"不针对某个 ASC，而是针对所有人"的东西。

---

## 八、四个最常见的困惑

**Q1：为什么伤害要绕一圈写成 `Damage` 再转成 `-Health`？**
因为要留一个"拦截点"。`Damage` 只是候选值，在转成 `-Health` 之前，属性集可以检查免疫、GodMode、上限，还能顺便广播消息。如果 Execution 直接扣血，这些检查就没地方放。

**Q2：`LyraCombatSet` 和 `LyraHealthSet` 都有"伤害"相关字段，用哪个？**
`CombatSet` 是**攻击方**的（我这一下能打多少），`HealthSet` 是**受击方**的（我收到多少、还剩多少）。两个 ASC 上的两套属性。

**Q3：技能的自定义，为什么有的写在技能类里，有的写在配置表里？**
技能类里写的是**"我是什么"**（标签、组、花费），配置表（`LyraAbilityTagRelationshipMapping`）里写的是**"我和别人什么关系"**（谁挡谁）。这样加新技能不用改老代码。

**Q4：`Phases` 里的"阶段"和技能是什么关系？**
同一套机制。阶段就是"一个永不主动结束的技能"，用它的激活/结束来表示"进入/离开某个阶段"。好处是可以复用 GAS 的标签、网络同步。

---

## 九、如果想继续深入（建议顺序）

| 顺序 | 看什么 | 为什么 |
|---|---|---|
| 1 | `LyraGameplayAbility.h` | 项目的技能长什么样，最常打交道 |
| 2 | `LyraAbilitySystemComponent.h` | 输入缓冲 + 激活组，理解"技能为什么没放出来" |
| 3 | `LyraAbilitySet.h` | 理解"技能是怎么发到身上的" |
| 4 | `LyraHealthSet.h` + `LyraDamageExecution.cpp` | 理解一次伤害的完整链路 |
| 5 | `LyraAbilityTagRelationshipMapping.h` | 理解技能互斥为什么不用改代码 |
| 6 | `LyraGamePhaseSubsystem.h` | 理解比赛流程怎么驱动 |

---

## 十、三句话总结

1. **这个目录是"盖层"，不是"新框架"**：每个文件几乎都是引擎某个 GAS 类的 Lyra 子类。
2. **改动的方向只有两个**：把逻辑从代码搬到配置（标签关系、能力礼包、花费），以及把输入/互斥这类事集中到 ASC 统一管。
3. **一条主线记住全部**：输入 → ASC 处理 → 技能基类 → 花费/标签校验 → Execution 算数 → AttributeSet 落地 → 事件/消息/阶段响应。
