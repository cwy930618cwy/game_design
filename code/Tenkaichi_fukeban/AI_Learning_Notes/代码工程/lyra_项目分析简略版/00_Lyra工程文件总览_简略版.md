# LyraStarterGame 工程文件总览（简略版）

> 目标：**5 分钟搞清楚这个工程里每个文件夹是干嘛的**，知道该点开哪个、可以忽略哪个。
> 详细版见 `Lyra_项目分析/01_Lyra工程目录结构详解.md`。

---

## 一、先记住一句话

**LyraStarterGame = 一家餐厅。**

| 工程目录 | 类比 | 作用 |
|---|---|---|
| `Source/` | **后厨 + 菜谱** | 所有 C++ 代码，游戏的"脑子" |
| `Content/` | **食材 + 摆盘** | 美术资源（模型/贴图/蓝图/地图） |
| `Config/` | **店规** | 各种 ini 配置 |
| `Plugins/` | **外包档口** | 可插拔的独立功能模块 |
| `Build/` | **装修队** | 打包、部署、自动化脚本 |
| `Binaries/` `Intermediate/` `Saved/` `DerivedDataCache/` | **垃圾堆** | 引擎生成物，**永远不用看** |

> 🔑 **记住这个比例**：这个工程 90% 的知识量在 `Source/` 和 `Plugins/` 里。其他都是配角。

---

## 二、顶层目录长什么样

```
LyraStarterGame/
├── LyraStarterGame.uproject   ← 【入口】工程清单：引擎版本、模块、启用了哪些插件
├── README.md                  ← 说明"Content 不在 git 里，要自己下"
│
├── Source/                    ← 【核心】C++ 代码（487 个文件）★ 90% 时间看这里
├── Plugins/                   ← 【核心】插件（6162 个文件，17 个 .uplugin）
├── Config/                    ← 配置文件（45 个 .ini）
├── Content/                   ← 美术资源（通常很大，被 git 忽略）
├── Build/                     ← 打包/自动化脚本
│
├── Binaries/                  ← 编译产物 ❌
├── Intermediate/              ← 编译中间件 ❌
├── DerivedDataCache/          ← 资源缓存 ❌
├── Saved/                     ← 日志/存档 ❌
└── Platforms/                 ← 平台扩展 ❌
```

**`LyraStarterGame.uproject` 值得先扫一眼**，它回答三个问题：

```14:21:LyraStarterGame.uproject
	"EngineAssociation": "5.6",
	"Category": "Samples",
	"Modules": [
		{
			"Name": "LyraGame",
			"Type": "Runtime",
			"LoadingPhase": "Default",
```

- `EngineAssociation: 5.6` → 这是 UE 5.6 版的 Lyra
- 只有 **2 个模块**：`LyraGame`（运行时）+ `LyraEditor`（编辑器）
- 后面一长串 `"Plugins"` 数组 → 启用了 **70 多个插件**（其中很多是引擎自带能力，比如 EOS/Steam/CommonUI/GameFeatures）

---

## 三、`Source/`：代码在这，分三个部分

```
Source/
├── LyraGame/          ← ★★★ 真正的游戏逻辑（451 文件）—— 学习重点
├── LyraEditor/        ← 只在编辑器里跑的扩展（26 文件）
│
└── *.Target.cs        ← 10 个"编译目标"文件
```

### ① `LyraGame/` —— 唯一的运行时模块

**451 个文件**，全部游戏逻辑。往下细分 26 个目录，看第四节。
入口文件在根上：

| 文件 | 干嘛的 |
|---|---|
| `LyraGame.Build.cs` | 声明这个模块依赖哪些库（想加库就改这里） |
| `LyraGameModule.cpp/.h` | 模块启动/关闭时的回调（`StartupModule` / `ShutdownModule`） |
| `LyraGameplayTags.cpp/.h` | ★ 全局 GameplayTag 声明（`LyraGameplayTags::ShooterGame_...` 都在这） |
| `LyraLogChannels.cpp/.h` | 自定义日志通道（`UE_LOG(LogLyra, ...)`） |

### ② `LyraEditor/` —— 编辑器专用

只在编辑器里编译和运行，打包进游戏时**完全不存在**。放三类东西：

| 子目录 | 干嘛的 |
|---|---|
| `Validation/` | 资源检查规则（提交前检查美术资源是否规范） |
| `Commandlets/` | 命令行批处理工具（如 `ContentValidationCommandlet`） |
| `Utilities/` | 一次性小工具（如 `CreateRedirectorPackage`） |
| `Private/` | 自定义资源类型 + 编辑器样式（`GameEditorStyle`） |

从文件名就能看出：`AssetTypeActions_LyraContextEffectsLibrary` = **给自定义资产加右键菜单**，`LyraContextEffectsLibraryFactory` = **让它能在编辑器里被创建**。

### ③ 10 个 `*.Target.cs` —— 编译目标

**这是很多人会忽略但很重要的一块。** 它决定"编译出来的东西是给谁用的"：

```
LyraClient.Target.cs          → 纯客户端
LyraServer.Target.cs          → 纯服务器（无渲染）
LyraEditor.Target.cs          → 编辑器
LyraGame.Target.cs            → 单机/通用

  带 EOS 后缀  → 用 Epic Online Services 联机
  带 Steam 后缀 → 用 Steam 联机
  两个都有      → 两种都支持
```

每个文件都很短，只干一件事 —— 设置 `Type`，然后套用公共设置：

```7:15:Source/LyraServer.Target.cs
public class LyraServerTarget : TargetRules
{
	public LyraServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		ExtraModuleNames.AddRange(new string[] { "LyraGame" });

		LyraGameTarget.ApplySharedLyraTargetSettings(this);
```

> 💡 **关键点**：`TargetType.Server` 会定义 `UE_SERVER` 宏，并且不加载客户端专用的资源包（后面讲 Experience 时会看到"区分 client bundle / server bundle"正是为它服务的）。
> 这就是为什么 Lyra 代码里到处在判断 `NM_DedicatedServer` / `NM_Client`。

---

## 四、`Source/LyraGame/` 26 个目录（重点）

这么多目录硬看会迷路，**先按功能分四组**：

```
┌─ 【1. 骨架】游戏怎么跑起来的 ───────────────────────┐
│  System(27)  GameModes(20)  GameFeatures(16)  Player(16) │
├─ 【2. 玩法】游戏玩什么 ────────────────────────────┤
│  AbilitySystem(51)  Weapons(16)  Equipment(12)         │
│  Inventory(16)  Interaction(17)  Teams(22)  Character(17) │
├─ 【3. 表现】玩家看到/听到什么 ──────────────────────┤
│  UI(79)  Feedback(19)  Camera(12)  Animation(2)         │
│  Audio(4)  Cosmetics(11)  Messages(9)                   │
├─ 【4. 基建】支撑上面三组 ───────────────────────────┤
│  Settings(35)  Input(14)  Replays(4)  Hotfix(6)         │
│  Performance(6)  Physics(3)  Development(6)  Tests(4)   │
└─────────────────────────────────────────────────────┘
```

### 逐目录速查表

| 目录 | 文件数 | 一句话 | 代表文件 |
|---|---|---|---|
| **System** | 27 | ⭐ 启动链总枢纽：GameInstance / AssetManager / ReplicationGraph / Session | `LyraGameInstance` `LyraAssetManager` |
| **GameModes** | 20 | ⭐ 用 **Experience（体验）** 组织一局游戏 | `LyraExperienceManagerComponent` `LyraGameMode` |
| **GameFeatures** | 16 | ⭐ **GameFeatureAction** 的具体实现（挂玩法用） | `GameFeatureAction_AddAbilities` `_AddWidget` `_AddInputBinding` |
| **Player** | 16 | 玩家相关：Controller / PlayerState / 出生点 / 作弊 | `LyraPlayerController` `LyraPlayerState` |
| **AbilitySystem** | 51 | ⭐ GAS 技能系统，全工程第二大的单块 | `LyraAbilitySystemComponent` `LyraGameplayAbility` |
| **Weapons** | 16 | 武器：生成、瞄准、开火、弹道 | `LyraWeaponInstance` `LyraRangedWeaponInstance` |
| **Equipment** | 12 | 装备管理器（武器只是装备的一种） | `LyraEquipmentManagerComponent` |
| **Inventory** | 16 | 背包 / 拾取物 / 携带物 | `LyraInventoryManagerComponent` |
| **Interaction** | 17 | 交互：准星指着东西、按键互动 | `LyraInteractionAbility` / `*_Trace` / `*_Widget` |
| **Teams** | 22 | 队伍分配 / 敌我判定 / 显示队伍颜色 | `LyraTeamSubsystem` `LyraTeamDisplayAsset` |
| **Character** | 17 | 角色本体：移动、跳跃、Pawn 数据 | `LyraCharacter` `LyraPawnExtensionComponent` `LyraPawnData` |
| **UI** | 79 | ⭐ **全工程最大的单块**：HUD / 前端 / 通用控件 | `LyraHUD` `LyraActivatableWidget` |
| **Feedback** | 19 | 受击反馈"手感"：受击特效、飘数字 | `ContextEffects/` `NumberPops/` |
| **Camera** | 12 | 相机模式与相机组件 | `LyraCameraMode` `LyraCameraComponent` |
| **Animation** | 2 | 动画实例基类 | `LyraAnimInstance` |
| **Audio** | 4 | 音频混音 + 音频设置 | `LyraAudioMixEffectsSubsystem` |
| **Cosmetics** | 11 | 外观/皮肤（换色、换皮） | `LyraPawnData` 的皮肤相关 |
| **Messages** | 9 | **消息总线**：跨系统广播（击杀、通知） | `LyraVerbMessage` `GameplayMessageProcessor` |
| **Settings** | 35 | ⭐ 设置系统：画质/音频/按键/灵敏度 | `LyraSettingsLocal` `LyraGameSettingRegistry_*` |
| **Input** | 14 | 输入映射（基于 EnhancedInput） | `LyraInputConfig` `LyraInputComponent` |
| **Replays** | 4 | 录像回放（查询、播放） | `LyraReplaySubsystem` |
| **Hotfix** | 6 | 热更新（不改包直接改数值/文本） | `LyraHotfixManager` `LyraTextHotfixConfig` |
| **Performance** | 6 | 性能统计面板 | `LyraPerformanceStatSubsystem` |
| **Physics** | 3 | 碰撞通道定义 + 带标签的物理材质 | `LyraCollisionChannels.h` `PhysicalMaterialWithTags` |
| **Development** | 6 | 开发者设置 / 平台模拟 / 机器人作弊指令 | `LyraDeveloperSettings` `LyraBotCheats` |
| **Tests** | 4 | 自动化测试入口 | `LyraTestControllerBootTest` |

### 挑重点记（只记这 6 个）

如果只给你 6 个目录，优先看：

```
① System           —— 游戏怎么启动（GameInstance / AssetManager）
② GameModes        —— 一局游戏怎么配置（Experience 机制）
③ GameFeatures     —— 玩法怎么"挂"上去（GameFeatureAction）
④ AbilitySystem    —— 角色能力怎么做（GAS）
⑤ UI               —— 界面怎么搭（Lyra 的 UI 体系很完整）
⑥ Player/Character —— 玩家与角色的分层（Controller / State / Pawn 三层）
```

---

## 五、`Plugins/`：13 个插件目录

**分成两拨看**：

### ① Lyra 自带的功能插件（12 个）

这些是"**通用积木**"，官方做出来给大家复用的：

| 插件 | 干嘛的 | 重要度 |
|---|---|---|
| `CommonGame` | 通用游戏框架（前端/图层/UI 策略） | ⭐⭐ |
| `CommonUser` | 通用用户/登录/存档管理 | ⭐⭐ |
| `CommonLoadingScreen` | 加载界面管理 | ⭐ |
| `ModularGameplayActors` | 让 GameMode/PlayerState 等支持组件化 | ⭐⭐ |
| `GameplayMessageRouter` | 消息总线（`UGameplayMessageSubsystem`） | ⭐⭐ |
| `GameSettings` | 设置界面框架 | ⭐ |
| `GameSubtitles` | 字幕 | ⭐ |
| `UIExtension` | UI 插槽扩展（往已有界面里"插"控件） | ⭐ |
| `AsyncMixin` | 异步加载蓝图节点 | ⭐ |
| `PocketWorlds` | 小型独立世界（用于大厅预览） | – |
| `LyraExtTool` | 外部工具集成 | – |
| `LyraExampleContent` | 示例资源（78 个 uasset） | – |

### ② `GameFeatures/` —— 5 个**玩法**插件 ⭐

**这是 Lyra 玩法的实际所在地**，也是最能体现"插件化"的地方：

| 插件 | 内容 | 说明 |
|---|---|---|
| `ShooterCore` | 射击核心（14 h + 12 cpp + 258 uasset） | ⭐ **最值得看**，真正的玩法逻辑 |
| `ShooterMaps` | 射击地图（5325 个 uasset！） | 纯资源 |
| `ShooterExplorer` | 探索模式变体 | 换个资产组合就成新模式 |
| `TopDownArena` | 俯视角竞技场 | **另一个完全不同的玩法**，证明架构可复用 |
| `ShooterTests` | 玩法的自动化测试 | 带 `README.md` |

> 🔑 **这里藏着 Lyra 最核心的设计思想**：
> `ShooterCore` 和 `TopDownArena` 是**两个完全不同的游戏**，却共用同一套 `LyraGame` 框架。
> 靠的就是 `Experience` + `GameFeature` 这套机制 —— 代码不写在主工程里，而是打包成插件按需加载。

---

## 六、`Config/`：45 个 ini

| 文件 | 管什么 | 重要度 |
|---|---|---|
| `DefaultEngine.ini` | ⭐ 引擎级配置：默认地图、渲染、网络、AssetManager 规则 | 必看 |
| `DefaultGame.ini` | 项目信息、默认 GameMode | 必看 |
| `DefaultGameplayTags.ini` | ⭐ GameplayTag 的登记表（新增标签在这注册） | 必看 |
| `DefaultInput.ini` | 输入（EnhancedInput 时代的遗留配置） | ⭐ |
| `DefaultEditor*.ini` | 编辑器偏好、快捷键 | – |
| `DefaultScalability.ini` | 画质档位（低/中/高各档参数） | ⭐ |
| `DefaultDeviceProfiles.ini` | 设备分级 | – |
| `DefaultRuntimeOptions.ini` | 运行期可改的选项 | – |

子目录：`Windows/` `Android/` `IOS/` `Mac/` `Linux/`（平台覆盖配置）、`Localization/`（16 个翻译文件）、`Custom/`。

> 💡 想找"默认地图是哪张"、"AssetManager 怎么配的" → **直接翻 `DefaultEngine.ini`**。

---

## 七、`Build/`：打包与脚本

| 路径 | 干嘛的 |
|---|---|
| `BatchFiles/*.bat` | 一键脚本：`RunLocalPackage.bat` 本地打包、`RunLocalTests.bat` 跑测试、`RunLocalize.bat` 本地化 |
| `Scripts/` | C# 自动化脚本（9 个 .cs） |
| `Android/` `IOS/` `Windows/` | 各平台打包素材（图标等，都是 png） |
| `Replays/` | 回放相关的构建数据 |
| `LyraBuild.xml` / `LyraTests.xml` / `GauntletSettings.xml` | **Gauntlet** 自动化测试框架的配置 |
| `UnrealGameSync.ini` | UGS（团队同步工具）配置 |

> 日常开发基本用不到，**只在"要出包"或"要跑自动化测试"时才会碰**。

---

## 八、`Content/`：美术资源（一句话带过）

**不在 git 仓库里**，需要从 Epic Games Launcher 单独下载。原因见 `README.md`：

```6:12:LyraStarterGame/README.md
When downloading Lyra Source code from git, the content folders are not included.

To make use of the Lyra source code, you will need to download the content from the Unreal Marketplace through the Epic Games Launcher.
```

内容就是各种 `.uasset`（蓝图、材质、动画、UI 控件）和 `.umap`（地图）。
**学代码阶段可以先完全忽略这一块。**

---

## 九、可以放心忽略的目录

```
Binaries/           编译出来的 dll/exe
Intermediate/       编译中间文件
DerivedDataCache/   资源编译缓存
Saved/              日志、配置缓存、存档、截图
Platforms/          平台 SDK 扩展
```
这些删了会自动重建，**看到就往右划走**。

---

## 十、建议的学习路线

```
第 1 站  LyraStarterGame.uproject      ← 5 分钟，看启用了什么
第 2 站  Config/DefaultEngine.ini      ← 看默认地图 + AssetManager 配置
第 3 站  Source/LyraGame/System/       ← ⭐ 游戏怎么启动的（GameInstance/AssetManager）
第 4 站  Source/LyraGame/GameModes/    ← ⭐ Experience 机制（Lyra 的灵魂）
第 5 站  Source/LyraGame/GameFeatures/  ← ⭐ Action 怎么"挂"玩法
        Plugins/GameFeatures/ShooterCore/ ← ⭐ 配合看，实际玩法代码
第 6 站  Source/LyraGame/AbilitySystem/ ← 能力系统（GAS 落地）
第 7 站  Source/LyraGame/UI/           ← UI 体系
```

**一句话收尾**：

> `Source/` 是**脑子**（怎么跑），`Plugins/GameFeatures/` 是**四肢**（玩什么），
> `Content/` 是**皮相**（长什么样），`Config/` 是**规矩**（怎么配）。
> 学 Lyra，**先啃 `Source/` 的 6 个核心目录，剩下的按需查阅即可**。
