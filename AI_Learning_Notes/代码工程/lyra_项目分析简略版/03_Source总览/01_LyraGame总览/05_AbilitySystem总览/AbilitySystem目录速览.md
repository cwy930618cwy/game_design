# `AbilitySystem/` 目录速览

> 27 个类，51 个文件，**有 4 个子目录**。全项目第二大的目录，GAS 在 Lyra 里的落地全在这。

## 目录结构

```
AbilitySystem/
├── 根目录（10 个类）  ← ASC 本体与周边基础设施
│
├── Abilities/    (9 个类)  ← 技能本体 + 技能消耗
├── Attributes/   (3 个类)  ← 属性集：血量 / 战斗 / 基础
├── Executions/   (2 个类)  ← GE 执行计算：伤害 / 治疗
└── Phases/       (3 个类)  ← 游戏阶段（把"阶段"也做成 Ability）
```

## 根目录 10 个类

| 类 | 一句话 |
|---|---|
| `LyraAbilitySystemComponent` | ⭐⭐ ASC 本体：输入处理、激活组、Tag 关系 |
| `LyraAbilitySet` | ⭐ 数据资产：一组"技能 + GE + 属性集"打包授予 |
| `LyraAbilityTagRelationshipMapping` | 配置：技能 Tag 之间谁阻断/取消谁 |
| `LyraAbilitySystemGlobals` | 让引擎用 Lyra 自己的 `EffectContext` |
| `LyraGameplayCueManager` | GameplayCue 的加载与预加载管理 |
| `LyraGameplayEffectContext` | 自定义 GE 上下文，能携带伤害来源和弹夹 ID |
| `LyraGlobalAbilitySystem` | 给**所有** ASC 批量施加技能/GE |
| `LyraAbilitySourceInterface` | 接口：伤害衰减怎么算（距离、物理材质） |
| `LyraTaggedActor` | 带静态 GameplayTag 的简易 Actor |
| `LyraGameplayAbilityTargetData_SingleTargetHit` | 命中数据，带弹夹 ID |

## 四个子目录

| 子目录 | 类数 | 内容 |
|---|---|---|
| `Abilities/` | 9 | `LyraGameplayAbility` 基类、`Jump`/`Death`/`Reset` 三个具体技能、4 种技能消耗 |
| `Attributes/` | 3 | `LyraHealthSet`（血量）、`LyraCombatSet`（伤害/护甲）、`LyraAttributeSet`（基础） |
| `Executions/` | 2 | `LyraDamageExecution`（伤害怎么算）、`LyraHealExecution` |
| `Phases/` | 3 | `LyraGamePhaseAbility` + `LyraGamePhaseSubsystem`（把游戏阶段做成 Ability） |

## 补充说明

| 点 | 说明 |
|---|---|
| Lyra 的技能必须是"实例化的" | ASC 里多处 `ensureMsgf` 检查 `GetInstancingPolicy() != NonInstanced`（注释说 NonInstanced 因易用性问题正被弃用） |
| 输入 → 技能的路径 | `AbilityInputTagPressed(Tag)` → 按 InputTag 找 Spec → `ProcessAbilityInput()` 统一激活 |
| 激活组（ActivationGroup） | `Independent`（不冲突）/ `Exclusive_Replaceable`（可被打断）/ `Exclusive_Blocking`（会挡住别人） |
| 技能 Tag 关系从哪来 | `PawnData->TagRelationshipMapping`，由 `LyraPawnExtensionComponent::InitializeAbilitySystem` 设进去 |

**优先级**：`LyraAbilitySystemComponent` → `LyraAbilitySet` → `Abilities/LyraGameplayAbility` → `Attributes/LyraHealthSet`
