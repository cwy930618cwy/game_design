# 01 — 教程设计总纲（贴合你的 `Tenkaichi` 项目）

> **定位**：把 [00 教学路线](./00_教学开篇与路线.md) 的"大方向"落成**可执行的课时设计**。每一课都精确到：**在 `e:\ue5\game_design\code\Tenkaichi` 项目里建哪个文件、参考 Lyra 哪个源文件、验收标准是什么**。
>
> **一句话**：不是"读 Lyra"，而是"**在你自己的 `Tenkaichi` 工程里，用 Lyra 的写法，一课一课把游戏搭出来**"。

---

## 一、你现在的起点（`Tenkaichi` 项目盘点）

已确认 `e:\ue5\game_design\code\Tenkaichi` 当前状态（**2026-09-22 已重置为干净空白工程**，逐个文件核对）：

| 项 | 状态 | 说明 |
|----|------|------|
| 引擎 | ✅ UE **5.6** | `Tenkaichi.uproject` → `EngineAssociation: "5.6"` |
| 工程类型 | ✅ 空白 C++ Game 工程 | 单模块 `Tenkaichi`，Type = Runtime / Game |
| Enhanced Input | ✅ 已加 | `Tenkaichi.Build.cs` 已包含 `EnhancedInput` 模块 |
| GAS 依赖 | ❌ 未加 | `Tenkaichi.Build.cs` 有 `Core/CoreUObject/Engine/InputCore/EnhancedInput`；`.uproject` 仅启用 ModelingToolsEditorMode，未启用 GameplayAbilities 插件 |
| 属性集 | ❌ 未建 | 无任何 AttributeSet |
| ASC / Character / GameMode | ❌ 未建 | 无任何 GAS 相关类 |
| 输入 / 相机 / 动画 | ❌ 未建 | 后续课做 |

> **结论**：你现在站在**真正的"零"**——一个刚建好的空白 C++ 工程，**没有任何 GAS 痕迹**（但 Enhanced Input 已就绪）。本总纲据此排课——**第 02 课从"给工程加 GAS 依赖"这真正的第一步开始**，再建属性集、挂 ASC。

---

## 二、终态目标（我们要把 `Tenkaichi` 搭成什么样）

一个**最小可玩的 Lyra 风格射击游戏骨架**：

```
能跑能跳的角色（组件化）
  → 第三人称相机（LyraCameraMode 思路）
  → 输入（Enhanced Input，移动/视角/开火分派）
  → 一个开火技能（GAS：GA + 射线 + GE 扣血）
  → 血量归零死亡 + 重生
  → 一把武器（装备到角色）
  → 一个 HUD（血条 + 准星，CommonUI）
```

**注意**：这是"教学骨架"，追求**麻雀虽小五脏俱全**——把 Lyra 的核心范式（组件化 + 数据驱动 + GAS + Tag）走通一遍，而不是一上来就堆联机/插件。

---

## 三、课时设计（6 阶段 · 每课可验收）

> 每课统一格式：**目标 / 你要建的文件 / 参考 Lyra 源文件 / 关键概念 / 验收标准**。
> 文件路径均相对 `e:\ue5\game_design\code\Tenkaichi\Source\`。

### 阶段一：GAS 打通（让 Health 真正能被改）

#### 第 02 课 · 给角色挂上 ASC + 属性集 ⭐（你的下一课）
- **目标**：一个能 Spawn 的角色，身上有 ASC + `UTenkaichiAttributeSet`，能读出初始血量。
- **你要建的文件 / 改动**：
  - **第 0 步（真正的第一步）**：给工程加 GAS 依赖——`Tenkaichi.Build.cs` 加 `GameplayAbilities`/`GameplayTags`/`GameplayTasks` 三个模块，`.uproject` 启用 GameplayAbilities 插件（否则编译找不到模块）。
  - `AbilitySystem/Attributes/TenkaichiAttributeSet.h/.cpp`（**基类**，对应 Lyra `LyraAttributeSet`，不含属性，只有宏+工具函数）
  - `AbilitySystem/Attributes/TenkaichiHealthSet.h/.cpp`（**血量集**，对应 Lyra `LyraHealthSet`，继承基类，定义 `Health` 属性）
  - `AbilitySystem/TenkaichiAbilitySystemComponent.h/.cpp`（继承 `UAbilitySystemComponent`）
  - `Character/TenkaichiCharacter.h/.cpp`（继承 `ACharacter`，先不继承 Lyra）
  - 在角色构造里 `CreateDefaultSubobject` 挂 ASC + 属性集
- **参考 Lyra 源文件**：
  - `Character/LyraCharacterWithAbilities.h/.cpp`（角色 + ASC 的挂法、`InitAbilityActorInfo`）
  - `AbilitySystem/LyraAbilitySystemComponent.h`（ASC 自定义扩展）
  - `AbilitySystem/LyraAbilitySet.h`（属性集怎么被授予）
- **关键概念**：GAS 依赖怎么加、ASC 挂在哪（角色 or PlayerState）、`InitAbilityActorInfo`、属性初始化（`InitHealth`）。
- **验收**：PIE 里 Spawn 角色，`GetHealth()` 返回你设的初值（如 100）。

#### 第 03 课 · 第一个技能：一次伤害
- **目标**：按一个键，对目标造成一次伤害，血量下降。
- **你要建的文件**：
  - `AbilitySystem/Abilities/GA_DealDamage.h/.cpp`（继承 `UGameplayAbility`）
  - `AbilitySystem/Effects/GE_Damage.h`（继承 `UGameplayEffect`，配 `DamageExecution`）
  - `AbilitySystem/Executions/TenkaichiDamageExecution.h/.cpp`（`FGameplayEffectExecutionCalculation`，真正改 Health）
- **参考 Lyra 源文件**：
  - `AbilitySystem/Abilities/LyraGameplayAbility.h/.cpp`（技能基类，看 `ActivateAbility` 怎么调 Super + 留 `K2_` 钩子）
  - `AbilitySystem/Effects/LyraGameplayEffect.h`（GE 基类）
  - `AbilitySystem/LyraDamageExecution.cpp`（伤害计算执行）
  - `Character/LyraHealthComponent.cpp`（`OnOutOfHealth` 死亡判定思路）
- **关键概念**：GA 激活流程、GE 是"改属性的唯一途径"、Execution 里 `SetAttributeBaseValue`、Attribute Capture。
- **验收**：触发技能 → 目标 `Health` 从 100 掉到某值，日志打印扣血量。

---

### 阶段二：角色动起来（输入 + 相机 + 移动）

#### 第 04 课 · Enhanced Input 接入
- **目标**：WASD 移动、鼠标转视角。
- **你要建的文件**：
  - `Input/TenkaichiInputConfig.h`（`UDataAsset`，映射 InputTag→InputAction，仿 Lyra）
  - `Input/TenkaichiInputComponent.h/.cpp`（`ULyraInputComponent` 思路，`BindNativeAction`）
  - 角色上加 `SetupPlayerInputComponent` 绑定
- **参考 Lyra 源文件**：
  - `Input/LyraInputConfig.h`（翻译官字典）
  - `Input/LyraInputComponent.h/.cpp`（绑定模板）
  - `Character/LyraHeroComponent.cpp` 的 `InitializePlayerInput` / `Input_Move` / `Input_LookMouse`
- **关键概念**：InputAction→GameplayTag 映射、`AddMovementInput`、镜头相对移动（取 `ControlRotation.Yaw`）。
- **验收**：进 PIE 角色能走能转视角。

> 📌 概念可先读 [人物介绍/09_输入故事](../../人物介绍/09_输入_按键到角色运动的故事.md)。

#### 第 05 课 · 组件化重构角色
- **目标**：把输入/相机逻辑从角色里**拆成 Component**（Lyra 的灵魂）。
- **你要建的文件**：
  - `Character/TenkaichiHeroComponent.h/.cpp`（仿 `ULyraHeroComponent`，管输入+相机）
  - `Character/TenkaichiPawnExtensionComponent.h/.cpp`（仿 `LyraPawnExtensionComponent`，协调初始化）
- **参考 Lyra 源文件**：
  - `Character/LyraHeroComponent.h/.cpp`
  - `Character/LyraPawnExtensionComponent.h/.cpp`（InitState 四阶段）
  - `Character/LyraCameraComponent.h/.cpp`
- **关键概念**：功能拆组件、InitState（Spawned→DataAvailable→DataInitialized→GameplayReady）、组件间协调。
- **验收**：角色逻辑分散到组件，运行行为不变，但结构"Lyra 化"了。

#### 第 06 课 · 第三人称相机
- **目标**：标准的第三人称跟随相机 + 弹簧臂。
- **你要建的文件**：`Camera/TenkaichiCameraMode.h/.cpp`（仿 `ULyraCameraMode_ThirdPerson`）
- **参考 Lyra 源文件**：`Camera/LyraCameraMode.h`、`Camera/LyraCameraMode_ThirdPerson.h/.cpp`、`Camera/LyraPlayerCameraManager.h`
- **关键概念**：CameraMode 数据驱动、CameraManager 驱动、弹簧臂避障。
- **验收**：角色移动时相机平滑跟随、鼠标自由转视角。

---

### 阶段三：战斗成型（把第 03 课升级成"开火"）

#### 第 07 课 · 开火技能（射线版）
- **目标**：按开火键 → 射线检测 → 命中则扣血 + 播特效。
- **你要建的文件**：
  - `AbilitySystem/Abilities/GA_Fire.h/.cpp`（仿 `ULyraGameplayAbility_RangedWeapon`）
  - 蓝图 `GA_Fire`（事件图表串 Ability Task：PlayMontageAndWait → 射线 → ExecuteGameplayCue / ApplyGE）
- **参考 Lyra 源文件**：
  - `Weapons/LyraRangedWeaponInstance.h/.cpp`（远程武器射线逻辑）
  - `AbilitySystem/Abilities/LyraGameplayAbility_RangedWeapon`（远程技能基类）
  - `Weapons/LyraWeaponStateComponent.h`（武器状态）
- **关键概念**：Ability Task 异步流程、射线检测、`HasAuthority` 服务器算伤害、GameplayCue 播表现。
- **验收**：开火 → 命中假人 → 掉血 + 命中特效。

> 📌 蓝图拆解可对照 [人物介绍/07_拆解GA_Weapon_Fire](../../人物介绍/07_实战_拆解真实的GA_Weapon_Fire蓝图.md)。

#### 第 08 课 · 死亡与重生
- **目标**：血量归零 → 死亡 → 延迟重生。
- **你要建的文件**：`AbilitySystem/Abilities/GA_Death.h/.cpp`、重生逻辑（GameMode 里）
- **参考 Lyra 源文件**：`Character/LyraHealthComponent.cpp`（`OnOutOfHealth`）、`GameModes/LyraGameMode.cpp`（重生调度）、`AbilitySystem/Abilities/LyraGameplayAbility_Death`
- **关键概念**：死亡是"事件驱动技能"（血量归0→发 GameplayEvent.Death→死亡技能激活）、重生流程。
- **验收**：打空血 → 角色死亡表现 → 几秒后在出生点重生、满血。

---

### 阶段四：装备与数据驱动

#### 第 09 课 · 武器作为"装备"
- **目标**：武器用 DataAsset 定义，装备到角色手上生成。
- **你要建的文件**：
  - `Equipment/TenkaichiEquipmentDefinition.h`（仿 `ULyraEquipmentDefinition`，配要 Spawn 的 Actor + 授予的技能）
  - `Equipment/TenkaichiEquipmentManagerComponent.h/.cpp`
- **参考 Lyra 源文件**：`Equipment/LyraEquipmentDefinition.h`、`Equipment/LyraEquipmentInstance.h`、`Equipment/LyraEquipmentManagerComponent.h/.cpp`
- **关键概念**：Definition→Instance 模式、装备授予 GA、`ActorsToSpawn` 生成武器实体。
- **验收**：给角色"装备"一把枪 → 手上出现枪模型 + 获得开火技能。

#### 第 10 课 · 背包/物品（Fragment 组合）
- **目标**：物品用 Fragment 组合（Lyra 的招牌模式）。
- **你要建的文件**：`Inventory/TenkaichiInventoryItemDefinition.h`（仿 `ULyraInventoryItemDefinition` + Fragment）
- **参考 Lyra 源文件**：`Inventory/LyraInventoryItemDefinition.h`、`Inventory/LyraInventoryItemInstance.h`、`Inventory/InventoryFragment_*.h`
- **关键概念**：`DefaultToInstanced` + `EditInlineNew` Fragment 组合、Instance 运行时状态。
- **验收**：一个"治疗药水"物品，由多个 Fragment 拼出（图标 + 使用效果）。

---

### 阶段五：UI（CommonUI + LyraUI 三层）

#### 第 11 课 · HUD（血条 + 准星）
- **目标**：屏幕左下血条 + 中心准星，数据实时刷新。
- **你要建的文件**：
  - `UI/TenkaichiHUDLayout.h/.cpp`（继承 `ULyraActivatableWidget` 思路，用 CommonUI 层栈）
  - `UI/TenkaichiHealthBarWidget.h/.cpp`（绑定 ASC 的 Health 属性变化）
- **参考 Lyra 源文件**：`UI/LyraActivatableWidget.h`、`UI/LyraHUDLayout.h/.cpp`、`UI/Basic/MaterialProgressBar.h`、`UI/Weapons/LyraReticleWidgetBase.h`
- **关键概念**：CommonUI 层栈（`PushContentToLayer`）、属性变化委托绑定 UI、GameplayTag 分层。
- **验收**：掉血时血条实时减少；开火时显示准星。

> 📌 三层关系先读 [人物介绍/10_UI三层](../../人物介绍/10_UI三层_UMG_CommonUI_LyraUI人物谱.md)、目录看 [14_UI目录](../../Lyra_项目分析/14_UI目录/01_UI目录总览_文件清单.md)。

---

### 阶段六：工程化（可选进阶）

#### 第 12 课 · GameFeature 插件化 + Experience
- **目标**：把"一套玩法"抽成 Experience 配置 + GameFeature 插件。
- **参考 Lyra 源文件**：`GameModes/LyraExperienceDefinition.h`、`GameFeatures/GameFeatureAction_AddAbilities.h`、`GameFeatures/GameFeatureAction_AddWidget.h`
- **验收**：换一个 Experience 资产就能切换玩法/装备配置，无需改代码。

#### 第 13 课 · 多人联机基础
- **目标**：把 ASC/属性/开火跑通网络复制（Server RPC + 复制）。
- **参考 Lyra 源文件**：`AbilitySystem/LyraAbilitySystemComponent.cpp`（复制）、`Inventory/LyraInventoryManagerComponent.cpp`（`FFastArraySerializer`）
- **验收**：两个客户端，一人开火另一人看到掉血同步。

---

## 四、每课的标准结构（写正文时统一遵守）

每课正文建议按这个模板写，方便你"照着做"：

```
# NN — 课名

> 一句话：本课要做出什么

## 一、本课目标 & 验收标准
（学完你能看到什么）

## 二、动手前：要理解的概念
（先讲清楚再动手，别硬抄）

## 三、第一步：建文件 / 写代码
（贴完整可编译代码，含注释）

## 四、第二步：配置（蓝图/DataAsset/输入资产）
（编辑器里怎么点）

## 五、运行 & 验收
（PIE 里看到什么算成功）

## 六、常见坑
（编译错/空指针/初始化顺序）

## 七、对照 Lyra
（你的写法 vs Lyra 的写法，差在哪、为什么）
```

---

## 五、教程资源映射速查（Lyra 源文件 → 教哪一课）

| Lyra 源文件（`Source/LyraGame/`） | 教哪课 | 学什么 |
|-----------------------------------|--------|--------|
| `Character/LyraCharacterWithAbilities.h` | 02 | 角色挂 ASC |
| `AbilitySystem/LyraAbilitySystemComponent.h` | 02 | 自定义 ASC |
| `AbilitySystem/LyraAbilitySet.h` | 02 | 属性集授予 |
| `AbilitySystem/Abilities/LyraGameplayAbility.*` | 03/07 | 技能基类 |
| `AbilitySystem/LyraDamageExecution.cpp` | 03 | 伤害执行 |
| `Character/LyraHealthComponent.cpp` | 03/08 | 血量/死亡 |
| `Input/LyraInputConfig.h` / `LyraInputComponent.*` | 04 | 输入映射 |
| `Character/LyraHeroComponent.cpp` | 04/05 | 输入分派 |
| `Character/LyraPawnExtensionComponent.*` | 05 | 初始化协调 |
| `Camera/LyraCameraMode_ThirdPerson.*` | 06 | 第三人称相机 |
| `Weapons/LyraRangedWeaponInstance.*` | 07 | 射线开火 |
| `GameModes/LyraGameMode.cpp` | 08 | 重生 |
| `Equipment/LyraEquipmentDefinition.h` | 09 | 装备定义 |
| `Inventory/LyraInventoryItemDefinition.h` | 10 | Fragment 物品 |
| `UI/LyraHUDLayout.*` / `UI/Basic/MaterialProgressBar.h` | 11 | HUD |
| `GameFeatures/GameFeatureAction_Add*.h` | 12 | 插件化 |

---

## 六、下一步

**你的下一课 = 第 02 课（给角色挂 ASC + 属性集）**——这是你 `Tenkaichi` 项目当前进度正好的下一步。

告诉我"**开始第 02 课**"，我就按第四节的模板，写出完整可编译的代码 + 配置步骤 + 验收方法，直接在你的 `e:\ue5\game_design\code\Tenkaichi` 工程里落地。

> 也可以指定别的课，或让我调整路线（比如你想先做相机/先做武器）。
