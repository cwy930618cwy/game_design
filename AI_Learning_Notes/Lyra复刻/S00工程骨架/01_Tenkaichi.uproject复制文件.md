# 01 — `Tenkaichi.uproject` 复制文件

> 依据：`E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（**347 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**（JSON 不支持注释）。

---

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
	// Modules 数组结束（S00 只有 LyraGame；LyraEditor 在 S22 补）
	// 插件数组；每一项是一个插件开关
	"Plugins": [
		// ── 插件 1 ──
		{
			// Actor 调色板：编辑器里把常用 Actor 存成面板，一键拖进场景
			"Name": "ActorPalette",
			// true = 启用
			"Enabled": true
		},
		// ── 插件 2 ──
		{
			// 网络加密组件（AES-GCM 算法），防抓包篡改
			"Name": "AESGCMHandlerComponent",
			"Enabled": true
		},
		// ── 插件 3 ──
		{
			// 网络加密组件（DTLS 协议），用于语音/UDP 加密
			"Name": "DTLSHandlerComponent",
			"Enabled": true
		},
		// ── 插件 4 ──
		{
			// ★ GAS 插件：技能、属性、GameplayEffect、GameplayCue 全在这
			"Name": "GameplayAbilities",
			// 必须启用，否则 Build.cs 的 GameplayAbilities 依赖找不到
			"Enabled": true
		},
		// ── 插件 5 ──
		{
			// 自动化测试框架，CI 无人值守跑测试用
			"Name": "Gauntlet",
			"Enabled": true
		},
		// ── 插件 6 ──
		{
			// D3D 外接显卡性能统计
			"Name": "D3DExternalGPUStatistics",
			"Enabled": true,
			// Optional = 找不到这个插件也不报错（平台专属插件常用）
			"Optional": true,
			// 限定只在下列平台启用
			"SupportedTargetPlatforms": [
				// 仅 Windows 64 位
				"Win64"
			]
		},
		// ── 插件 7 ──
		{
			// NPC 对话树系统（对话编辑器 + 运行时）
			"Name": "CommonConversation",
			"Enabled": true
		},
		// ── 插件 8 ──
		{
			// ★ GameFeature 机制：把玩法做成可插拔功能包，运行时加载/卸载
			"Name": "GameFeatures",
			// Lyra 的 Experience 系统靠它加载玩法插件
			"Enabled": true
		},
		// ── 插件 9 ──
		{
			// ★ 模块化 Gameplay：提供 UGameFrameworkComponentManager 和初始化状态机
			"Name": "ModularGameplay",
			// ALyraPawn 的基类 AModularPawn 来自 ModularGameplayActors，后者依赖它
			"Enabled": true
		},
		// ── 插件 10 ──
		{
			// ★ 增强输入系统：UInputAction / UInputMappingContext
			"Name": "EnhancedInput",
			// LyraGame.Build.cs 依赖它
			"Enabled": true
		},
		// ── 插件 11 ──
		{
			// PS4/PS5 手柄支持
			"Name": "WinDualShock",
			"Enabled": true,
			"SupportedTargetPlatforms": [
				// 仅 Windows
				"Win64"
			]
		},
		// ── 插件 12 ──
		{
			// 体积云 / 体积雾
			"Name": "Volumetrics",
			"Enabled": true
		},
		// ── 插件 13 ──
		{
			// 数据注册表：按 ID 查资产，Lyra 用它管理 GameplayCue 等
			"Name": "DataRegistry",
			"Enabled": true
		},
		// ── 插件 14 ──
		{
			// ★ 网络复制图：按空间网格决定谁同步给谁，省带宽
			"Name": "ReplicationGraph",
			// LyraReplicationGraph 依赖它
			"Enabled": true
		},
		// ── 插件 15 ──
		{
			// 重要性管理器：远处 Actor 降频更新（不跑 AI、不更新动画）
			"Name": "SignificanceManager",
			"Enabled": true
		},
		// ── 插件 16 ──
		{
			// 粒子系统（VFX）
			"Name": "Niagara",
			"Enabled": true
		},
		// ── 插件 17 ──
		{
			// 水体系统
			"Name": "Water",
			"Enabled": true
		},
		// ── 插件 18 ──
		{
			// ★ CommonUI 框架：手柄/键鼠/触屏统一的 UI 输入与焦点管理
			"Name": "CommonUI",
			// Lyra 全部 UI 的地基
			"Enabled": true
		},
		// ── 插件 19 ──
		{
			// 控制流：节点图执行框架
			"Name": "ControlFlows",
			"Enabled": true
		},
		// ── 插件 20 ──
		{
			// 新一代音频引擎（MetaSounds），替代旧 SoundCue
			"Name": "Metasound",
			"Enabled": true
		},
		// ── 插件 21 ──
		{
			// MagicLeap AR 平台支持
			"Name": "MagicLeap",
			// 禁用：保留条目方便以后开
			"Enabled": false
		},
		// ── 插件 22 ──
		{
			// MagicLeap 媒体播放
			"Name": "MagicLeapMedia",
			"Enabled": false
		},
		// ── 插件 23 ──
		{
			// MagicLeap 可通行世界
			"Name": "MagicLeapPassableWorld",
			"Enabled": false
		},
		// ── 插件 24 ──
		{
			// OpenXR 眼动追踪
			"Name": "OpenXREyeTracker",
			"Enabled": false
		},
		// ── 插件 25 ──
		{
			// OpenXR 手部追踪
			"Name": "OpenXRHandTracking",
			"Enabled": false
		},
		// ── 插件 26 ──
		{
			// OpenXR 头显
			"Name": "OpenXRHMD",
			"Enabled": false
		},
		// ── 插件 27 ──
		{
			// SteamVR 支持
			"Name": "SteamVR",
			"Enabled": false
		},
		// ── 插件 28 ──
		{
			// 三星 GearVR 支持
			"Name": "GearVR",
			"Enabled": false
		},
		// ── 插件 29 ──
		{
			// MagicLeap Lumin 平台特性
			"Name": "LuminPlatformFeatures",
			"Enabled": false,
			"SupportedTargetPlatforms": [
				// 仅 Lumin 平台
				"Lumin"
			]
		},
		// ── 插件 30 ──
		{
			// MagicLeap SDK
			"Name": "MLSDK",
			"Enabled": false
		},
		// ── 插件 31 ──
		{
			// 在线框架：大厅、会话、匹配
			"Name": "OnlineFramework",
			"Enabled": true
		},
		// ── 插件 32 ──
		{
			// PlayFab 语音/网络
			"Name": "PlayFabParty",
			"Enabled": true,
			// 平台白名单：只有列出的平台才允许启用
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
		// ── 插件 33 ──
		{
			// Epic Online Services 旧接口（OnlineSubsystem 系列）
			"Name": "OnlineSubsystemEOS",
			"Enabled": true
		},
		// ── 插件 34 ──
		{
			// EOS 新接口（OnlineServices 系列）
			"Name": "OnlineServicesEOS",
			"Enabled": true
		},
		// ── 插件 35 ──
		{
			// 空实现：离线测试用，不连任何在线服务
			"Name": "OnlineServicesNull",
			"Enabled": true
		},
		// ── 插件 36 ──
		{
			// 把旧 OSS 接口适配到新 OnlineServices
			"Name": "OnlineServicesOSSAdapter",
			"Enabled": true
		},
		// ── 插件 37 ──
		{
			// Steam 平台接口
			"Name": "OnlineSubsystemSteam",
			"Enabled": true
		},
		// ── 插件 38 ──
		{
			// Steam 网络套接字
			"Name": "SteamSockets",
			"Enabled": true
		},
		// ── 插件 39 ──
		{
			// 资产引用限制规则：防止跨模块乱引用
			"Name": "AssetReferenceRestrictions",
			"Enabled": true
		},
		// ── 插件 40 ──
		{
			// 建模工具编辑器模式
			"Name": "ModelingToolsEditorMode",
			"Enabled": true
		},
		// ── 插件 41 ──
		{
			// 几何脚本：程序化建模
			"Name": "GeometryScripting",
			"Enabled": true
		},
		// ── 插件 42 ──
		{
			// 动画移动库：步态混合
			"Name": "AnimationLocomotionLibrary",
			"Enabled": true
		},
		// ── 插件 43 ──
		{
			// 音频调制：参数化混音
			"Name": "AudioModulation",
			"Enabled": true
		},
		// ── 插件 44 ──
		{
			// 音频玩法体积：区域混响
			"Name": "AudioGameplayVolume",
			"Enabled": true
		},
		// ── 插件 45 ──
		{
			// 音频玩法组件
			"Name": "AudioGameplay",
			"Enabled": true
		},
		// ── 插件 46 ──
		{
			// 音频工具函数库
			"Name": "SoundUtilities",
			"Enabled": true
		},
		// ── 插件 47 ──
		{
			// 动画扭曲：脚部贴地、朝向修正
			"Name": "AnimationWarping",
			"Enabled": true
		},
		// ── 插件 48 ──
		{
			// 影片渲染管线：离线出片
			"Name": "MovieRenderPipeline",
			"Enabled": true
		},
		// ── 插件 49 ──
		{
			// 影片渲染的遮罩通道
			"Name": "MoviePipelineMaskRenderPass",
			"Enabled": true
		},
		// ── 插件 50 ──
		{
			// 资产搜索
			"Name": "AssetSearch",
			"Enabled": true
		},
		// ── 插件 51 ──
		{
			// 编辑器里可视化 GAS 运行状态（调试技能用）
			"Name": "GameplayInsights",
			"Enabled": true
		},
		// ── 插件 52 ──
		{
			// Google Resonance 空间音频
			"Name": "ResonanceAudio",
			"Enabled": false
		},
		// ── 插件 53 ──
		{
			// 运行时 PhysX 烘焙
			"Name": "RuntimePhysXCooking",
			"Enabled": false
		},
		// ── 插件 54 ──
		{
			// 空间音频
			"Name": "Spatialization",
			"Enabled": true
		},
		// ── 插件 55 ──
		{
			// 功能测试编辑器
			"Name": "FunctionalTestingEditor",
			"Enabled": true
		},
		// ── 插件 56 ──
		{
			// 玩法交互框架（SmartObject 交互）
			"Name": "GameplayInteractions",
			"Enabled": true
		},
		// ── 插件 57 ──
		{
			// 智能对象：AI 可交互点
			"Name": "SmartObjects",
			"Enabled": true
		},
		// ── 插件 58 ──
		{
			// 情境动画：翻越、开门
			"Name": "ContextualAnimation",
			"Enabled": true
		},
		// ── 插件 59 ──
		{
			// 玩法行为 + 智能对象桥接
			"Name": "GameplayBehaviorSmartObjects",
			"Enabled": true
		},
		// ── 插件 60 ──
		{
			// 玩法状态树
			"Name": "GameplayStateTree",
			"Enabled": true
		},
		// ── 插件 61 ──
		{
			// 玩法行为框架
			"Name": "GameplayBehaviors",
			"Enabled": true
		}
	],
	// Plugins 数组结束（共 61 项，全部是引擎自带插件）
	// 示例工程哈希；Lyra 原样保留，不要改
	"EpicSampleNameHash": "451731683"
}
// 整个 JSON 对象结束
```

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件名 | `Tenkaichi.uproject`，在工程根目录 |
| `Modules` | 只有 `LyraGame` 一项，`Type` 是 `Runtime` |
| `Plugins` | 61 项，**没有** `AsyncMixin` / `CommonGame` / `GameSettings` 等项目插件 |
| 关键插件 | 有 `GameplayAbilities` / `CommonUI` / `EnhancedInput` / `ModularGameplay` / `GameFeatures` |
| 语法 | 最后一个元素后**不能**有逗号；`true`/`false` 必须小写 |
| 缩进 | Tab（不是空格） |
| 注释 | 所有 `//` 开头的行**全部删掉**，一行都不留 |

---

## 源码依据

- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:1-347`（全文）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:6-21`（`Modules` 段，含 `LyraEditor`）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:22-345`（`Plugins` 段，76 项）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:346`（`EpicSampleNameHash`）