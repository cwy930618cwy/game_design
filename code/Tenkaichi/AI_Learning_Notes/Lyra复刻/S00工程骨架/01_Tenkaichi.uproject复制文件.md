# 01 — `Tenkaichi.uproject` 复制文件

> 依据：`E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（**347 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**（JSON 不支持注释）。
>
> ⚠️ **本版已重写（2026-11-23）**：修正了旧版「61 项插件」的错误 —— 实际是 **70 项**（59 引擎 + 9 工程 + 2 豁免）。
> 每一项都标注了「来源 = 引擎 / 工程」和「本机是否存在」。**插件校验详情见 `00_S00开工必读_插件校验与配置前置.md`**。

---

## 一、先记住 4 条插件语义（不然敲了也是白敲）

| 写法 | 含义 | 本机缺失时会怎样 |
|---|---|---|
| `"Enabled": true` | 必须有 | ❌ **报错** `X plugin is referenced by target but not found` |
| `"Enabled": false` | 禁用 | ✅ 无害（引擎压根不查） |
| `"Optional": true` | 可选（有就用，没有就跳过） | ✅ 无害 |
| `"PlatformAllowList"` / `"SupportedTargetPlatforms"` | 平台白名单 | ✅ 当前平台不在名单里就跳过 |

---

## 二、完整代码（70 项插件）

```jsonc
// 整个 .uproject 是一个 JSON 对象，从这里开始
{
	// .uproject 格式版本号；UE5 固定写 3，写错引擎不认
	"FileVersion": 3,
	// 绑定本机注册的 UE 5.4；双击工程时引擎按这个版本启动
	"EngineAssociation": "5.4",
	// 启动器里的分类标签，纯展示用，不影响编译
	"Category": "Samples",
	// 工程描述，纯展示用，留空即可
	"Description": "",
	// 代码模块数组；每一项对应 Source\ 下的一个目录
	"Modules": [
		// 第 1 个模块对象开始
		{
			// 模块名；引擎据此去 Source\LyraGame\ 找 LyraGame.Build.cs
			"Name": "LyraGame",
			// 模块类型：Runtime = 运行时模块，会打包进游戏
			"Type": "Runtime",
			// 加载时机：Default = 引擎启动时加载
			"LoadingPhase": "Default",
			// 额外的 UHT 依赖；让这些模块的反射代码先生成
			"AdditionalDependencies": [
				// 开发者设置模块；Lyra 的 ULyraDeveloperSettings 依赖它
				"DeveloperSettings",
				// 引擎主体模块；几乎所有 UCLASS 都间接依赖它
				"Engine"
			]
		}
	],
	// Modules 数组结束（S00 只有 LyraGame；LyraEditor 在 S22 补回）
	// 插件数组；每一项是一个插件开关
	"Plugins": [
		// ── 1 ── 引擎插件 ──
		{
			// Actor 调色板：编辑器里把常用 Actor 存成面板，一键拖进场景
			"Name": "ActorPalette",
			"Enabled": true
		},
		// ── 2 ── 引擎插件 ──
		{
			// 网络加密组件（AES-GCM 算法），防抓包篡改
			"Name": "AESGCMHandlerComponent",
			"Enabled": true
		},
		// ── 3 ── 引擎插件 ──
		{
			// 网络加密组件（DTLS 协议），用于语音 / UDP 加密
			"Name": "DTLSHandlerComponent",
			"Enabled": true
		},
		// ── 4 ── 引擎插件 ──
		{
			// ★ GAS 插件：技能、属性、GameplayEffect、GameplayCue 全在这
			"Name": "GameplayAbilities",
			"Enabled": true
		},
		// ── 5 ── 引擎插件 ──
		{
			// 自动化测试框架，CI 无人值守跑测试用
			"Name": "Gauntlet",
			"Enabled": true
		},
		// ── 6 ── ★ 本机没有此插件（靠 Optional 豁免）──
		{
			// D3D 外接显卡性能统计
			"Name": "D3DExternalGPUStatistics",
			"Enabled": true,
			// ★ 关键：Optional=true → 本机找不到也不报错
			"Optional": true,
			// 仅在下列平台启用
			"SupportedTargetPlatforms": [
				// 仅 Windows 64 位
				"Win64"
			]
		},
		// ── 7 ── 引擎插件 ──
		{
			// NPC 对话树系统（对话编辑器 + 运行时）
			"Name": "CommonConversation",
			"Enabled": true
		},
		// ── 8 ── 引擎插件 ──
		{
			// ★ GameFeature 机制：把玩法做成可插拔功能包，运行时加载 / 卸载
			"Name": "GameFeatures",
			"Enabled": true
		},
		// ── 9 ── 引擎插件 ──
		{
			// ★ 模块化 Gameplay：提供 UGameFrameworkComponentManager 和初始化状态机
			"Name": "ModularGameplay",
			"Enabled": true
		},
		// ── 10 ── 引擎插件 ──
		{
			// ★ 增强输入系统：UInputAction / UInputMappingContext
			"Name": "EnhancedInput",
			"Enabled": true
		},
		// ── 11 ── 引擎插件 ──
		{
			// PS4 / PS5 手柄支持
			"Name": "WinDualShock",
			"Enabled": true,
			"SupportedTargetPlatforms": [
				// 仅 Windows
				"Win64"
			]
		},
		// ── 12 ── 引擎插件 ──
		{
			// 体积云 / 体积雾
			"Name": "Volumetrics",
			"Enabled": true
		},
		// ── 13 ── 引擎插件 ──
		{
			// 数据注册表：按 ID 查资产，Lyra 用它管理 GameplayCue 等
			"Name": "DataRegistry",
			"Enabled": true
		},
		// ── 14 ── 引擎插件 ──
		{
			// ★ 网络复制图：按空间网格决定谁同步给谁，省带宽
			"Name": "ReplicationGraph",
			"Enabled": true
		},
		// ── 15 ── 引擎插件 ──
		{
			// 重要性管理器：远处 Actor 降频更新（不跑 AI、不更新动画）
			"Name": "SignificanceManager",
			"Enabled": true
		},
		// ── 16 ── 引擎插件 ──
		{
			// 粒子系统（VFX）
			"Name": "Niagara",
			"Enabled": true
		},
		// ── 17 ── 引擎插件（★ 配错 ini 会报水体碰撞错误）──
		{
			// 水体系统；启动时会检查 DefaultEngine.ini 里有没有 WaterBodyCollision
			"Name": "Water",
			"Enabled": true
		},
		// ── 18 ── 引擎插件 ──
		{
			// ★ CommonUI 框架：手柄 / 键鼠 / 触屏统一的 UI 输入与焦点管理
			"Name": "CommonUI",
			"Enabled": true
		},
		// ── 19 ── 引擎插件 ──
		{
			// 控制流：节点图执行框架
			"Name": "ControlFlows",
			"Enabled": true
		},
		// ── 20 ── 引擎插件 ──
		{
			// 新一代音频引擎（MetaSounds），替代旧 SoundCue
			"Name": "Metasound",
			"Enabled": true
		},
		// ── 21 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// MagicLeap AR 平台支持（UE5.4 起已从引擎移除）
			"Name": "MagicLeap",
			// 禁用：条目保留以便对照 Lyra；禁用状态引擎不查，本机没有也不报错
			"Enabled": false
		},
		// ── 22 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// MagicLeap 媒体播放（已移除）
			"Name": "MagicLeapMedia",
			"Enabled": false
		},
		// ── 23 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// MagicLeap 可通行世界（已移除）
			"Name": "MagicLeapPassableWorld",
			"Enabled": false
		},
		// ── 24 ── 引擎插件 ──
		{
			// OpenXR 眼动追踪
			"Name": "OpenXREyeTracker",
			"Enabled": false
		},
		// ── 25 ── 引擎插件 ──
		{
			// OpenXR 手部追踪
			"Name": "OpenXRHandTracking",
			"Enabled": false
		},
		// ── 26 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// OpenXR 头显（UE5.4 已合并进 OpenXR 主插件）
			"Name": "OpenXRHMD",
			"Enabled": false
		},
		// ── 27 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// SteamVR 支持（5.4 已改为可选分发）
			"Name": "SteamVR",
			"Enabled": false
		},
		// ── 28 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// 三星 GearVR 支持（已废弃）
			"Name": "GearVR",
			"Enabled": false
		},
		// ── 29 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// MagicLeap Lumin 平台特性（已废弃）
			"Name": "LuminPlatformFeatures",
			"Enabled": false,
			"SupportedTargetPlatforms": [
				// 仅 Lumin 平台
				"Lumin"
			]
		},
		// ── 30 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// MagicLeap SDK（已移除）
			"Name": "MLSDK",
			"Enabled": false
		},
		// ── 31 ── 引擎插件 ──
		{
			// 在线框架：大厅、会话、匹配
			"Name": "OnlineFramework",
			"Enabled": true
		},
		// ── 32 ── ★ 本机没有此插件（平台白名单豁免）──
		{
			// PlayFab 语音 / 网络
			"Name": "PlayFabParty",
			"Enabled": true,
			// ★ 关键：只有 XB1 / XSX 允许启用 → Win64 下引擎直接跳过，本机没有也不报错
			"PlatformAllowList": [
				// Xbox One
				"XB1",
				// Xbox Series X
				"XSX"
			],
			"SupportedTargetPlatforms": [
				"XB1",
				"XSX"
			]
		},
		// ── 33 ── 引擎插件 ──
		{
			// Epic Online Services 旧接口（OnlineSubsystem 系列）
			"Name": "OnlineSubsystemEOS",
			"Enabled": true
		},
		// ── 34 ── 引擎插件 ──
		{
			// EOS 新接口（OnlineServices 系列）
			"Name": "OnlineServicesEOS",
			"Enabled": true
		},
		// ── 35 ── 引擎插件 ──
		{
			// 空实现：离线测试用，不连任何在线服务
			"Name": "OnlineServicesNull",
			"Enabled": true
		},
		// ── 36 ── 引擎插件 ──
		{
			// 把旧 OSS 接口适配到新 OnlineServices
			"Name": "OnlineServicesOSSAdapter",
			"Enabled": true
		},
		// ── 37 ── 引擎插件 ──
		{
			// Steam 平台接口
			"Name": "OnlineSubsystemSteam",
			"Enabled": true
		},
		// ── 38 ── 引擎插件 ──
		{
			// Steam 网络套接字
			"Name": "SteamSockets",
			"Enabled": true
		},
		// ══════════ 以下 9 项是「工程插件」，源码在 S01~S05 才建 ══════════
		// ── 39 ── 工程插件（S02 建源码）──
		{
			// 通用加载界面：ILoadingProcessInterface 与加载屏管理器
			// 来源：Tenkaichi\Plugins\CommonLoadingScreen\
			"Name": "CommonLoadingScreen",
			"Enabled": true
		},
		// ── 40 ── 工程插件（S01 建源码）──
		{
			// ★ 模块化玩法 Actor：AModularPawn / AModularCharacter 等
			// 来源：Tenkaichi\Plugins\ModularGameplayActors\
			// LyraGame.Build.cs 公共依赖它，缺了主模块编不过
			"Name": "ModularGameplayActors",
			"Enabled": true
		},
		// ── 41 ── 工程插件（S04 建源码）──
		{
			// 设置系统：GameSetting / 设置界面框架
			// 来源：Tenkaichi\Plugins\GameSettings\
			"Name": "GameSettings",
			"Enabled": true
		},
		// ── 42 ── 工程插件（S02 建源码）──
		{
			// 通用用户系统：登录 / 会话 / 在线服务封装
			// 来源：Tenkaichi\Plugins\CommonUser\
			"Name": "CommonUser",
			"Enabled": true
		},
		// ── 43 ── 工程插件（S03 建源码）──
		{
			// 通用游戏层：GameInstance / UI 策略 / UI 管理层栈
			// 来源：Tenkaichi\Plugins\CommonGame\
			"Name": "CommonGame",
			"Enabled": true
		},
		// ── 44 ── 工程插件（S04 建源码）──
		{
			// 字幕系统：字幕显示组件与 SSubtitleDisplay
			// 来源：Tenkaichi\Plugins\GameSubtitles\
			"Name": "GameSubtitles",
			"Enabled": true
		},
		// ── 45 ── 工程插件（S03 建源码）──
		{
			// UI 扩展点：UIExtensionPointWidget，允许玩法模块往 HUD 里插控件
			// 来源：Tenkaichi\Plugins\UIExtension\
			"Name": "UIExtension",
			"Enabled": true
		},
		// ── 46 ── 工程插件（S01 建源码）──
		{
			// 异步 Mixin：AsyncTask 组合工具
			// 来源：Tenkaichi\Plugins\AsyncMixin\
			"Name": "AsyncMixin",
			"Enabled": true
		},
		// ── 47 ── 工程插件（S01 建源码）──
		{
			// ★ 玩法消息路由：GameplayMessageSubsystem 与监听用的异步节点
			// 来源：Tenkaichi\Plugins\GameplayMessageRouter\
			"Name": "GameplayMessageRouter",
			"Enabled": true
		},
		// ══════════ 以下回到引擎插件 ══════════
		// ── 48 ── 引擎插件 ──
		{
			// 资产引用限制规则：防止跨模块乱引用
			"Name": "AssetReferenceRestrictions",
			"Enabled": true
		},
		// ── 49 ── 引擎插件 ──
		{
			// 建模工具编辑器模式
			"Name": "ModelingToolsEditorMode",
			"Enabled": true
		},
		// ── 50 ── 引擎插件 ──
		{
			// 几何脚本：程序化建模
			"Name": "GeometryScripting",
			"Enabled": true
		},
		// ── 51 ── 引擎插件 ──
		{
			// 动画移动库：步态混合
			"Name": "AnimationLocomotionLibrary",
			"Enabled": true
		},
		// ── 52 ── 引擎插件 ──
		{
			// 音频调制：参数化混音
			"Name": "AudioModulation",
			"Enabled": true
		},
		// ── 53 ── 引擎插件 ──
		{
			// 音频玩法体积：区域混响
			"Name": "AudioGameplayVolume",
			"Enabled": true
		},
		// ── 54 ── 引擎插件 ──
		{
			// 音频玩法组件
			"Name": "AudioGameplay",
			"Enabled": true
		},
		// ── 55 ── 引擎插件 ──
		{
			// 音频工具函数库
			"Name": "SoundUtilities",
			"Enabled": true
		},
		// ── 56 ── 引擎插件 ──
		{
			// 动画扭曲：脚部贴地、朝向修正
			"Name": "AnimationWarping",
			"Enabled": true
		},
		// ── 57 ── 引擎插件 ──
		{
			// 影片渲染管线：离线出片
			"Name": "MovieRenderPipeline",
			"Enabled": true
		},
		// ── 58 ── 引擎插件 ──
		{
			// 影片渲染的遮罩通道
			"Name": "MoviePipelineMaskRenderPass",
			"Enabled": true
		},
		// ── 59 ── 引擎插件 ──
		{
			// 资产搜索
			"Name": "AssetSearch",
			"Enabled": true
		},
		// ── 60 ── 引擎插件 ──
		{
			// 编辑器里可视化 GAS 运行状态（调试技能用）
			"Name": "GameplayInsights",
			"Enabled": true
		},
		// ── 61 ── 引擎插件 ──
		{
			// Google Resonance 空间音频
			"Name": "ResonanceAudio",
			"Enabled": false
		},
		// ── 62 ── ★ 本机没有此插件（Enabled=false 豁免）──
		{
			// 运行时 PhysX 烘焙（UE5 默认 Chaos，已移除）
			"Name": "RuntimePhysXCooking",
			"Enabled": false
		},
		// ── 63 ── 引擎插件 ──
		{
			// 空间音频
			"Name": "Spatialization",
			"Enabled": true
		},
		// ── 64 ── 引擎插件 ──
		{
			// 功能测试编辑器
			"Name": "FunctionalTestingEditor",
			"Enabled": true
		},
		// ── 65 ── 引擎插件 ──
		{
			// 玩法交互框架（SmartObject 交互）
			"Name": "GameplayInteractions",
			"Enabled": true
		},
		// ── 66 ── 引擎插件 ──
		{
			// 智能对象：AI 可交互点
			"Name": "SmartObjects",
			"Enabled": true
		},
		// ── 67 ── 引擎插件 ──
		{
			// 情境动画：翻越、开门
			"Name": "ContextualAnimation",
			"Enabled": true
		},
		// ── 68 ── 引擎插件 ──
		{
			// 玩法行为 + 智能对象桥接
			"Name": "GameplayBehaviorSmartObjects",
			"Enabled": true
		},
		// ── 69 ── 引擎插件 ──
		{
			// 玩法状态树
			"Name": "GameplayStateTree",
			"Enabled": true
		},
		// ── 70 ── 引擎插件 ──
		{
			// 玩法行为框架
			"Name": "GameplayBehaviors",
			"Enabled": true
		}
	],
	// Plugins 数组结束（共 70 项：59 引擎 + 9 工程 + 2 豁免；S23 再补 6 个示例玩法插件）
	// 示例工程哈希；Lyra 原样保留，不要改
	"EpicSampleNameHash": "451731683"
}
// 整个 JSON 对象结束
```---

## 三、敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件名 | `Tenkaichi.uproject`，在工程根目录 |
| `FileVersion` / `EngineAssociation` | `3` / `"5.4"` |
| `Modules` | **只有 1 项** `LyraGame`，`Type` 是 `Runtime`，`LoadingPhase` 是 `Default` |
| `AdditionalDependencies` | `DeveloperSettings` + `Engine` 两条 |
| `Plugins` 总数 | **70 项**（数一下 `"Name"` 出现次数，不含 `Modules` 里那个） |
| 插件 6 | `D3DExternalGPUStatistics` 必须带 `"Optional": true` |
| 插件 32 | `PlayFabParty` 必须带 `"PlatformAllowList": ["XB1","XSX"]` |
| 插件 21/22/23/26/27/28/29/30/62 | 9 个已废弃插件必须是 `"Enabled": false` |
| 工程插件 39~47 | 9 项 `Enabled: true`，且 `Tenkaichi\Plugins\` 下有对应 `.uplugin` |
| 语法 | 最后一个元素后**不能**有逗号；`true` / `false` 必须小写 |
| 缩进 | Tab（不是空格） |
| 注释 | 所有 `//` 开头的行**全部删掉**，一行都不留 |

### 敲完跑这个校验（PowerShell）

```powershell
$proj = "E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject"
$json = Get-Content $proj -Raw | ConvertFrom-Json
$engineNames = Get-ChildItem "D:\ue5\Epic Games\UE_5.4\Engine\Plugins" -Recurse -Filter *.uplugin -File -EA SilentlyContinue | ForEach-Object { $_.BaseName }
$projNames   = Get-ChildItem "E:\ue5\game_design\code\Tenkaichi\Plugins" -Recurse -Filter *.uplugin -File -EA SilentlyContinue | ForEach-Object { $_.BaseName }

"插件总数: " + $json.Plugins.Count

Write-Output "=== 会报错的（启用 + 不存在 + 无 Optional + 无平台白名单）==="
$json.Plugins | Where-Object {
    $_.Enabled -eq $true -and
    $engineNames -notcontains $_.Name -and
    $projNames   -notcontains $_.Name -and
    -not ($_.PSObject.Properties.Name -contains 'Optional') -and
    -not ($_.PSObject.Properties.Name -contains 'PlatformAllowList')
} | ForEach-Object { "  ERROR: " + $_.Name }
```

**期望输出**：`插件总数: 70`，`ERROR` 列表**为空**。

---

## 四、为什么是 70 项而不是 Lyra 的 76 项

| 差项 | 数量 | 原因 | 何时补 |
|---|---|---|---|
| `PocketWorlds` | 1 | 工程插件，源码 S05 才复刻 | **S05** |
| `ShooterCore` / `ShooterMaps` / `TopDownArena` / `ShooterExplorer` / `ShooterTests` | 5 | 示例玩法插件，源码 S23 才复刻 | **S23** |

> ⚠️ 现在写进 `.uproject` → 引擎在 `Plugins\` 下找不到对应 `.uplugin` → 报
> `X plugin is referenced by target but not found`。
>
> 📌 顺序必须是：**先建插件目录（含 `.uplugin`）→ 再往 `.uproject` 里加清单。**

### 那 9 个「本机没有但保留」的插件为什么不删

| 理由 | 说明 |
|---|---|
| 一比一 | Lyra 原文就是这么写的，删了偏离源 |
| 无害 | `Enabled: false` → 引擎压根不去找这些 `.uplugin` |
| 对照方便 | 以后换引擎版本（比如源码版 5.4）这些插件就有了，不用再改 |

---

## 五、S00 阶段已知会有的「预期报错」（不是你的错）

`.uproject` 敲完、双击启动，可能看到：

| 现象 | 原因 | 什么时候好 |
|---|---|---|
| `Could not find definition for module 'ModularGameplayActors'` (编译时) | `LyraGame.Build.cs` 依赖的插件模块 S01 才建 | S05 末 |
| `Asset manager settings do not include a rule for assets of type GameFeatureData` | `DefaultGame.ini` 缺规则 | **现在就能修**（见 `00_S00开工必读`） |
| `碰撞描述文件设置不包括水体碰撞描述文件的条目` | `DefaultEngine.ini` 缺 `WaterBodyCollision` | **现在就能修**（见 `00_S00开工必读`） |
| IDE 里 `#include "Modules/ModuleManager.h"` 标红 | UBT 编不过 → IntelliSense 拿不到引擎路径 | S05 末 |

---

## 源码依据

- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:1-347`（源全文：**2 个 Module / 76 项插件**）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:6-21`（`Modules` 段：`LyraGame` + `LyraEditor`）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:22-345`（`Plugins` 段 76 项）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:44-50`（`D3DExternalGPUStatistics` 带 `Optional:true`）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:193-204`（`PlayFabParty` 带 `PlatformAllowList`）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:346`（`EpicSampleNameHash`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins`（递归 620 个 `.uplugin`，逐个核对）
- `E:\ue5\game_design\code\Tenkaichi\Plugins`（9 个工程插件 `.uplugin`，逐个核对）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\System\NativeProjects.cs:351`（缺失插件报错原文）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\System\NativeProjects.cs:498-501`（`bOptional` 跳过逻辑）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\System\PluginReferenceDescriptor.cs:28-30,45,75`（三个字段的定义注释）