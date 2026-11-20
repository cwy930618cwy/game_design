# P05 AbilitySystem（GAS 核心）—— 总 md（为什么 + 全景 + 依赖分析）

> 依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\AbilitySystem\`（**51 文件**，含 `Abilities/` `Attributes/` `Executions/` `Phases/` 四个子目录）
> 你新建：`Source/TenkaichiGame/AbilitySystem/` 下对应文件（工程该目录已有 P05.1 三个叶子）
> 前缀替换：`Lyra` → `Tenkaichi`（`ULyraAbilitySystemComponent`→`UTenkaichiAbilitySystemComponent`、`LyraAbilitySystemComponent.generated.h`→`TenkaichiAbilitySystemComponent.generated.h` 等，铁律 24/41）。
> **依赖结论（铁律 30，已查 `.h`+`.cpp` 真实 include）**：P05 是**全项目最大、依赖最深的阶段**，横跨 P02/P04/P06/P12/P14/P16 与引擎 GAS。**不能整体独立编译**——`.cpp` 依赖 P14（AssetManager）、P16（AnimInstance）未就位，需**注释挂起**。其余依赖类（P06/P12）的 `.h` 已在工程，可编译。

---

## 一、这个阶段解决什么问题（为什么）

**AbilitySystem = GAS（Gameplay Ability System）核心**。前面 P00~P04 都是"地基/叶子"，从 P05 开始真正进入**战斗能力系统的中枢**。Lyra 把引擎自带的 GAS 深度定制，分四块：

1. **`AbilitySystemComponent`（ASC，P05.2）**：GAS 的"中央处理器"——挂在角色上，管理所有技能的授予/激活/取消、输入分发、激活组互斥、Tag 关系扩展。**所有能力能力的核心容器**。
2. **`GameplayAbility`（P05.3，`Abilities/`）**：**技能基类**——定义技能怎么激活（输入触发/持续/出生）、怎么互斥（激活组）、怎么花钱（额外消耗）、怎么失败反馈（消息/动画）。Lyra 所有具体技能都继承它。
3. **`AbilitySet` / `GlobalAbilitySystem`（P05.3）**：技能的组织与批量授予——`AbilitySet` 把一组技能/效果打包成资产；`GlobalAbilitySystem` 给**所有** ASC 统一授予技能/效果（全局 Buff）。
4. **`Attributes/`（P05.4）**：属性集——`AttributeSet`（血/蓝等数值）、`CombatSet`（攻击/防御）、`HealthSet`（生命）。
5. **`Executions/`（P05.4）**：伤害/治疗计算——`DamageExecution`、`HealExecution`（MMC 修改器 + 计算）。
6. **`GameplayEffectContext` / `TargetData` / `TaggedActor` / `Phases/`（P05.5）**：效果上下文、目标数据、标签 Actor、游戏阶段（战斗阶段/入场阶段状态机）。

> 🔑 **一句话**：P05 用引擎 GAS + Lyra 深度定制，搭起「ASC 容器 + 技能基类 + 技能资产打包 + 全局授予 + 属性/伤害计算 + 阶段状态机」整套战斗能力中枢，是解开阶段四 13 项挂起的钥匙。

---

## 二、依赖全景（铁律 30 查证，按被依赖在下）

```
P05 AbilitySystem
├─ P05.1 前置叶子（✅ 已沿用重构版1，工程已落盘）
│    ├─ GameplayCueManager        —— 引擎 GameplayCues ✅
│    ├─ AbilitySourceInterface    —— 纯接口（引擎 Interface）✅
│    └─ AbilityTagRelationshipMapping —— 引擎 GameplayTags ✅
├─ P05.3 GameplayAbility（Abilities/，含 AbilityCost/SimpleFailureMessage）
│    ├─ 引擎：Abilities/GameplayAbility.h（GAS ✅ Build.cs 已加）
│    ├─ 已教：GameplayTags ✅ / LogChannels ✅ / PhysicalMaterialWithTags(P00) ✅
│    ├─ 已教：LyraCameraMode(P04) ✅ / GameplayMessageSubsystem 插件 ✅ / AbilitySourceInterface(P05.1) ✅
│    ├─ P05.5：LyraGameplayEffectContext（同阶段，一起落盘）
│    ├─ P06/P12：LyraCharacter.h / LyraHeroComponent.h / LyraPlayerController.h —— **.h 已在工程**（P06/P12 只挂 .cpp），可编译
│    └─ 内部：Abilities/ LyraAbilityCost.h / LyraAbilitySimpleFailureMessage.h
├─ P05.2 AbilitySystemComponent / AbilitySystemGlobals
│    ├─ 引擎：AbilitySystemComponent.h / AbilitySystemGlobals.h ✅
│    ├─ P05.1：AbilityTagRelationshipMapping ✅ / P05.3：GameplayAbility（死结）
│    ├─ P05.3：GlobalAbilitySystem（同阶段）
│    ├─ P02：LyraGameData ✅ / LyraLogChannels ✅
│    ├─ 🔴 P14：LyraAssetManager.h —— **工程无此文件，需挂起**
│    └─ 🔴 P16：LyraAnimInstance.h —— **工程无此文件，需挂起**
├─ P05.4 Attributes/ Executions/
│    └─ 引擎 GAS（AttributeSet / GameplayEffectExecutionCalculation）+ P05.2/05.3
└─ P05.5 GameplayEffectContext / TargetData / TaggedActor / Phases/
     └─ 引擎 GAS + P05.2/05.3 + P02/已教内容
```

### 具体每个文件的真实依赖（`.cpp` include 为准）与编译判断

| 文件 | 关键 `.cpp` 依赖 | 能否编译 |
|------|------------------|----------|
| `LyraGameplayCueManager` | 引擎 GameplayCues | ✅（P05.1 已落盘） |
| `LyraAbilitySourceInterface` | 纯接口，无 `.cpp` 硬调用 | ✅（P05.1 已落盘） |
| `LyraAbilityTagRelationshipMapping` | 引擎 GameplayTags | ✅（P05.1 已落盘） |
| `LyraGameplayAbility` | PlayerController/Character/HeroComponent `.h`（P06/P12 存在）+ CameraMode(P04✅)+物理(P00✅)+Message插件✅+EffectContext(P05.5) | ✅ 可编译 |
| `LyraAbilityCost`（Abilities/） | 引擎 GAS + GameplayAbility | ✅ 可编译 |
| `LyraAbilitySimpleFailureMessage` | 纯结构体 `.h` | ✅ 可编译 |
| `LyraAbilitySystemGlobals` | `LyraGameplayEffectContext.h`（P05.5） | ✅ 与 P05.5 一起落盘后可编译 |
| `LyraAbilitySystemComponent` | 🔴 `LyraAssetManager.h`（P14 无）+ 🔴 `LyraAnimInstance.h`（P16 无）+ GameData(P02✅)+GlobalAbilitySystem(P05.3) | ⚠️ **需挂起 AssetManager/AnimInstance 段** |
| `LyraGlobalAbilitySystem` | 引擎 WorldSubsystem + 前置声明 ASC | ✅ 可编译 |

### 挂起总结（铁律 33/34，需登记注释备忘录）

**P05.2 `LyraAbilitySystemComponent.cpp` 不能整体独立编译**，因 `.cpp` 硬 include 两个工程不存在的头：
- `#include "System/LyraAssetManager.h"`（**P14 AssetManager 未教**）
- `#include "Animation/LyraAnimInstance.h"`（**P16 Animation 未教**）

**处理**：`AddDynamicTagGameplayEffect` / `RemoveDynamicTagGameplayEffect`（用 AssetManager+GameData）和 `InitAbilityActorInfo` 里的 `ULyraAnimInstance` 段，`#if 0` 挂起 + include 注释；等 P14/P16 就位后解开。其余（输入分发、激活组、Tag 关系、失败通知等）可编译，当场完整落盘。

> ✅ **已落地（2026-10-22 教 P05.2 `.cpp`）**：`TenkaichiAbilitySystemComponent.cpp` 已落盘，挂起 4 段（`EndPlay` 函数体 + `InitAbilityActorInfo` 的 RegisterASC/Anim 两段 + `Add/RemoveDynamicTagGameplayEffect` 函数体）+ 3 个 include（`Animation/TenkaichiAnimInstance.h` + `TenkaichiGlobalAbilitySystem.h` + `System/TenkaichiAssetManager.h`），登记注释备忘录 ㉓，待 P05.3-C GlobalAbilitySystem / P14 AssetManager / P16 AnimInstance 就位后放开。

> ✅ **已落地（2026-10-22 教 P05.2-2 `.cpp`）**：`TenkaichiAbilitySystemGlobals.cpp` 已教（总 md + `.h` + `.cpp` md 已建）。`.h` 可独立编译零挂起（`FGameplayEffectContext*` 返回前置声明即可）；`.cpp` 构造函数不挂起，`AllocGameplayEffectContext` 函数体 + `TenkaichiGameplayEffectContext.h` include `#if 0` 挂起，登记注释备忘录 ㉔，待 P05.5 EffectContext 就位后放开。

> ✅ **已落地（2026-10-22 教 P05.3-3 `.cpp`）**：`TenkaichiGlobalAbilitySystem.cpp` 已教（总 md + `.h` + `.cpp` md 已建，`P05.3-3_TenkaichiGlobalAbilitySystem_*`）。`.h` 可独立编译零挂起；`.cpp` 依赖 P05.2 ASC（已落盘）全部可编译**零挂起**。**联动**：GlobalAbilitySystem 就位后，ASC.cpp ㉓ 里 `EndPlay`（`UnregisterASC`）段 + `InitAbilityActorInfo` 的 RegisterASC 段可解开（仅这两段，其余仍挂起待 P14/P16）。

> ✅ **已落地（2026-10-22 教 P05.3-4 `.cpp`）**：`TenkaichiAbilitySet.cpp` 已教（总 md + `.h` + `.cpp` md 已建，`P05.3-4_TenkaichiAbilitySet_*`）。`.h` 可独立编译零挂起；`.cpp` 依赖 P05.3-B GameplayAbility + P05.2 ASC + 第0步 LogChannels（全已落盘）全部可编译**零挂起**。**P05.3-C（GlobalAbilitySystem + AbilitySet）全部完成**。

> ✅ **已落地（2026-10-22 教 P05.4-1 `.h`）**：`TenkaichiAttributeSet` 基类 `.h` 已教（`P05.4-1_TenkaichiAttributeSet_h.md` 已建）。`.h` 可独立编译零挂起（`ATTRIBUTE_ACCESSORS` 宏 + `FTenkaichiAttributeEvent` 委托 + `UTenkaichiAttributeSet` 基类，前置声明足够）。工程 `TenkaichiAttributeSet.h` 已落盘（⚠️ 目前放在 `AbilitySystem\` 根目录，应按 Lyra 移到 `AbilitySystem\Attributes\` 子目录）。

> ✅ **已落地（2026-10-22 教 P05.4-1 `.cpp`）**：`TenkaichiAttributeSet` 基类 `.cpp` 已教（`P05.4-1_TenkaichiAttributeSet_cpp.md` 已建）。`.cpp` 依赖 P05.2 ASC（已落盘）可编译零挂起（构造函数空 + `GetWorld()` 走 `GetOuter()` 链 + `GetTenkaichiAbilitySystemComponent()` 一次 `Cast`）。工程 `.cpp` 待落盘。

> 其余 P05 文件（GameplayAbility、AbilitySystemGlobals、GlobalAbilitySystem、AbilityCost、Attributes、Executions、EffectContext、TargetData、TaggedActor、Phases）因依赖的 `.h`（P06/P12）已在工程，**可编译，零挂起**。

---

## 三、全景：本阶段关键类/结构职责（51 文件，按子目录归纳）

### 根目录（AbilitySystem/ 下）
| 类 | 子阶段 | 一句话职责 |
|----|--------|-----------|
| `ULyraGameplayCueManager` | P05.1✅ | 游戏提示管理器（扩展引擎，路由 GameplayCue 到世界/UI） |
| `ILyraAbilitySourceInterface` | P05.1✅ | 能力来源接口（武器/道具作为伤害来源） |
| `ULyraAbilityTagRelationshipMapping` | P05.1✅ | 技能 Tag 关系表（激活/取消/阻塞的 Tag 扩展） |
| `ULyraAbilitySystemComponent` | P05.2 | **ASC 中央处理器**（技能授予/激活/输入分发/激活组互斥） |
| `ULyraAbilitySystemGlobals` | P05.2 | GAS 全局单例（分配自定义 EffectContext） |
| `ULyraAbilitySet` | P05.3 | 技能资产打包（一组技能/效果/属性，一键授予） |
| `ULyraGlobalAbilitySystem` | P05.3 | 全局技能系统（给所有 ASC 批量授予技能/效果） |
| `FLyraGameplayEffectContext` | P05.5 | 效果上下文（携带来源/能力Tag/命中结果） |
| `ULyraGameplayAbilityTargetData_SingleTargetHit` | P05.5 | 单目标命中数据（伤害目标信息） |
| `ALyraTaggedActor` | P05.5 | 带标签的 Actor 基类 |
| `ULyraAbilityTagRelationshipMapping` | P05.1✅ | （同上） |

### Abilities/（P05.3，技能相关）
| 类 | 一句话职责 |
|----|-----------|
| `ULyraGameplayAbility` | **技能基类**（激活策略/激活组/额外消耗/失败反馈/相机模式） |
| `ULyraAbilityCost` | 技能额外消耗抽象基类 |
| `ULyraAbilityCost_InventoryItem` | 消耗库存物品 |
| `ULyraAbilityCost_ItemTagStack` | 消耗物品 Tag 堆栈 |
| `ULyraAbilityCost_PlayerTagStack` | 消耗玩家 Tag 堆栈 |
| `FLyraAbilitySimpleFailureMessage` | 技能失败简单消息结构体 |
| `ULyraGameplayAbility_Death` | 死亡技能 |
| `ULyraGameplayAbility_Jump` | 跳跃技能 |
| `ULyraGameplayAbility_Reset` | 重置技能 |

### Attributes/（P05.4）
| 类 | 一句话职责 |
|----|-----------|
| `ULyraAttributeSet` | 属性集基类 |
| `ULyraCombatSet` | 战斗属性（攻击/防御/暴击等） |
| `ULyraHealthSet` | 生命属性（当前/最大/回血等） |

### Executions/（P05.4）
| 类 | 一句话职责 |
|----|-----------|
| `ULyraDamageExecution` | 伤害计算执行（MMC 算出伤害） |
| `ULyraHealExecution` | 治疗计算执行 |

### Phases/（P05.5）
| 类 | 一句话职责 |
|----|-----------|
| `ULyraGamePhaseAbility` | 游戏阶段技能（进入某个阶段的技能） |
| `ULyraGamePhaseSubsystem` | 游戏阶段子系统（阶段状态机） |
| `FLyraGamePhaseLog` | 阶段日志通道 |

> 注：`Phases/` 内 `GamePhaseSubsystem` 用 GameFeatureAction（P15）动态注册阶段，`.cpp` 可能依赖后期内容，教到时再按铁律 30 查证。

---

## 四、教学节奏（铁律 29/35，严格按依赖自下而上）

> ⚠️ **顺序修正**：总纲原 P05.2（ASC）→ P05.3（GameplayAbility）顺序**不依赖自下而上**——因为 `AbilitySystemComponent.h` include `GameplayAbility.h`，**必须先教 `LyraGameplayAbility`（P05.3 的基类部分），再教 `AbilitySystemComponent`（P05.2）**。两者 `.cpp` 互相依赖构成死结，一起落盘即闭环（死结处理铁律）。

- ⬜ **本总 md**（P05.0）：概念 + 全景 + 依赖分析 + 挂起清单。
- ✅ **P05.1 前置叶子**：`GameplayCueManager` / `AbilitySourceInterface` / `AbilityTagRelationshipMapping` —— 重构版1 阶段五 `3.3`/`3.4.1`/`3.4.2` 已教完，工程 `TenkaichiGameplayCueManager` / `TenkaichiAbilitySourceInterface` / `TenkaichiAbilityTagRelationshipMapping` 已落盘。**直接沿用，不重教**。
- ✅ **P05.3-A**：`LyraAbilityCost`（基类+3 个实现）—— **总 md ✅ + `.h` ✅ + `.cpp` ✅**（`P05.3-1_TenkaichiAbilityCost_总md/_h/_cpp.md` 三件套已建；4 个 `.h` 可编译零挂起，`InventoryItem.cpp` Lyra 自带 `#if 0` 可编译；`ItemTagStack.cpp`/`PlayerTagStack.cpp` 整体 `#if 0` 挂起，登记注释备忘录 ⑳㉑，待 P05.3-B GameplayAbility + 阶段八/九 就位）。
- ✅ **P05.3-B**：`LyraGameplayAbility`（技能基类，Abilities/）+ `LyraAbilitySimpleFailureMessage`（小结构体，随 `.cpp` 顺带教）—— **总 md ✅ + `.h` ✅ + `.cpp` ✅**（`P05.3-2_TenkaichiGameplayAbility_总md/_h/_cpp.md` 三件套已建；`.h` 可独立编译零挂起；`.cpp` 硬依赖 P05.2 ASC + P05.5 EffectContext 的 **6 个函数 `#if 0` 挂起**（`CanActivateAbility`/`ApplyCost`/`MakeEffectContext`/`DoesAbilitySatisfyTagRequirements`/`CanChangeActivationGroup`/`ChangeActivationGroup`）+ 两个 include 挂起，登记注释备忘录 ㉒，待 P05.2/P05.5 就位放开；其余函数可编译）。
- ✅ **P05.2**：`LyraAbilitySystemGlobals` + `LyraAbilitySystemComponent`（与 GameplayAbility 死结，一起落盘）—— **P05.2-1 ASC ✅：总 md ✅ + `.h` ✅ + `.cpp` ✅**（`P05.2-1_TenkaichiAbilitySystemComponent_总md/_h/_cpp.md` 三件套已建，工程 `TenkaichiAbilitySystemComponent.h/.cpp` 已落盘；`.h` 可独立编译零挂起；`.cpp` 4 段（`EndPlay`+`InitAbilityActorInfo` 两段+动态标签两函数）+ 3 include `#if 0` 挂起待 P05.3-C GlobalAbilitySystem/P14 AssetManager/P16 AnimInstance，登记注释备忘录㉓）。**P05.2-2 AbilitySystemGlobals ✅：总 md ✅ + `.h` ✅ + `.cpp` ✅**（`P05.2-2_TenkaichiAbilitySystemGlobals_总md/_h/_cpp.md` 三件套已建；`.h` 可独立编译零挂起；`.cpp` 构造函数不挂起、`AllocGameplayEffectContext` 函数体 + include `#if 0` 挂起待 P05.5 EffectContext，登记注释备忘录㉔）。
- ✅ **P05.3-C**：`LyraGlobalAbilitySystem`（`P05.3-3_TenkaichiGlobalAbilitySystem_总md/_h/_cpp.md`，工程待落盘，`.h`+`.cpp` 均可编译零挂起）+ `LyraAbilitySet`（`P05.3-4_TenkaichiAbilitySet_总md/_h/_cpp.md`，工程待落盘，`.h`+`.cpp` 均可编译零挂起）。
- 🟡 **P05.4**：`Attributes/`（AttributeSet/CombatSet/HealthSet）+ `Executions/`（Damage/Heal）—— **P05.4-1 AttributeSet 主题（基类+CombatSet+HealthSet）全部完成** ✅：主题总 md ✅（`P05.4-1_TenkaichiAttributeSet_总md.md`）；**基类 `.h` ✅ + `.cpp` ✅**（`P05.4-1_TenkaichiAttributeSet_h/_cpp.md`，工程已落盘，⚠️ 路径待移到 `Attributes\` 子目录，可编译零挂起）；**CombatSet `.h` ✅ + `.cpp` ✅**（`P05.4-1_TenkaichiCombatSet_h/_cpp.md`，工程已落盘，⚠️ 也在根目录，可编译零挂起）；**HealthSet `.h` ✅ + `.cpp` ✅**（`P05.4-1_TenkaichiHealthSet_h/_cpp.md`，工程已落盘，依赖 P05.2 ASC/第0步 GameplayTags/P01 VerbMessage/CommonGame 全就位，可编译零挂起）；**P05.4-2 Executions 总 md ✅**（`P05.4-2_Executions_总md.md`，含 Damage/Heal 2 类全景 + 依赖分析）；待教 Damage/Heal 的 `.h` → `.cpp`（Damage `.cpp` 依赖 P05.5 EffectContext + 阶段四 TeamSubsystem 需挂起）。
- 🟡 **P05.5**：`GameplayEffectContext` / `TargetData` / `TaggedActor` / `Phases/`（含 `GamePhaseAbility` / `GamePhaseSubsystem` / `GamePhaseLog`）—— **主题总 md ✅**（`P05.5_EffectContext等_总md.md` 已建，含 4 组内容全景 + 依赖分析：EffectContext/TargetData/TaggedActor 零挂起可编译，Phases 的 GamePhaseSubsystem 待查证 P15）。
  - ✅ **P05.5-1 `FLyraGameplayEffectContext` 全部教完**：主题总 md ✅ + 独立总 md ✅ + `.h` ✅（`P05.5-1_TenkaichiGameplayEffectContext_h.md`，工程 `AbilitySystem\TenkaichiGameplayEffectContext.h` 已落盘 81 行）+ `.cpp` ✅（`P05.5-1_TenkaichiGameplayEffectContext_cpp.md`，工程 `.cpp` 已落盘 72 行，5 个函数（ExtractEffectContext / NetSerialize / SetAbilitySource / GetAbilitySource / GetPhysicalMaterial）+ Iris 段齐全、注释全中文、零挂起，2026-10-26 核对通过）。**🔓 这是解开挂起 ㉔㉕㉒ 的钥匙**。
  - ✅ **P05.5-2 `FLyraGameplayAbilityTargetData_SingleTargetHit` 全部教完**：独立总 md ✅（`P05.5-2_SingleTargetHit_总md.md`）+ `.h` ✅（`P05.5-2_..._h.md`，工程已落盘 48 行，三个易错点全对：NetSerialize 非 virtual / TStructOpsTypeTraits 只有 WithNetSerializer / 注释中文）+ `.cpp` ✅（`P05.5-2_..._cpp.md`，工程已落盘 33 行，2 个函数（AddTargetDataToContext + NetSerialize）+ include 齐全、注释全中文、零挂起，2026-10-26 核对通过）。
  - ✅ **P05.5-3 `ALyraTaggedActor` 全部教完**：独立总 md ✅（`P05.5-3_TaggedActor_总md.md`）+ `.h` ✅（`P05.5-3_TenkaichiTaggedActor_h.md`，工程已落盘 36 行，与 Lyra 行数完全一致）+ `.cpp` ✅（`P05.5-3_TenkaichiTaggedActor_cpp.md`，工程已落盘 32 行，3 个函数、构造函数无默认参数、`GET_MEMBER_NAME_CHECKED(AActor, Tags)` 原样、`Super::CanEditChange` 兜底齐全，2026-10-26 核对通过）。
  - 🟡 **P05.5-4 `Phases/`**（GamePhaseAbility / GamePhaseSubsystem / GamePhaseLog，3 文件）：**主题总 md ✅**（`P05.5-4_Phases_总md.md`）。依赖已查证（铁律 30）：**整组零挂起** —— 只依赖 P05.3-B GameplayAbility（✅已落盘）+ P05.2 ASC（✅已落盘）+ 引擎 `WorldSubsystem`/`GameStateBase`/`DataValidation`。⚠️ Subsystem.cpp 运行时需 GameState 挂 ASC（P13 阶段），**不影响编译，照写不挂起**。
  - ✅ **P05.5-4-1 `ULyraGamePhaseAbility` 全部教完**：独立总 md ✅（`P05.5-4-1_TenkaichiGamePhaseAbility_总md.md`）+ `.h` ✅（工程已落盘 48 行，`Abstract`/`HideCategories=Input`、`GetGamePhaseTag` 内联、`Category="Tenkaichi|Game Phase"` 全对）+ `.cpp` ✅（工程已落盘 68 行，4 个 GA 策略、`Super::` 兜底在 `if` 外、`LOCTEXT_NAMESPACE` 已替换、`#undef` 无参、UI 文本保持英文，2026-10-26 核对通过）。
  - ✅ **P05.5-4-2 `ULyraGamePhaseSubsystem` 全部教完**：独立总 md ✅ + `.h` ✅（工程已落盘 119 行）+ `.cpp` ✅（工程已落盘 238 行，13 个函数全齐：4 生命周期 + 2 启动 + 4 观察者 + **`OnBeginPhase`/`OnEndPhase`/`IsMatch` 核心裁判**；`!IncomingPhaseTag.MatchesTag(ActivePhaseTag)` 互斥判定、两步走 `ActivePhases` 数组防迭代器失效、`ensure`/`CastChecked`/`FindChecked`/`FindOrAdd`、`CreateWeakLambda` 全对，2026-10-26 核对通过）。
  - ✅ **P05.5-4-3 `LogTenkaichiGamePhase`**（`.h` 7 行已落盘核对通过，**只有 `.h` 无 `.cpp`**，2026-10-26）。**🎉 P05.5 主题 4 个子项（EffectContext / SingleTargetHit / TaggedActor / Phases 3 文件）全部教完，整组零挂起。**
  - 🟡 **P05.6 `Abilities/` 剩余三类**（Jump / Death / Reset）：**主题总 md ✅**（`P05.6_Abilities剩余_总md.md`）。依赖已查证（铁律 30）：**Jump 零挂起**、**Reset 零挂起**（`ATenkaichiCharacter::Reset()` + GameplayTags 4 标签 + 消息子系统全就位）、**Death `.h` 零挂起 / `.cpp` 需挂起**（`UTenkaichiHealthComponent` 工程不存在，登记注释备忘录 ㉖，待 P06）。**下一步：建 P05.6-1 Jump 自己的总 md**。：**独立总 md ✅**（`P05.5-4-3_TenkaichiGamePhaseLog_总md.md`）。⚠️ **它不是类，是日志分类**；**只有 `.h` 无 `.cpp`**（`DEFINE_LOG_CATEGORY` 已写在 4-2 的 `.cpp` 第 16 行，Lyra 原样）。依赖仅引擎 `Logging/LogMacros.h`，**零挂起**；**下一步：教 `.h`（7 行，P05 最短文件）**。
- ⬜ **P05 收尾**：整体编译 + 解开注释备忘录挂起项（①②③④⑤⑥⑧⑨⑩⑪⑬ 等 13 项），P05 就位 = GAS 核心落地。

> 每个独立类都按铁律 35 先建它自己的总 md → 停 → 教 `.h` → 停 → 教 `.cpp`。

---

## 五、衔接说明

- P05 依赖**引擎 GAS**：`GameplayAbilities`/`GameplayTags`/`GameplayTasks` 三个模块 Build.cs 第 0 步已启用（`TenkaichiGame.Build.cs` 第 30~32 行）——**无新增插件**。
- P05 依赖的 **P06 Character / P12 Player 的 `.h` 已在工程**（MEMORY：`.cpp` 断点挂起，`.h` 完整）→ P05 的 `GameplayAbility.cpp`/`AbilitySystemComponent.h` 能 include 它们、**能编译**。
- P05 依赖的 **P14 AssetManager / P16 AnimInstance 尚未建文件** → `AbilitySystemComponent.cpp` 对应 `#include` + 函数体 `#if 0` 挂起，登记注释备忘录（铁律 33/34）。
- **P05 GAS 核心就位 = 解开阶段四 12 项 + P01.5 的 GAS 段（共 13 项）挂起的时机**（注释备忘录台账）。P05 每个断点类教完、对应依赖就位后，逐项提醒解开。

---

## 待确认

1. P05 AbilitySystem 总 md 看懂了：这是全项目**最大、依赖最深**的阶段（51 文件），搭起「ASC 容器 + 技能基类 + 技能资产打包 + 全局授予 + 属性/伤害计算 + 阶段状态机」整套战斗能力中枢，是解开阶段四 13 项挂起的钥匙。
2. **P05.1 前置叶子已沿用完成**（GameplayCueManager / AbilitySourceInterface / AbilityTagRelationshipMapping，工程已落盘）——直接沿用，不重教，OK 吗？
3. **教学顺序已按依赖修正**：先 `LyraGameplayAbility`（P05.3，技能基类）→ 再 `AbilitySystemComponent`（P05.2，ASC，AssetManager/AnimInstance 段挂起）→ 再 GlobalAbilitySystem/AbilitySet → Attributes/Executions → EffectContext/TargetData/Phases。与总纲原 P05.2→P05.3 顺序不同，按此推进 OK 吗？
4. **P05.2 AbilitySystemComponent.cpp 需挂起** P14（AssetManager）/ P16（AnimInstance）两段（登记注释备忘录），其余可编译。确认后我从 **P05.3-A `LyraAbilityCost`** 开始教。

---

## 源码依据

- Lyra 文件夹：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\AbilitySystem\`（51 文件，含 4 子目录）
- 依赖（`.cpp` include 为准）：
  - `LyraGameplayAbility.cpp` → `Player/LyraPlayerController.h`(P12)、`Character/LyraCharacter.h`+`LyraHeroComponent.h`(P06)、`Camera/LyraCameraMode.h`(P04)、`Physics/PhysicalMaterialWithTags.h`(P00)、`LyraAbilityCost.h`+`LyraAbilitySimpleFailureMessage.h`(Abilities)、`GameplayMessageSubsystem.h`(插件)、`LyraAbilitySourceInterface.h`(P05.1)、`LyraGameplayEffectContext.h`(P05.5)、`LyraGameplayTags.h`+`LyraLogChannels.h`(第0步)
  - `LyraAbilitySystemComponent.cpp` → `System/LyraAssetManager.h`(🔴P14)、`Animation/LyraAnimInstance.h`(🔴P16)、`System/LyraGameData.h`(P02)、`LyraGlobalAbilitySystem.h`(P05.3)、`LyraAbilityTagRelationshipMapping.h`(P05.1)、`Abilities/LyraGameplayAbility.h`(P05.3)
  - `LyraAbilitySystemGlobals.cpp` → `LyraGameplayEffectContext.h`(P05.5)
- 引擎模块：`GameplayAbilities`/`GameplayTags`/`GameplayTasks`（Build.cs 第 30~32 行已加）