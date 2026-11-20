# 12 — Lyra 到底把 GAS 改成什么样了？改造思路全梳理

> **定位**：上一篇理清了"GAS = 整套框架，ASC = 里面那个管家"。这一篇回答：**Lyra 拿到官方 GAS 这套框架后，动手改了哪些地方？整体思路是什么？** 这能帮你从"看懂类"升级到"看懂工程的组织哲学"。
>
> 不看零散代码，按"官方给了什么 → Lyra 补了什么"来对比讲。

---

## 〇、先说结论（Lyra 改 GAS 的一句话思路）

> **官方 GAS 是"一套能用的通用零件库，但不关心你的具体游戏"；Lyra 的思路是：不改 GAS 的地基（不用 ASC 的机理重写），而是——①把它封装成数据驱动、②给它补上玩法规则、③给它接上 Lyra 自己的输入/镜头/动画/装备体系。** 说白了：**官方给的是"引擎能力"，Lyra 在上面铺了一层"游戏规则层"。**

---

## 一、官方 GAS 只给了什么？（改之前的"地基"）

官方 GAS（`GameplayAbilities` 插件）免费送你：

| 官方提供 | 它解决 | 它不给的 |
|---|---|---|
| `UAbilitySystemComponent`（ASC） | 技能授予/激活/取消、效果、属性、Tag、**网络预测** | 不知道你游戏的输入长啥样、技能怎么触发 |
| `UGameplayAbility` | 一个技能能干嘛 | 不知道"这技能该按一下还是按住、跟别的技能冲不冲突" |
| `UGameplayEffect` | 改数值/Tag 的一张表 | 不知道伤害/治疗的公式 |
| `UAttributeSet` | 属性怎么存 | 不知道你要哪些属性 |
| 基础 Tag/Cue | 标记状态、播特效 | 没有项目级归类 |

**核心认知**：官方 GAS 是"玩法无关"的通用引擎能力。它强在**网络同步 + 预测 + 数值系统**这套硬核地基，但**技能怎么触发、怎么互斥、怎么算伤害**这些"游戏规则"，官方两手一摊，留给每个项目自己定。

---

## 二、Lyra 的改造思路：四大招

看完 Lyra `AbilitySystem/` 目录，你会发现它其实只做了四件"加法"，没推翻任何地基。

```
官方 GAS  ──────────────►  Lyra（在官方上面加了"游戏规则层"）
│ 通用引擎能力               │
│                          ├─ ① 数据驱动：AbilitySet 打包授权
│                          ├─ ② 玩法规则：输入策略 / 激活互斥
│                          ├─ ③ 数值延伸：伤害治疗公式 + 项目属性
│                          └─ ④ 外围缝合：镜头/动画/装备/全局广播
```

---

## 三、第一招：数据驱动——把"授权什么技能"变成数据资产

### 官方的问题
授予技能/效果/属性，得靠**写代码逐个调** `GiveAbility`。换一套角色就重写一遍，很笨。

### Lyra 的解法：`ULyraAbilitySet`（能力包资产）
它不是类，是 `UPrimaryDataAsset`（数据资产）。里面装着三个"待授权清单"：

```
ULyraAbilitySet（能力包 = 一张"配方表"）
├─ GrantedGameplayAbilities：要给哪些【技能】（附等级 + 输入 Tag）
├─ GrantedGameplayEffects  ：要给哪些【效果】
└─ GrantedAttributes       ：要给哪些【属性集】
        │
        ▼ GiveToAbilitySystem()
  把整包一次性授权给某个 ULyraAbilitySystemComponent
```

**好处**：以后想给"骑士"配一套技能，美术/策划在编辑器里填一张资产就行，**不写一行 C++**。想换职业，换个数据资产。这就是之前笔记里反复强调的 Lyra **"配方化、数据驱动"** 哲学，在能力系统里的体现。

> **教学场景**：你在做一个"职业选择"功能——战士、法师、刺客三职业。用 `ULyraAbilitySet` 做三份数据资产，玩家转职时 `GiveToAbilitySystem` 换上对应资产，比写死在代码里优雅一百倍。

---

## 四、第二招：玩法规则——给"技能怎么跑"定规矩

### Lyra 给 `UGameplayAbility` 加了两个"玩法开关"（在 `ULyraGameplayAbility` 里）

这些是官方完全没有、Lyra 自己发明的字段：

**开关 1：激活策略 `ActivationPolicy`（什么时候激活）**
```cpp
enum class ELyraAbilityActivationPolicy : uint8
{
	OnInputTriggered,  // 按一下触发一次（闪现）
	WhileInputActive,  // 按住持续（蓄力/引导）
	OnSpawn            // 一出生自动激活（被动光环）
};
```

**开关 2：激活分组 `ActivationGroup`（跟别的技能怎么共存）**
```cpp
enum class ELyraAbilityActivationGroup : uint8
{
	Independent,          // 独立：随便跑（走路）
	Exclusive_Replaceable,// 独占·会被顶掉（普攻）
	Exclusive_Blocking,   // 独占·挡住别人（开大）
};
```

### 怎么被用起来？（回到 `ULyraAbilitySystemComponent`）
```
每帧 ProcessAbilityInput：
  玩家按"技能Tag" → 查所有绑了该Tag的技能
  按其 ActivationPolicy 决定是"触发/按住持续激活"
  按其 ActivationGroup 决定"会不会顶掉其他独占技能"
```

> **这就是游戏手感的关键**。官方 GAS 只管"技能能不能激活"，Lyra 用这两个字段把"竞技游戏的技能操控手感"给补上了——你在别的 GAS 项目里写死在这儿的逻辑，Lyra 变成了**可配置的枚举**。

---

## 五、第三招：数值延伸——属性集 + 伤害公式

### Lyra 把"属性"拆成了业务化的一堆 Set（`Attributes/`）
官方只有一个空壳 `UAttributeSet`，Lyra 分成：
- `ULyraCombatSet`：战斗属性（攻击力、暴击等）
- `ULyraHealthSet`：血量 + **出生/击杀事件委托**（`OnHealthChanged`、`OnOutOfHealth` 等）

### Lyra 把"伤害/治疗"抽成独立的"执行类"（`Executions/`）
- `ULyraDamageExecution`：**伤害计算算法**（读取武器伤害 → 算减伤 → 得出最终伤害）
- `ULyraHealExecution`：治疗算法

**为什么单独抽 Executions？** 官方 GAS 里伤害公式是写在 `UGameplayEffect` 的 `Execution` 里的。Lyra 的做法是给"伤害"和"治疗"各写一个**标准执行器**，让所有武器/技能都用同一个公式，保证数值一致、好维护、好 debug。

---

## 六、第四招：外围缝合——让 GAS 融入 Lyra 自己的世界

这是最能体现"Lyra 是完整工程不是 demo"的部分，GAS 跟 Lyra 的其他模块全接上了：

| Lyra 的 GAS 类 | 它缝合的外部系统 |
|---|---|
| `ULyraGameplayAbility` | 塞进 `TSubclassOf<ULyraCameraMode>`：**技能能切镜头**（放大招拉近） |
| `ULyraGameplayAbility` + AnimInstance | 技能播动画蒙太奇、失败播对应动作 |
| `ULyraAbilityCost_InventoryItem` | 施法需要**消耗背包里的某个物品**（装备/消耗品系统） |
| `ULyraAbilityCost_PlayerTagStack` | 施法消耗玩家身上的 **Tag 层数**（如"体力值"用 Tag 层数表示） |
| `ULyraGlobalAbilitySystem` | 世界级广播：给**所有角色的 ASC** 统一上 buff（全服环境效果） |
| `ULyraGameplayEffectContext` | 扩展了效果的上下文，携带更多"是谁造成、来源武器"等信息 |
| `Phases/`（GamePhaseAbility） | 用技能系统驱动**游戏阶段切换**（进对局、结算），GAS 连"玩法流程"都管 |

> **关键洞察**：官方 GAS 是"孤立的技能工具"。Lyra 通过**接口 + 封装**把它织进了一张大网——技能既能切镜头、又能消耗背包、又能播动画。**GAS 从"技能执行器"升级成了"玩法行为总调度"**。这就是之前学的 ModularGameplay / DataAsset / PawnData 那套"组合、数据驱动"思想在能力系统上的集中爆发。

---

## 七、总结：Lyra 改造 GAS 的四个关键词

| # | 关键词 | 做了什么 |
|---|---|---|
| 1 | **数据驱动** | `ULyraAbilitySet` 把"授权啥"变成资产，零代码配职业 |
| 2 | **玩法规则** | 给 Ability 加 `ActivationPolicy`/`ActivationGroup`，把手感做成可配置枚举 |
| 3 | **数值延伸** | 业务化属性集（Combat/Health）+ 统一伤害/治疗执行器 |
| 4 | **外围缝合** | 接入镜头/动画/背包/全局广播/游戏阶段，让 GAS 融进整个工程 |

**一句话收束**：Lyra 没有"改写"GAS 的地基（网络预测、数值系统照用），而是**在官方 ASC/Ability/Effect/AttributeSet 之上加了一层"数据驱动 + 玩法规则 + 工程缝合"的壳**，把一套通用工具打磨成了适合做完整商业化射击游戏的专用框架。

---

## 八、下一步

- 深入 `ULyraAbilitySet` 的 `GiveToAbilitySystem` 完整流程（它怎么把技能/效果/属性一股脑给 ASC）。
- 深入 `ULyraHealthSet` 的血量事件：死亡、出生、击杀是怎么通知出去的（和之前 Pawn 的生命周期呼应）。
- 深入 `Phases/` 游戏阶段子系统的玩法流程驱动。
- 回看 `ULyraGameplayAbility.cpp`，看镜头切换、失败消息这些"缝合"具体代码怎么写。
