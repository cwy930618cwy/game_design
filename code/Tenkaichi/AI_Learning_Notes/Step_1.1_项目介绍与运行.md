# Step 1.1：项目介绍与运行

> 对应教程章节：`001_项目介绍` + `002_源码速览` + `003_蓝图速览`
> 学习周期：第 1-2 天 | 阶段：第一阶段·全局认知
> **目标**：建立对 Lyra 整体架构、设计理念、工程结构的宏观理解，能在编辑器中跑起来并看懂目录。

---

## 一、Lyra 是什么？（来自 001 章）

### 1.1 官方定位
**LyraStarterGame** 是 Epic Games 官方提供的 **UE5 示例游戏工程**，用于展示 UE5 引擎的最佳实践和推荐架构。

它不是"一个能玩的游戏"，而是"**一份可运行的架构教科书**"。

### 1.2 三种游戏模式
Lyra 内置了三种典型玩法，覆盖不同游戏类型：

| 模式 | 英文名 | 类型 | 特点 |
|------|--------|------|------|
| 前端大厅 | **FrontEnd** | 菜单/大厅 | 无战斗，纯 UI 交互，展示 CommonUI 框架 |
| 团队死斗 | **Elimination** | 第三人称射击 (TPS) | ShooterCore GameFeature，经典 TDM 玩法 |
| 占点控制 | **Control** | 俯视角射击 | TopDownArena GameFeature，类似炸弹人视角 |

> 💡 **关键洞察**：这三种模式共享同一套底层框架（Experience + GAS + CommonUI），只是通过不同的 **GameFeature 插件** 动态加载不同的玩法逻辑。这就是 Lyra 最核心的设计思想——**数据驱动 + 模块化**。

### 1.3 技术栈清单
教程明确列出的关键技术：
- **GAS** (Gameplay Ability System) — 技能/属性/效果系统
- **CommonUI** — 跨平台 UI 框架
- **Enhanced Input** — 增强输入系统
- **GameFeatures 插件机制** — 运行时动态加载玩法模块
- **Modular Gameplay Actors** — 模块化角色组件
- **ReplicationGraph** — 网络复制优化
- **MetaSound** — 下一代音频系统

---

## 二、源码结构速览（来自 002 章）

### 2.1 顶层目录
```
LyraStarterGame/
├── Source/              ← C++ 源码（重点！）
│   ├── LyraGame/        ← Runtime 主模块（约 448 文件）
│   └── LyraEditor/      ← Editor 扩展模块（约 26 文件）
├── Content/             ← 美术/音频/蓝图资产（重点！）
├── Plugins/             ← GameFeature + 功能插件
├── Config/              ← 引擎/项目配置 (.ini)
├── Build/               ← 构建脚本
├── Platforms/           ← 各平台特定配置
├── LyraStarterGame.uproject  ← 项目描述文件
└── LyraStarterGame.sln       ← VS 解决方案
```

### 2.2 Source/LyraGame 核心子目录（必背）

这是整个项目的**代码心脏**，共 27 个子目录。按职责分类记忆：

#### 🎮 游戏框架层
| 目录 | 职责 | 关键类 |
|------|------|--------|
| `GameModes/` | 游戏模式/状态/玩家控制器等 | `ALyraGameMode`, `ALyraGameState`, `ALyraPlayerController`, `ALyraPlayerState`, `ALyraPawn`, `ALyraCharacter` |
| `System/` | 全局系统组件 | `ULyraAssetManager`, `ULyraGameInstance`, `ULyraGameEngine`, `ULyraExperienceManagerComponent` |
| `Settings/` | 游戏设置相关 | `ULyraSettingsLocal`, `ULyraSettingsShared`, `ULyraGameUserSettings` |

#### 🤖 角色与能力层
| 目录 | 职责 | 关键类 |
|------|------|--------|
| `Character/` | 角色组件化设计 | `ULyraPawnExtensionComponent`, `ULyraHeroComponent`, `ULyraHealthComponent`, `ULyraCameraComponent` |
| `AbilitySystem/` | GAS 集成 | `ULyraAbilitySystemComponent`, `ULyraGameplayAbility`, `LyraAttributeSet` 系列 |
| `Equipment/` | 装备系统 | `ULyraEquipmentDefinition`, `ULyraEquipmentInstance`, `ULyraEquipmentManagerComponent` |
| `Inventory/` | 背包系统 | `ULyraInventoryManagerComponent`, `ULyraInventoryItemDefinition`, Fragment 体系 |
| `Weapons/` | 武器系统 | `ULyraRangedWeaponInstance`, `ALyraWeaponSpawner` |

#### 🎨 UI 与输入层
| 目录 | 职责 | 关键类 |
|------|------|--------|
| `UI/` | CommonUI 界面 | `ULyraHUDLayout`, `ULyraTabListWidgetBase`, `GameSetting` 系列 |
| `Input/` | Enhanced Input 集成 | `ULyraInputConfig`, `ULyraInputComponent`, `ULyraInputModifiers` |
| `Camera/` | 相机系统 | `ULyraCameraComponent`, `ULyraCameraMode` |
| `Animation/` | 动画相关 | AnimInstance 扩展 |

#### 🔧 工具与辅助层
| 目录 | 职责 | 关键类 |
|------|------|--------|
| `Interaction/` | 交互系统 | `ULyraInteractionOptionComponent` |
| `Feedback/` | 反馈效果（数字飘字等） | `ULyraNumberPopComponent` |
| `Teams/` | 队伍系统 | Team Subsystem |
| `Messages/` | 消息路由 | GameplayMessage 相关 |
| `Performance/` | 性能优化 | Significance 相关 |
| `Physics/` | 物理材质 | `LyraPhysicalMaterial` |
| `Audio/` | 音频管理 | Music Component |
| `Development/` | 开发调试 | Cheat Manager, Debug 工具 |
| `Tests/` | 自动化测试 | Gauntlet 测试用例 |

### 2.3 LyraEditor 模块
只有约 26 个文件，主要负责：
- **资产验证器** (Validators) — 检查资产是否符合规范
- **内容浏览器扩展** — 右键菜单、自定义显示
- **编辑器工具** — Actor Factory、Content Bundle 等

> 💡 **为什么分离？** Editor 专用代码不能打进最终游戏包，所以单独放一个模块，只在编辑器下编译。

---

## 三、蓝图/Content 结构速览（来自 003 章）

### 3.1 Content 顶层分类
```
Content/
├── Audio/                    ← MetaSound + 音效总线 + 衰减预设
├── Characters/               ← 角色骨骼网格体 + 换装系统
├── Core/                     ← 核心蓝图（GameData, GameInstance, InputConfig）
├── Effects/                  ← Niagara 粒子特效
├── Equipment/                ← 装备定义（DataAsset）
├── Feedback/                 ← 命中反馈、伤害数字
├── FrontEnd/                 ← 前端大厅 UI
├── GameModes/                ← 各模式的 Experience Definition
├── HUD/                      ← 游戏内 HUD
├── Input/                    ← Input Action / Mapping Context
├── Inventory/                ← 物品定义
├── Maps/                     ← 关卡地图
├── Settings/                 ← 设置界面
├── UI/                       ← CommonUI 控件
├── Weapons/                  ← 武器蓝图 + 子弹
└── ...
```

### 3.2 六大核心框架（蓝图视角）

教程把 Content 归纳为 6 大板块，便于理解：

| 板块 | 核心内容 | 你要记住的点 |
|------|---------|-------------|
| **1. 核心框架** | `DefaultGameData`（硬编码默认资产）、`GameInstance`（全局状态）、`InputConfig`（输入映射） | DataAsset 驱动一切 |
| **2. 角色系统** | `CatMesh` 动态换装、物理材质 `WeakSpot`（弱点触发暴击） | 组件化拼装角色 |
| **3. 能力与交互** | `GA_` 前缀的 GAS 蓝图、`AnimNotify` 触发攻击判定 | 逻辑在 C++，表现调 BP |
| **4. 资源管理** | MetaSound 参数化音频、Submix 环境混响、材质函数复用 | 数据驱动音频/FX |
| **5. 插件扩展** | ShooterCore（射击核心）、Experience（动态挂载）、CommonUI（堆栈式 UI） | GameFeature 即插即用 |
| **6. 工具与测试** | Editor Utilities、几何体原型工具、回放系统 | 开发者效率工具 |

---

## 四、关键设计理念（必须理解）

### 4.1 数据驱动 (Data-Driven)
Lyra 几乎不用硬编码，一切行为由 **DataAsset** 决定：
- `ExperienceDefinition` → 决定加载哪些 GameFeature
- `PawnData` → 决定角色的能力集、输入配置、外观
- `AbilitySet` → 决定一个角色拥有什么 GA/GE/AttributeSet
- `InputConfig` → 决定按键映射到哪个 InputTag

> **一句话总结**：改 DataAsset = 改游戏玩法，不用动 C++ 代码。

### 4.2 组件化 (Component-Based)
角色不是一个大类，而是**一堆组件的组合**：
```
ALyraCharacter (空壳)
  ├── ULyraPawnExtensionComponent  ← 协调所有组件初始化顺序
  ├── ULyraHeroComponent           ← 绑定输入 + 相机
  ├── ULyraHealthComponent         ← 生命值 + 死亡处理
  ├── ULyraCameraComponent         ← 相机模式管理
  ├── ULyraAbilitySystemComponent  ← GAS（实际在 PlayerState 上）
  └── ...其他业务组件
```

### 4.3 模块化 (Module-Based via GameFeature)
玩法不是写死的，而是通过 **GameFeature 插件** 动态加载：
```
基础工程（LyraGame）
  └── 运行时根据 ExperienceDefinition 决定加载：
      ├── ShooterCore    → 射击玩法
      ├── TopDownArena   → 俯视角玩法
      └── ShooterExplorer → 探索玩法
```

每个 GameFeature 可以包含自己的：
- C++ 代码（Source）
- 蓝图资产（Content）
- 配置（Config）

并且可以在运行时**热插拔**（启用/禁用）。

---

## 五、动手实践任务

### ✅ 任务 1：下载并运行 Lyra（如果还没做）
1. 从 Epic 商城下载 **Lyra Sample Project**（确保引擎版本 5.6.1）
2. 打开 `LyraStarterGame.uproject`
3. 点击 **Compile** 编译 C++ 代码
4. 打开任意地图（如 `L_ShooterGame_Main`）
5. 点击 **Play (PIE)** 运行

### ✅ 任务 2：切换三种模式
在编辑器中：
1. 打开 `Content/GameModes/` 找到三个 Experience Definition
2. 分别双击打开，观察它们的 GameFeature 配置差异
3. 通过控制台命令或修改 DefaultGameMode 切换模式运行

### ✅ 任务 3：画一张架构脑图
用你喜欢的工具（XMind / draw.io / 手绘），画出：
```
Lyra 工程
├── Source/
│   ├── LyraGame/ (27 子目录，列出你记住的分类)
│   └── LyraEditor/
├── Plugins/ (5 个 GameFeature + N 个功能插件)
├── Content/ (6 大板块)
└── 核心设计：数据驱动 + 组件化 + 模块化
```

### ✅ 任务 4：浏览 Content 目录
在 Content Browser 里按以下顺序浏览一遍，建立感性认识：
1. `Content/Core/` — 看 DataAsset 长什么样
2. `Content/GameModes/` — 看 Experience Definition 的结构
3. `Content/Characters/` — 看 CatMesh 换装系统
4. `Content/Weapons/` — 看武器定义
5. `Content/UI/` — 看 CommonUI 控件层级

---

## 六、思考题（加深理解）

1. **为什么 Lyra 要把 ASC 放在 PlayerState 而不是 Character 上？**
   > 提示：考虑网络同步、生命周期、分屏支持

2. **如果我要新增一个"潜行模式"，需要创建哪些东西？**
   > 提示：Experience Definition → GameFeature 插件 → PawnData → AbilitySet

3. **DataAsset 和 DataTable 有什么区别？Lyra 为什么偏爱 DataAsset？**
   > 提示：引用资产的能力、编辑器体验、类型安全

4. **GameFeature 插件和普通插件有什么区别？**
   > 提示：ExplicitlyLoaded、运行时启用/禁用、Mounting 机制

---

## 七、下一步

完成以上 4 个实践任务后，进入 **Step 1.2：工程创建与构建基础**（Target 文件 / Build.cs / 日志类别）。

有任何问题随时问我，也可以先记下来等到 QA 环节一起讨论。
