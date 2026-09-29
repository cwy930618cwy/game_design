# Lyra 四大核心概念：ModularGameplay / GameMode / Experience / GameFeature

> **定位**：讲清 Lyra（也是 UE5 现代游戏）架构里最核心的四个概念，以及它们**怎么串起来**构成一局游戏。这是理解 Lyra "模块化游戏框架"的总纲。
>
> **关联**：[01_Lyra项目模块划分](./01_Lyra项目模块划分_LyraGame与LyraEditor.md)
>
> **一句话**：Lyra 把"一局游戏"拆成四层——**GameFeature**（可插拔功能插件）往 **ModularGameplay**（模块化玩法框架）里插，由 **Experience**（体验定义，数据资产）描述"这局玩什么"，最终交给 **GameMode**（游戏规则大脑）执行。四者协作，做到"改玩法不改核心代码、功能即插即用"。

---

## 一、先用一个比喻建立整体印象

把"做一局游戏"想象成**办一场活动**：

| 概念 | 活动比喻 | 一句话职责 |
|------|---------|-----------|
| **GameFeature** | 一个个**外请的表演节目**（魔术、乐队、杂技） | 可插拔的功能插件，按需启用/关闭 |
| **ModularGameplay** | 舞台的**模块化接口系统**（标准插座、挂载点） | 让节目能"即插即用"的框架 |
| **Experience** | 这场活动的**流程表/策划案**（先乐队后魔术） | 数据资产，描述"这局玩什么、怎么赢" |
| **GameMode** | 活动的**总导演**（按流程表执行、判胜负） | 游戏规则大脑，执行逻辑 |

> **核心思想**：节目（GameFeature）是**外请的、可替换的**；舞台接口（ModularGameplay）是**固定的标准**；流程表（Experience）是**数据，能改**；导演（GameMode）是**执行者**。改流程表就能换玩法，加节目不用改舞台——这就是模块化的威力。

---

## 二、逐个讲清楚

### ① GameFeature —— 可插拔的功能插件（最外层）

**是什么**：UE5 的 **Game Feature Plugin**（游戏功能插件）。把一块完整功能（比如"射击模式""赛车模式""某个特殊角色"）打包成一个**独立插件**，可以**运行时动态启用/禁用**。

**关键特性**：
- **独立插件**：一个 GameFeature 是一个 `.uplugin`，代码/资源自成一包。
- **按需启用**：通过配置（`DefaultGame.ini` 或命令行）决定这局要不要它。
- **热插拔**：理想情况下，游戏运行中也能开关（如 DLC、活动模式）。

**Lyra 里的例子**：
```
Plugins/GameFeatures/
├── ShooterMaps/          ← 射击地图相关功能
├── TopDownArena/         ← 俯视竞技场模式
├── LyraExampleGame/      ← 示例玩法
└── ...
```

> **为什么重要**：传统游戏加个新模式要改核心代码、重新编译整个项目。GameFeature 让你**新增一个插件**就加了一个模式，不碰核心，编译也快（只编改动的插件）。这是"大型游戏持续扩展"的基础设施。

---

### ② ModularGameplay —— 模块化玩法框架（中间层，插座标准）

**是什么**：UE5 的 **ModularGameplay** 框架（引擎插件 `Runtime/ModularGameplay`）。它提供一套**标准化的"接口/挂载点"机制**，让 GameFeature 能"即插即用"地往游戏里加东西。

**核心机制**：
- **Modular Gameplay Actions**：定义"往哪加、加什么"的标准动作（给 Pawn 加组件、加能力、加输入映射等）。
- **挂载点（Extension Points）**：游戏的各个部位（角色、控制器、GameMode 等）暴露标准接口，GameFeature 按接口插入。
- **解耦**：核心代码不知道具体 GameFeature 的存在，GameFeature 也不知道彼此——都只认 ModularGameplay 定的标准。

> **类比**：ModularGameplay 是"舞台的标准插座"（统一的电压、接口规格）。任何符合规格的节目（GameFeature）插上就能用，不用每次改舞台布线。**它是 GameFeature 能"即插即用"的前提。**

**和 GameFeature 的关系**：
```
GameFeature 是"内容"（具体功能）
ModularGameplay 是"机制"（让内容能插进来的规则）
   ↓
GameFeature 通过 ModularGameplay 定义的接口，把功能注入游戏
```

---

### ③ Experience —— 体验定义（数据层，流程表）

**是什么**：Lyra 特有的核心概念——一个 **`ULyraExperienceDefinition` 数据资产**，用数据描述"**这一局游戏玩什么、怎么赢、用什么规则**"。

**它定义什么**：
- 用哪个 **GameFeature**（这局启用哪些插件）
- 用哪个 **GameMode**（这局用什么规则）
- 玩家用什么 **Pawn/角色**、什么**输入**、什么**能力**
- 胜利条件、队伍设置、加载流程等

**关键特性**：
- **纯数据**（DataAsset）：不改代码，改资产就能换玩法。
- **可复用/组合**：多个 Experience 共享同一套底层系统。
- **驱动加载**：进入一局游戏时，先加载 Experience，它再拉起对应的 GameFeature 和 GameMode。

**Lyra 里的例子**：
```
每个游戏模式（消除/射击/竞技场）对应一个 Experience 资产
   ↓ 玩家选模式
加载对应的 Experience
   ↓ Experience 指定
用哪些 GameFeature + 哪个 GameMode + 什么角色/输入
```

> **类比**：Experience 是"活动流程表"。同一批节目（GameFeature）和同一个导演（GameMode），流程表不同，办出来的活动（玩法）就完全不同。**改流程表 = 换玩法，不用动代码。**

---

### ④ GameMode —— 游戏规则大脑（执行层）

**是什么**：UE 经典的 **GameMode**（`ALyraGameMode`），一局游戏的**规则大脑**——管"这局游戏的规则、玩家进出、胜负判定、重生逻辑"。

**它负责**：
- 玩家登录/登出、生成 PlayerController/Pawn
- 胜负条件判定
- 重生、计分、队伍管理
- 配合 Experience 提供的配置执行规则

**在 Lyra 里的角色**：
- GameMode 是**执行者**——它读取 Experience 给的配置，执行具体规则。
- 它**不再是"什么都管"的上帝类**（传统 UE 里 GameMode 常被塞满逻辑）——Lyra 把很多职责**下放**给 Experience + GameFeature + 各种 Component/Subsystem。

> **类比**：GameMode 是"活动总导演"。它不亲自表演（那是 GameFeature 的活），而是**按流程表（Experience）协调、执行规则、判胜负**。

---

## 三、四者怎么串起来（完整流程）★

这是最重要的部分——四个概念不是孤立的，而是**一条链**：

```
【玩家选了一个游戏模式，比如"射击模式"】
        ↓
① 加载对应的 Experience（数据资产）
   "射击模式"的 Experience 里写着：
     - 启用哪些 GameFeature（ShooterMaps 等）
     - 用哪个 GameMode
     - 玩家用什么角色/输入/能力
        ↓
② Experience 触发启用 GameFeature
   通过配置拉起 ShooterMaps 等插件
        ↓
③ GameFeature 通过 ModularGameplay 注入功能
   把射击相关的组件/能力/输入，按标准接口插进游戏
        ↓
④ GameMode 按 Experience 的配置执行规则
   生成玩家、管胜负、判得分
        ↓
【一局"射击模式"游戏跑起来了】
```

**换玩法时**：
```
想换成"俯视竞技场"模式？
   ↓ 不用改代码
换一份 Experience 资产（它指定了 TopDownArena 的 GameFeature + 对应 GameMode）
   ↓
新的 GameFeature 被启用，新的规则执行
   ↓
玩法变了，核心代码一行没动
```

> **这就是模块化的终极目标**：**数据驱动玩法（Experience）+ 功能即插即用（GameFeature via ModularGameplay）+ 规则执行（GameMode）**。策划改数据就能出新玩法，程序加功能只需写独立插件。

---

## 四、一张表对比四者

| | GameFeature | ModularGameplay | Experience | GameMode |
|---|---|---|---|---|
| **层次** | 功能内容（最外层） | 框架机制（中间层） | 数据定义（数据层） | 规则执行（执行层） |
| **是什么** | 可插拔功能插件 | 模块化接口框架 | 数据资产 | 游戏规则类 |
| **比喻** | 外请表演节目 | 舞台标准插座 | 活动流程表 | 总导演 |
| **可改性** | 增删插件 | 一般不动（引擎级） | 改数据换玩法 | 定规则 |
| **何时介入** | 被 Experience 启用时 | 一直在（框架底座） | 进入一局游戏时 | 一局游戏全程 |
| **典型例子** | ShooterMaps 插件 | ModularGameplay 引擎插件 | 各模式的 Experience 资产 | `ALyraGameMode` |

---

## 五、常见误区

| 误区 | 正确理解 |
|------|---------|
| "GameMode 就是全部玩法" | ❌ Lyra 把玩法拆给 Experience + GameFeature，GameMode 只是执行规则 |
| "GameFeature 和 ModularGameplay 是一回事" | ❌ 前者是内容（插件），后者是让内容能插进来的机制（框架） |
| "Experience 是代码" | ❌ 是数据资产（DataAsset），改它不用改代码 |
| "加个新模式要改核心代码" | ❌ 加个 GameFeature 插件 + 配个 Experience 就行 |
| "Experience 和 GameMode 重复" | ❌ Experience 是"数据描述"，GameMode 是"执行逻辑"，分工不同 |
| "GameFeature 运行时不能关" | 可以，GameFeature 支持动态启用/禁用（这是它的核心特性） |

---

## 六、和你之前学的模块划分什么关系

- **`LyraGame`**（第 01 篇）：游戏本体模块，**GameMode、Experience 定义、ModularGameplay 的接入逻辑**等核心框架代码在这里。
- **GameFeature 插件**：在 `Plugins/GameFeatures/` 下，是**独立于 LyraGame 的插件模块**（每个一个 `.uplugin`）。
- **`LyraEditor`**（第 02 篇）：编辑器工具，和这四大概念的运行无关。

```
LyraGame（核心框架：GameMode/Experience定义/接入逻辑）
    ↓ 运行时按 Experience 启用
Plugins/GameFeatures/（一个个 GameFeature 插件）
    ↓ 通过
ModularGameplay（引擎框架，让插件即插即用）
```

> 所以四大概念跨越了"LyraGame 核心"和"GameFeature 插件"两个代码层——这正是 Lyra 架构的精妙：**核心稳定，功能可扩展**。

---

## 七、学习路径建议

| 阶段 | 重点 |
|------|------|
| **先理解** | 本文的四概念关系（建立框架感） |
| **再深入 Experience** | 看 `ULyraExperienceDefinition` 怎么定义、怎么加载 |
| **再深入 GameFeature** | 看一个具体插件（如 ShooterMaps）怎么被启用、怎么注入功能 |
| **理解 ModularGameplay** | 看 GameFeature 用的 ModularGameplay Action 怎么注入 Pawn/能力 |
| **最后看 GameMode** | 看 `ALyraGameMode` 怎么读 Experience 配置、执行规则 |

> **别一上来就扎进 GameMode 代码**——先搞懂"Experience 驱动一切"这个思路，再看 GameMode 会清晰很多。

---

## 八、总结速查

```
Lyra 一局游戏的四层：
  GameFeature     = 可插拔功能插件（内容，外请节目）
  ModularGameplay = 模块化接口框架（机制，舞台插座标准）
  Experience      = 体验数据资产（数据，活动流程表）
  GameMode        = 游戏规则大脑（执行，总导演）

串起来的流程：
  玩家选模式 → 加载 Experience（数据）
    → Experience 启用 GameFeature（插件）
    → GameFeature 经 ModularGameplay 注入功能
    → GameMode 按 Experience 配置执行规则
    → 一局游戏跑起来

核心思想：
  数据驱动玩法（Experience）
  功能即插即用（GameFeature via ModularGameplay）
  规则由 GameMode 执行
  → 改数据换玩法，加插件加功能，核心代码不动
```

**一句话**：Lyra 把一局游戏拆成四层——**GameFeature**（可插拔功能插件，像外请节目）通过 **ModularGameplay**（模块化接口框架，像舞台标准插座）即插即用地注入功能，由 **Experience**（体验数据资产，像活动流程表）描述"这局玩什么"，最终交给 **GameMode**（规则大脑，像总导演）执行。四者协作实现"**改数据换玩法、加插件加功能、核心代码不动**"的模块化游戏框架。

---

## 九、架构全景图

### 9.1 四层关系图（谁驱动谁）

```
┌─────────────────────────────────────────────────────────────┐
│                        一局游戏                               │
│                                                               │
│   ④ GameMode（规则大脑 / 总导演）                             │
│      └─ 按 Experience 的配置执行：生成玩家、判胜负、管计分     │
│                          ▲ 执行规则                            │
│                          │                                     │
│   ③ Experience（数据资产 / 流程表）                           │
│      └─ 描述：用哪些 GameFeature + 哪个 GameMode + 什么角色    │
│          ▲ 驱动                            ▲ 指定             │
│          │                                  │                 │
│   ① GameFeature（可插拔插件 / 外请节目）                      │
│      ├─ ShooterMaps   ├─ TopDownArena   ├─ ...（按需启用）    │
│      └─ 通过 ② 的接口把功能注入游戏                           │
│              │                                                 │
│              ▼                                                 │
│   ② ModularGameplay（框架机制 / 舞台插座标准）                │
│      └─ 提供标准挂载点：Pawn 组件 / 能力 / 输入 的注入接口     │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

> 读图：**Experience（数据）在中间驱动一切**——它拉起 GameFeature，GameFeature 借 ModularGameplay 注入功能，最后 GameMode 按 Experience 配置执行规则。

---

### 9.2 玩家选模式时的完整时序

```
玩家点击"射击模式"
        │
        ▼
┌───────────────────┐
│ 加载 Experience    │  ← 数据资产（流程表）
│ (射击模式对应那份) │
└─────────┬─────────┘
          │ ① 指定启用哪些插件
          ▼
┌───────────────────┐
│ 启用 GameFeature   │  ← ShooterMaps 等插件被拉起
│ (按需热插拔)       │
└─────────┬─────────┘
          │ ② 通过标准接口注入功能
          ▼
┌───────────────────┐
│ ModularGameplay    │  ← 把射击组件/能力/输入插进 Pawn/控制器
│ (即插即用框架)     │
└─────────┬─────────┘
          │ ③ 功能就位，开局
          ▼
┌───────────────────┐
│ GameMode 执行规则  │  ← 生成玩家、判胜负、管重生计分
│ (总导演)           │
└─────────┬─────────┘
          │
          ▼
     一局"射击模式"跑起来了 🎮
```

---

### 9.3 换玩法 = 换数据（核心威力）

```
        ┌──────────── 不改核心代码 ────────────┐
        │                                        │
   射击模式                俯视竞技场模式
   Experience A    ──换──▶  Experience B
        │                        │
        ├ 启用 ShooterMaps       ├ 启用 TopDownArena
        ├ GameMode 配置甲        ├ GameMode 配置乙
        └ 角色/输入/能力甲       └ 角色/输入/能力乙

  同一套底层框架（ModularGameplay + 核心），
  只是换了一份 Experience 数据 → 玩法完全不同
```

> **一句话记住**：**Experience 是开关和配方，GameFeature 是积木，ModularGameplay 是积木的接口，GameMode 是按配方搭出来的成品。**

---

## 十、下一步

- 深入 `ULyraExperienceDefinition`：看它有哪些字段、怎么驱动加载。
- 深入一个 GameFeature 插件（如 `ShooterMaps`）：看它怎么被启用、怎么注入 Pawn/能力。
- 理解 ModularGameplay 的 `UGameFeatureAction`：看"注入"到底怎么发生。
- 看 `ALyraGameMode` 怎么和 Experience 配合执行规则。
