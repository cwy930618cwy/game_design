# P10 Interaction —— 阶段总 md

> **定位**：本文件是 P10 Interaction（交互系统）的**阶段总 md**（为什么 + 全景 + 依赖分析 + 教学顺序）。
> 每个类另有自己的总 md + `.h` + `.cpp` 教学 md（铁律 35）。
>
> **建立时间**：2026-11-02
> **教学状态**：✅ 已教完 P10.1~P10.9（3 结构体 + `IInteractableTarget` + `IInteractionInstigator` + `UInteractionStatics` + 3 任务 + `AGameplayAbilityTargetActor_Interact` + `UTenkaichiGameplayAbility_Interact`）。P10.9 `.cpp` 依赖 P12 PlayerController + P16 UI，已注释挂起（登记注释备忘录）。**P10 阶段全部完结 ✅**

---

## 一、本阶段要解决什么

P08 讲了"物品/背包"，P09 讲了"装备"。但还有一个基础能力没做：**角色靠近一个东西，能"交互"（按个键捡起来、开门、对话、坐椅子……）**。

P10 Interaction 就是**"交互系统"**，它回答：

1. **"什么东西能被交互？"** → `IInteractableTarget` 接口（可交互目标）。
2. **"谁在发起交互？"** → `IInteractionInstigator` 接口（交互发起者/仲裁者）。
3. **"交互有哪些选项？"** → `FInteractionOption`（一个交互选项，含文本/技能/UI 控件）。
4. **"怎么发现附近的交互物、怎么选中？"** → 三个 AbilityTask（`GrantNearbyInteraction` 附近扫描 + `WaitForInteractableTargets` 等待目标 + `SingleLineTrace` 射线追踪）。
5. **"交互技能本体"** → `ULyraGameplayAbility_Interact`（交互技能，扫描 + 展示 UI + 触发交互）。

一句话：**P10 = 一套"发现附近可交互物 → 生成交互选项 → 展示 UI → 触发交互"的完整机制**，是 GAS 技能 + 接口 + 异步任务三者的结合。

---

## 二、全景：9 个教学内容（16 文件）

| 编号 | 类 | 文件 | 作用 |
|------|------|------|------|
| P10.1 | `FInteractionQuery` + `FInteractionOption` + `FLyraInteractionDurationMessage` | 3×`.h`（无 .cpp） | 交互查询 / 交互选项 / 交互时长消息（3 个纯结构体） |
| P10.2 | `IInteractableTarget` | `.h`（无 .cpp） | 可交互目标接口（纯虚 `GatherInteractionOptions`） |
| P10.3 | `IInteractionInstigator` | `.h`（无 .cpp） | 交互发起者接口（纯虚 `ChooseBestInteractionOption`） |
| P10.4 | `UInteractionStatics` | `.h` + `.cpp` | 交互静态工具类（取 Actor / 找目标 / 追加目标） |
| P10.5 | `UAbilityTask_GrantNearbyInteraction` | `.h` + `.cpp` | 附近扫描任务（重叠检测 + 授予技能） |
| P10.6 | `UAbilityTask_WaitForInteractableTargets` | `.h` + `.cpp` | 等待目标任务（抽象，射线 + 选项筛选） |
| P10.7 | `UAbilityTask_WaitForInteractableTargets_SingleLineTrace` | `.h` + `.cpp` | 单射线追踪任务（继承 P10.6） |
| P10.8 | `AGameplayAbilityTargetActor_Interact` | `.h` + `.cpp` | 交互目标 Actor（继承引擎 TargetActor_Trace） |
| P10.9 | `ULyraGameplayAbility_Interact` | `.h` + `.cpp` | 交互技能本体（扫描 + UI 指示器 + 触发） |

> 命名注意（铁律 24）：大部分类**无 `Lyra` 前缀 → 保持原样**（`FInteractionQuery`、`IInteractableTarget`、`UInteractionStatics`、`UAbilityTask_*`、`AGameplayAbilityTargetActor_Interact`、`FLyraInteractionDurationMessage`）。**只有 `ULyraGameplayAbility_Interact` 有前缀 → `UTenkaichiGameplayAbility_Interact`**。

---

## 三、依赖分析（铁律 30/31）

```
P10.9 LyraGameplayAbility_Interact ── 依赖 P05 LyraGameplayAbility + P12 PlayerController + P16 UI(Indicator)
P10.8 GameplayAbilityTargetActor_Interact ── 纯引擎 GameplayAbilityTargetActor_Trace
P10.7 SingleLineTrace ── 依赖 P10.6 WaitForInteractableTargets
P10.6 WaitForInteractableTargets（抽象）── 依赖 P10.1 Option + P10.2 IInteractableTarget
P10.5 GrantNearbyInteraction ── 依赖 P00 CollisionChannels + P10.4 Statics + P10.2
P10.4 UInteractionStatics ── 依赖 P10.2 IInteractableTarget
P10.3 IInteractionInstigator ── 依赖 P10.1 Option
P10.2 IInteractableTarget ── 依赖 P10.1 Option + 前置 FInteractionQuery
P10.1 Query + Option + DurationMessage ── 纯引擎（最底）
```

**关键断点（⚠️）：**

| 断点 | 依赖 | 处理 |
|------|------|------|
| `LyraGameplayAbility_Interact.cpp` | `Player/LyraPlayerController.h`（P12）+ `UI/IndicatorSystem/*`（P16） | `.cpp` 挂起 P12+P16（铁律 33/34） |
| `AbilityTask_GrantNearbyInteraction.cpp` | `Physics/LyraCollisionChannels.h`（P00 ✅） | 已就位，不挂起 |

> **已就位 ✅**：P05 GAS 核心（`ULyraGameplayAbility`、`UAbilitySystemComponent`）、P00 `LyraCollisionChannels`（`Lyra_TraceChannel_Interaction`）。
> **待就位 ⬜**：P12 PlayerController、P16 UI IndicatorSystem（都只在 `LyraGameplayAbility_Interact.cpp` 里硬调用）。

---

## 四、教学顺序（自下而上，铁律 20/29/35）

| 序 | 编号 | 内容 | 为什么排这里 |
|----|------|------|-------------|
| 1 | **P10.1** | `FInteractionQuery` + `FInteractionOption` + `FLyraInteractionDurationMessage` | 3 个纯结构体，最底，被后面所有类依赖 |
| 2 | **P10.2** | `IInteractableTarget` | 依赖 Option，被 Statics/Task 依赖 |
| 3 | **P10.3** | `IInteractionInstigator` | 依赖 Option，接口 |
| 4 | **P10.4** | `UInteractionStatics` | 依赖 IInteractableTarget |
| 5 | **P10.5** | `UAbilityTask_GrantNearbyInteraction` | 依赖 Statics + CollisionChannels |
| 6 | **P10.6** | `UAbilityTask_WaitForInteractableTargets` | 依赖 Option + IInteractableTarget |
| 7 | **P10.7** | `UAbilityTask_WaitForInteractableTargets_SingleLineTrace` | 继承 P10.6 |
| 8 | **P10.8** | `AGameplayAbilityTargetActor_Interact` | 纯引擎，相对独立 |
| 9 | **P10.9** | `ULyraGameplayAbility_Interact` | 收口，依赖 P05 + P12 + P16（.cpp 挂起） |

---

## 五、核心难点预读

1. **接口双类结构**：`IInteractableTarget` / `IInteractionInstigator` 都是 `UINTERFACE` 外壳 + C++ 接口体（P08.8 `IPickupable` 同款）。
2. **`FInteractionOptionBuilder`**：一个**构造器辅助类**（在 `IInteractableTarget.h` 里），负责把"目标作用域"和"选项数组"打包，供 `GatherInteractionOptions` 往里填选项。
3. **AbilityTask 三件套**：`GrantNearbyInteraction`（附近重叠扫描 + 授予技能）、`WaitForInteractableTargets`（抽象，射线 + 选项筛选）、`SingleLineTrace`（单射线追踪，继承抽象类）。
4. **交互技能收尾**：`ULyraGameplayAbility_Interact` 在 `ActivateAbility` 里启动 `GrantNearbyInteraction` 任务，`UpdateInteractions` 里用 `ULyraIndicatorManagerComponent`（P16 UI）展示交互图标，`TriggerInteraction` 里触发目标技能。

---

## 六、命名对照（铁律 24）

| Lyra | Tenkaichi | 说明 |
|------|-----------|------|
| `FInteractionQuery` / `FInteractionOption` / `FLyraInteractionDurationMessage` | 同名 | **无前缀，保持原样** |
| `IInteractableTarget` / `IInteractionInstigator` | 同名 | **无前缀，保持原样** |
| `UInteractionStatics` | 同名 | **无前缀，保持原样** |
| `UAbilityTask_GrantNearbyInteraction` / `WaitForInteractableTargets` / `_SingleLineTrace` | 同名 | **无前缀，保持原样** |
| `AGameplayAbilityTargetActor_Interact` | 同名 | **无前缀，保持原样** |
| `ULyraGameplayAbility_Interact` | `UTenkaichiGameplayAbility_Interact` | ✅ **有前缀，换 Tenkaichi** |

---

## 七、教学节奏（铁律 20/29/35）

1. ✅ 本阶段总 md（现在这一步）
2. ✅ P10.1 三个结构体 ✅ → P10.2 `IInteractableTarget` ✅ → P10.3 `IInteractionInstigator` ✅ → P10.4 `UInteractionStatics` ✅ → P10.5 `UAbilityTask_GrantNearbyInteraction` ✅ → P10.6 `UAbilityTask_WaitForInteractableTargets` ✅ → P10.7 `UAbilityTask_WaitForInteractableTargets_SingleLineTrace` ✅ → P10.8 `AGameplayAbilityTargetActor_Interact` ✅ → P10.9 `UTenkaichiGameplayAbility_Interact`（待建）→ 逐个「总 md → `.h` → `.cpp`」

> ⚠️ 注意：
> - P10.1/P10.2/P10.3 是**纯 `.h`（无 `.cpp`）**，教学节奏简化为「总 md → 教 `.h`」（无 `.cpp` 步骤）。
> - 到 **P10.9 `ULyraGameplayAbility_Interact`** 时，`.cpp` 需挂起 P12 PlayerController + P16 UI，**登记注释备忘录**。

---

## 八、一句话结论

P10 Interaction 是"发现附近可交互物 → 生成选项 → 展示 UI → 触发交互"的完整机制：9 个教学内容（3 结构体 + 2 接口 + 1 工具 + 3 任务 + 1 目标 Actor + 1 交互技能）。依赖 P05 GAS + P00 CollisionChannels（已就位），`ULyraGameplayAbility_Interact.cpp` 挂起 P12+P16。大部分类无 `Lyra` 前缀保持原样，只有 `ULyraGameplayAbility_Interact` 换 `Tenkaichi`。