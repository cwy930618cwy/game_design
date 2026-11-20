# UE5 Lyra 系统学习路线图

> 基于 `Document/Lyra学习指南/` 122+ 章节内容整理，按「宏观→微观→实战」三阶段递进。
> 每阶段标注**核心章节**（必读）与**进阶章节**（选读），建议总周期 8-12 周。

---

## 前置要求

- C++ 基础扎实（参考 [UEC++从基础到进阶](https://www.bilibili.com/cheese/play/ss28043)）
- 已安装 UE5.6 源码引擎 + Lyra 工程（可从 Epic 商城下载二进制版或源码构建）
- 能独立编译、运行、调试 Lyra 工程

---

## 第一阶段：全局认知（第 1-2 周）

**目标**：建立对 Lyra 整体架构、设计理念、工程结构的宏观理解，能在编辑器中跑起来并看懂目录。

### Step 1.1 项目介绍与运行（第 1-2 天）
- **核心**：`001_项目介绍` — Lyra 是什么、三种游戏模式、下载安装
- **核心**：`002_源码速览` — Source/ 目录结构、LyraGame vs LyraEditor 模块划分
- **核心**：`003_蓝图速览` — Content 目录结构、各 GameFeature 插件的内容分布
- **实践**：在本地跑通 PIE，切换 FrontEnd / Elimination / Control 三种模式

**产出**：画一张 Lyra 模块依赖关系脑图（LyraGame 27 子目录 + Plugins 层级）。

### Step 1.2 工程创建与构建基础（第 3-4 天）
- **核心**：`004_创建工程` — 从模板新建 Lyra 风格工程
- **核心**：`005_Target文件` — `LyraGame.Target.cs` / `LyraClient.Target.cs` / `LyraServer.Target.cs` / `LyraEditor.Target.cs` 四类 Target 的差异
- **核心**：`006_Build文件` — `.Build.cs` 模块依赖声明、PCH、IWYU
- **核心**：`007_导入资产` — 如何将外部资产正确导入 Lyra 工程
- **核心**：`008_日志类别` — `LYRAPHYSICS_LOG` 等自定义 Log Channel 的使用

**产出**：手写一个最小可运行的 Lyra 衍生工程的 Target.cs + Build.cs。

### Step 1.3 Engine 与 AssetManager（第 5-7 天）
- **核心**：`009_Engine类` — `ULyraGameEngine` / `ULyraGameInstance` 的职责
- **补充**：`009_补_01_SPathView的bDisplayPluginFolders` — 插件文件夹显示机制
- **核心**：`012_AssetManager类` — `ULyraAssetManager` 异步加载策略、Bundle 管理
- **补充**：`012_泽_01_TSoftObjectPtr与FSoftObjectPath` — 软引用 vs 硬引用
- **补充**：`012_泽_02_一些性能优化的宏` — `UE_DEPRECATED` / `FORCEINLINE` 等

**产出**：梳理 Lyra 启动流程时序图（Target → GameEngine → GameInstance → WorldSettings → Experience）。

---

## 第二阶段：核心框架深入（第 3-5 周）

**目标**：吃透 Lyra 五大核心系统——Experience、GameMode 家族、GAS、输入、UI/CommonUI。

### Step 2.1 Experience 系统（第 8-9 天）⭐ 最重要
- **核心**：`010_定义Experience` — `ULyraExperienceDefinition` DataAsset 结构
- **核心**：`011_管理Experience` — `ULyraExperienceManagerComponent` 加载流程
- **补充**：`011_泽_01_异步资产加载和Bundle` — 异步加载机制
- **核心**：`023_异步询问体验加载` — `AsyncAction_ExperienceReady` 使用
- **核心**：`034_GameFeature` — GameFeature 插件机制原理
- **核心**：`035_LyraGameFeature` — `ULyraGameFeaturePolicy` 扩展点

**产出**：从零写一个新的 Experience Definition，加载自定义 GameFeature 并生效。

### Step 2.2 GameMode 家族与玩家生成（第 10-12 天）
- **核心**：`013_游戏实例` — GameInstance 生命周期
- **核心**：`014_创建游戏基础类` — `ALyraGameMode` / `ALyraGameState` 派生
- **核心**：`017_用户体验定义` — `ULyraUserFacingExperienceDefinition`
- **核心**：`018_玩家生成点` — `ALyraPlayerStart`
- **核心**：`019_玩家生成管理组件` — `ULyraPlayerSpawningManagerComponent`
- **核心**：`020_GameMode工作流程` — 完整 GameMode 初始化时序
- **核心**：`021_GameState工作流程` — GameState 上的 ExperienceManager
- **核心**：`024_PlayerState工作流程` — ASC 放在 PlayerState 的原因
- **核心**：`026_PlayerController工作流程` — 输入绑定、相机管理
- **核心**：`027_LocalPlayer` — 分屏支持
- **核心**：`028_GameUserSettings` — 通用用户设置接口

**产出**：画出 GameMode/GameState/PlayerController/PlayerState/Pawn 五者关系图 + 网络归属。

### Step 2.3 GAS 技能系统（第 13-16 天）⭐ 最重要
- **核心**：`080_GAS的架构` — ASC / GA / GE / AttributeSet 四件套
- **核心**：`081_ASC与GA` — `ULyraAbilitySystemComponent` / `ULyraGameplayAbility` 基类
- **核心**：`082_讲解GA_Jump` — 跳跃能力实现
- **核心**：`083_讲解GA_Dash` — 冲刺能力实现
- **核心**：`084_讲解GA_Melee` — 近战能力实现
- **核心**：`085_讲解属性集` — `LyraHealthSet` / `LyraCombatSet`
- **核心**：`086_讲解GEEC` — GameplayEffectExecutionCalculation 伤害计算
- **核心**：`087_生命值组件` — `ULyraHealthComponent` 死亡/复活流程
- **核心**：`104_异步混入AsyncMixin` — AsyncMixin 插件用法

**产出**：实现一个自定义 GA（如"护盾"），包含 Attribute、GE、GameplayCue。

### Step 2.4 输入系统与相机（第 17-18 天）
- **核心**：`070_输入系统` — `ULyraInputConfig` / `ULyraInputComponent` / Enhanced Input
- **核心**：`078_相机系统` — `ULyraCameraComponent` / `ULyraCameraMode`
- **核心**：`079_第三人称相机及其穿透功能` — 碰撞检测避免穿模

**产出**：为新模式添加自定义输入动作并映射到 GA。

### Step 2.5 UI 与 CommonUI（第 19-21 天）
- **核心**：`038_预加载界面` — 启动 Loading Screen
- **核心**：`039_CommonLoadingScreen` — 通用加载屏幕插件
- **核心**：`040_登录流程` — 前端大厅登录
- **核心**：`041_CommonUI_主界面` — CommonUI 框架集成
- **核心**：`042_异步推送控件的激活时机` — PushContentToLayerForPlayer
- **核心**：`043_LyraHUDLayout` — HUD 布局容器
- **核心**：`044_LyraButton` / `045_对话框` / `047_底部按钮栏`
- **核心**：`048_会话系统` — Session 管理
- **核心**：`049_CommonUser` — 用户身份抽象
- **核心**：`112_GameplayMessageRouter消息路由插件` — 消息驱动 UI

**产出**：用 CommonUI 实现一个新界面（如"成就列表"），通过 GameplayMessage 触发。

---

## 第三阶段：玩法系统与进阶主题（第 6-8 周）

**目标**：掌握角色、装备、武器、交互、队伍等具体玩法系统，以及性能/网络优化。

### Step 3.1 角色与动画（第 22-25 天）
- **核心**：`065_人物基类` — `ALyraCharacter` / `ALyraPawn`
- **核心**：`068_角色拓展中枢组件PawnExtension` — `ULyraPawnExtensionComponent` 协调初始化
- **核心**：`069_HeroComponent` — `ULyraHeroComponent` 输入/相机绑定
- **核心**：`067_移动组件和动画蓝图` — `ULyraCharacterMovementComponent`
- **核心**：`071_动画系统` / `072_动画要点` / `073_打通基本移动和相机`
- **核心**：`074_构建主动画蓝图线程安全逻辑` / `075_构建主动画蓝图状态机` / `076_构建动画层蓝图`
- **核心**：`077_动画涉及的专业概念`
- **补充**：`066_AI_角色移动模式的打包传输` — 网络同步移动

**产出**：创建一个带 PawnExtension + HeroComponent 的自定义 Pawn。

### Step 3.2 物品与装备（第 26-28 天）
- **核心**：`088_库存系统` — `ULyraInventoryManagerComponent` / Fragment 组合模式
- **核心**：`089_可交互定义` — `ULyraPickupDefinition`
- **核心**：`090_交互拾取流程` — 拾取→入库→装备
- **核心**：`091_装备系统架构` — `ULyraEquipmentDefinition` / `ULyraEquipmentInstance`
- **核心**：`097_武器生成器` — `ALyraWeaponSpawner`
- **核心**：`100_换装系统` — Cosmetics 外观定制

**产出**：新增一种可拾取装备（如"护甲"），含拾取、装备、卸载完整流程。

### Step 3.3 枪械与战斗（第 29-32 天）
- **核心**：`092_枪械系统_远程武器定义` — `LyraRangedWeaponInstance`
- **核心**：`093_枪械系统_武器开火技能定义` — `LyraGameplayAbility_RangedWeapon`
- **核心**：`094_枪械系统_武器准心` — Reticle Widget
- **核心**：`095_伤害反馈效果` — Context Effects
- **核心**：`096_伤害数字的生成` — NumberPop Component
- **核心**：`098_手榴弹技能` — 投掷物 GA
- **核心**：`099_辅助射击系统` — Aim Assist
- **补充**：`099_AI_如何理解拉力和减速效果`
- **补充**：`092_泽_如何构建子弹的散射`

**产出**：实现一把新武器（如"霰弹枪"），含散射、后坐力、命中反馈。

### Step 3.4 交互与指示器（第 33-34 天）
- **核心**：`103_指示器系统` — Indicator System（头顶标记、距离显示）
- **核心**：`029_Lyra本地游戏设置` / `030_音频混合子系统` / `031_平台渲染设置`
- **核心**：`032_SaveGame和LocalPlayerSaveGame` / `033_Lyra共享游戏设置`
- **核心**：`058-063_初始化XX设置` — 音频/手柄/玩法/键鼠/视频/性能分析

### Step 3.5 游戏设置 UI（第 35-37 天）
- **核心**：`050_游戏设置界面` — 设置面板入口
- **核心**：`051_LyraTabListWidgetBase` / `052_GameSettingPanel` / `053_GameSettingListView` / `054_GameSettingDetailView`
- **核心**：`055_LyraGameSettingRegistry` — 设置项注册表
- **核心**：`056_Get方法编译安全检测` / `057_GameSetting`
- **补充**：`055_泽_01_IWYU` — Include What You Use

### Step 3.6 队伍、阶段与 AI（第 38-41 天）
- **核心**：`101_队伍系统` — Team Subsystem
- **核心**：`102_外轮廓线` — 队友描边
- **核心**：`105_荣誉系统` / `106_击杀记录` — Accolades
- **核心**：`107_游戏阶段系统` — GamePhase Subsystem
- **核心**：`108_机器人系统` — Bot Creation
- **核心**：`109_行为树和环境查询` / `110_Gyra行为树补充`
- **核心**：`121_炸弹人玩法` — TopDownArena 完整案例

### Step 3.7 音乐与反馈（第 42-43 天）
- **核心**：`064_音乐组件` — Music Component
- **核心**：`037_LyraGameplayCueManager` — GameplayCue 管理
- **核心**：`029_泽_01_ControlBus` / `029_泽_02_HRTF` / `029_泽_03_ILatencyMarkerModule` / `029_泽_04_SetCustomLatencyMarker` / `029_泽_05_LyraSettingsHelpers`

---

## 第四阶段：工程化与高级主题（第 9-10 周）

**目标**：掌握网络同步、自动化测试、热更新、Editor 工具等生产级能力。

### Step 4.1 网络与性能（第 44-47 天）
- **核心**：`114_ReplicationGraph` — 网络复制优化
- **核心**：`115_自动化Automation系统` — UE Automation Framework
- **核心**：`116_Gauntlet自动化框架` — Gauntlet 测试编排
- **核心**：`117_回放系统` — Replay Subsystem
- **补充**：`025_Tag的FastArray容器` — FFastArraySerializer 增量复制
- **补充**：`020_泽_01_OptionsString` — 命令行参数解析
- **补充**：`021_泽_01_常见G开头的全局变量` — GEngine / GWorld 等

### Step 4.2 热更新与补丁（第 48-49 天）
- **核心**：`036_LyraHotFix` — 文本热修复、Runtime Options
- **核心**：`119_Editor模块代码` — LyraEditor 验证器
- **补充**：`119_泽_集合去重代码` — 实用工具代码

### Step 4.3 Editor 工具与 Pocket（第 50-51 天）
- **核心**：`034_AI_ActorFactory` — Actor 工厂
- **核心**：`034_AI_UContentBundleEngineSubsystem` — Content Bundle 子系统
- **核心**：`120_Pocket插件补充` — Pocket Worlds 口袋世界

### Step 4.4 作弊与调试（第 52 天）
- **核心**：`111_作弊调试系统` — Cheat Manager
- **核心**：`046_AI_关于Shift+ESC如何控制退出游戏`
- **核心**：`118_Game模板代码` — 模板工程最佳实践

### Step 4.5 终章与总结（第 53 天）
- **核心**：`122_终章` — 课程回顾、学习路径总结
- **复习**：回看第一阶段的脑图，补充细节

---

## 第五阶段：专题深化（第 11-12 周，选修）

根据实际项目需求选择方向深入。

### 专题 A：多人对战射击游戏
- 重点：`092-099` 枪械系统 + `101` 队伍 + `114` ReplicationGraph
- 实战：复刻一个 TDM 模式并加入自定义武器

### 专题 B：俯视角派对游戏
- 重点：`121_炸弹人玩法` + `TopDownArena` 全套
- 实战：基于 TopDownArena 做一个新玩法

### 专题 C：UGC / 编辑器扩展
- 重点：`119_Editor模块代码` + `034_AI_ActorFactory` + `120_Pocket插件补充`
- 实战：开发一个关卡编辑器工具

### 专题 D：性能与自动化
- 重点：`115-117` 自动化/Gauntlet/回放 + `025_FastArray`
- 实战：为项目搭建 CI 自动化测试流水线

---

## 学习方法建议

1. **每章必做**：打开对应源码文件，对照文档逐行阅读，关键函数加注释。
2. **动手优先**：每个 Step 的"产出"必须实际完成，不能只看不写。
3. **画图记忆**：复杂流程（Experience 加载、GAS 激活、网络同步）务必画时序图。
4. **对比学习**：遇到类似系统（如 Inventory vs Equipment、GameMode vs Experience）要列对比表。
5. **提问沉淀**：学习中遇到的问题写到 `AI_Learning_Notes/QA/` 目录，形成个人知识库。
6. **结合视频**：本指南是 B 站小刚老师课程的演讲稿，建议配合[原课程](https://www.bilibili.com/cheese/play/ss112001159)观看。

---

## 章节索引（按编号快速定位）

| 阶段 | 章节范围 | 主题 |
|------|---------|------|
| 一 | 001-009 | 项目介绍、源码、蓝图、Target、Build、Engine |
| 二·1 | 010-012, 023, 034-035 | Experience + GameFeature |
| 二·2 | 013-014, 017-021, 024, 026-028 | GameMode 家族 |
| 二·3 | 080-087, 104 | GAS 核心 |
| 二·4 | 070, 078-079 | 输入与相机 |
| 二·5 | 038-045, 047-049, 112 | CommonUI |
| 三·1 | 065-077 | 角色与动画 |
| 三·2 | 088-091, 097, 100 | 物品装备 |
| 三·3 | 092-099 | 枪械战斗 |
| 三·4 | 029-033, 103 | 设置与指示器 |
| 三·5 | 050-057, 058-063 | 游戏设置 UI |
| 三·6 | 101-102, 105-110, 121 | 队伍/AI/阶段 |
| 三·7 | 064, 037, 029_泽 | 音乐与反馈 |
| 四·1 | 114-117, 025, 020-021_泽 | 网络与自动化 |
| 四·2 | 036, 119 | 热更新与补丁 |
| 四·3 | 034_AI, 120 | Editor 工具 |
| 四·4 | 111, 046_AI, 118 | 调试与模板 |
| 四·5 | 122 | 终章 |

---

## 下一步行动

请确认此路线图是否符合你的学习节奏，我可以：
1. 为每个 Step 生成详细的**学习笔记模板**（含源码导读、关键函数清单、思考题）
2. 针对某一章进行**深度教学**（逐行解读源码 + 设计意图分析）
3. 提供**实战练习脚手架**（新 Experience / 新 GA / 新武器的起步代码）

告诉我你想从哪一步开始即可。

---

## 本地路径速查（不用每次找）

> 直接复制粘贴到文件资源管理器或 IDE 打开。

### Lyra 工程（5.6 版源码）

```
e:\code\lyra_fifty_six\LyraStarterGame
```

**常用子路径**：
- Lyra C++ 源码：`e:\code\lyra_fifty_six\LyraStarterGame\Source\LyraGame`
- Lyra 插件：`e:\code\lyra_fifty_six\LyraStarterGame\Plugins`
- Lyra 资产：`e:\code\lyra_fifty_six\LyraStarterGame\Content`

### UE5.6 引擎（源码）

```
d:\ue5\Epic Games\UE_5.6
```

**常用子路径**：
- 引擎 C++ 源码：`d:\ue5\Epic Games\UE_5.6\Engine\Source`
- Core 模块：`d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core`
- CoreUObject 模块：`d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\CoreUObject`
- Engine 模块：`d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine`

### 常用源码文件（引擎）

| 找什么 | 路径 |
|--------|------|
| APawn | `Engine\Source\Runtime\Engine\Classes\GameFramework\Pawn.h` |
| ACharacter | `Engine\Source\Runtime\Engine\Classes\GameFramework\Character.h` |
| AActor | `Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h` |
| UObject | `Engine\Source\Runtime\CoreUObject\Public\UObject\Object.h` |
| FObjectInitializer | `Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectGlobals.h` |
| 反射宏 | `Engine\Source\Runtime\CoreUObject\Public\UObject\ObjectMacros.h` |

> **提示**：完整路径前缀引擎为 `d:\ue5\Epic Games\UE_5.6\`，Lyra 为 `e:\code\lyra_fifty_six\LyraStarterGame\`，上表省略了前缀。
