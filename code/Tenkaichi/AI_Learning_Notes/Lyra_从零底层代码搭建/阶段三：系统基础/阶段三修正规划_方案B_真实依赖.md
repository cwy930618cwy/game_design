# 阶段三修正规划（方案 B）—— 按真实依赖链重新编排

> **性质**：吸取"线 A/B 划分失真"教训后，用**真实查证的依赖树**重新编排的阶段三教学规划。
> **触发**：原规划把 `GameData` 当"独立可编译"，实查发现它依赖 `AssetManager`（线 B），而 `AssetManager` 又无底依赖 GAS + 角色体系。用户选定**方案 B**（把 GAS 相关类提前教）。
> **本文件的承诺**：每个类都标注了**真实查证的依赖**（`.h` + `.cpp` 都看过），并按"能不能自包含"诚实分层，不再凭"我以为它独立"分组。
> ⚠️ **与总纲的对应**：本文件结论已并入 `00_学习路径总纲_自下而上.md` 的阶段三~五。阶段三 = 系统基础地基 + GAS 叶子；`AssetManager`/`GameData`/`GameInstance` 为**收口类**，移到阶段五教。教学顺序以总纲第四节为准。

---

## 〇、先说这次怎么吸取教训（方法论）

上次翻车，是因为**没查证就分组**。这次立三条硬规则：

1. **每个类先看 `.h` + `.cpp` 的完整 `#include` 和硬调用**，再定性。
2. **依赖要追到底**：A 依赖 B，就继续看 B 依赖什么，直到全是"引擎类 / 插件 / 已教内容 / 已还原代码"。
3. **区分"软引用"和"硬依赖"**：`.h` 里 `TSoftClassPtr` 不阻塞编译；`.cpp` 里**硬 `#include` + 硬调用**才决定能不能编译。

以下所有分层都基于真实查证，不是拍脑袋。

---

## 一、真实依赖树（已查证）

```
（被依赖在下，依赖别人在上）

[未就绪的高层 —— 无底断点]
  Character ──→ TeamAgentInterface(Teams) / HealthComponent / PawnExtensionComponent / PlayerState / ASC
  PlayerController ──→ CommonPlayerController(CommonGame插件 未装!) / CameraAssistInterface / TeamAgentInterface
  HeroComponent ──→ GameFrameworkInitStateInterface / PawnComponent / GameFeatureAction / InputConfig / CameraMode

[核心节点 —— 一碰就无底]
  GameplayAbility ──→ Character / PlayerController / HeroComponent / GameplayEffectContext / PhysicalMaterialWithTags / AbilitySimpleFailureMessage / GameplayMessageSubsystem(✓L2)
  AbilitySystemComponent ──→ GameplayAbility / AnimInstance / GlobalAbilitySystem / AssetManager / GameData
  AbilitySet ──→ GameplayAbility / AbilitySystemComponent

[可自包含 —— 只依赖引擎/GAS插件/已教]
  GameplayCueManager ──→ UGameplayCueManager(引擎) / LogChannels(✓L1) / GameplayTagsManager(引擎)     ✅
  TagRelationshipMapping ──→ GameplayTagContainer(引擎)                                                 ✅
  InputConfig ──→ LogChannels(✓L1) / UInputAction(EnhancedInput插件)                                   ✅
  AbilityCost ──→ UGameplayAbility(引擎) / 前置声明 ULyraGameplayAbility                                ✅
  AbilitySourceInterface ──→ UPhysicalMaterial(引擎) / tag                                              ✅
```

---

## 二、关键结论：能"提前"的只有叶子，碰节点就无底

**能真正自包含、现在就教的 GAS 类（5 个叶子）**：

| 类 | 依赖 | 能否独立编译 |
|----|------|-------------|
| `TenkaichiAbilitySourceInterface` | 引擎接口 | ✅ 完全独立 |
| `TenkaichiAbilityCost` | GAS 引擎 + 前置声明 | ✅ 几乎独立 |
| `TenkaichiAbilityTagRelationshipMapping` | GAS 引擎 tag 类 | ✅ 完全独立 |
| `TenkaichiInputConfig` | LogChannels + EnhancedInput | ✅ 几乎独立 |
| `TenkaichiGameplayCueManager` | GAS 引擎类 + LogChannels | ✅ 几乎独立 |

**一碰就无底的节点（3 个核心类）**：

| 类 | 硬依赖的未就绪类 | 后果 |
|----|-----------------|------|
| `TenkaichiGameplayAbility` | Character / PlayerController / HeroComponent | 拖出角色体系 |
| `TenkaichiAbilitySystemComponent` | GameplayAbility / AnimInstance / GlobalAbilitySystem / AssetManager / GameData | 双向依赖 + 动画 |
| `TenkaichiAbilitySet` | GameplayAbility / ASC | 依赖上面两个 |

**真正的断点**：`PlayerController` 继承 **`CommonPlayerController`**，而 `CommonGame` 插件**还没装**。也就是说，要碰 `GameplayAbility`，就得先还原 `Character`/`PlayerController`/`HeroComponent`，而 `PlayerController` 又要先有 `CommonGame` 插件 + Teams + CameraAssist…… 这是**把阶段四 + CommonGame 插件一起提前**。

---

## 三、修正后的教学顺序（自下而上，分三段）

### 阶段 A：先教能自包含的 GAS 叶子（5 个，现在就能做）

按依赖由浅到深：

1. **`TenkaichiAbilitySourceInterface`**（接口，最简单）
2. **`TenkaichiAbilityCost`**（能力消耗基类）
3. **`TenkaichiAbilityTagRelationshipMapping`**（能力 Tag 关系表）
4. **`TenkaichiInputConfig`**（输入配置，依赖 EnhancedInput）
5. **`TenkaichiGameplayCueManager`**（GAS 技能提示管理器）

> 每个都走「总 md → `.h` → `.cpp`」，一步一反馈。
> 这 5 个完成后，GAS 的"叶子层"就位，不依赖任何未教的东西。

### 阶段 B：断点决策点（必须先跟用户确认，不能自己定）

`GameplayAbility` / `AbilitySystemComponent` / `AbilitySet` 三个核心节点，**无法独立教**。要继续只有两条路：

- **路线 B1**：把 `Character`、`PlayerController`、`HeroComponent`、`PawnExtensionComponent`、`HealthComponent`、`PlayerState`、`Teams` 以及 **CommonGame 插件**全部提前 → 等于把阶段四 + L2 插件也拉进来，范围爆炸。
- **路线 B2**：三个核心节点**先按 Lyra 原样写，但用"占位声明"隔离**未就绪的高层类（Character/PC/Hero 只给前置声明），等阶段四/五补实 → 保证阶段三能编译，但阶段四/五要回头补。

> **这不是我能替你定的**，取决于你愿不愿意把阶段四/CommonGame 提前。所以到这一步**必须停**，让你选。

### 阶段 C：回到阶段三原目标（System/ 目录）

等 GAS 依赖解决后，再按真实依赖教：

- `TenkaichiGameData` → 依赖 AssetManager
- `TenkaichiAssetManager` → 依赖 GameData + GameplayCueManager + PawnData
- `TenkaichiGameInstance` → 依赖 CommonGame + Player
- 以及 `GameSession`、`ActorUtilities`、`SystemStatics`、`DevelopmentStatics` 等独立类

---

## 四、本次明确承认的边界（不假装能解决）

| 依赖 | 状态 |
|------|------|
| CommonGame 插件 | ❌ 未装，`PlayerController` 直接依赖，阶段三动不了 |
| Teams / Character / PlayerState | ❌ 阶段五，`Character` 依赖 |
| PawnExtensionComponent / HealthComponent | ❌ 阶段五，`Character` 依赖 |
| AnimInstance | ❌ 阶段五动画，`ASC` 依赖 |

> **诚实结论**：阶段三的 `GameData`/`AssetManager`/`GameplayAbility` 体系，**不是"提前教几个 GAS 类"能解决的**，它真正依赖的是"角色/玩家/Team/CommonGame 整块"。要么把这块整体提前（范围爆炸），要么用占位隔离（阶段四五回头补）。

---

## 五、下一步建议（请用户确认）

**先教阶段 A 的 5 个 GAS 叶子**（现在就能做、不依赖任何未教内容）。这 5 个教完，GAS 的底层就位，到时候再面对阶段 B 的断点决策，你能基于真实情况选择。

**要不要现在开始教第 1 个 `TenkaichiAbilitySourceInterface`？**（按「总 md → h → cpp」走）