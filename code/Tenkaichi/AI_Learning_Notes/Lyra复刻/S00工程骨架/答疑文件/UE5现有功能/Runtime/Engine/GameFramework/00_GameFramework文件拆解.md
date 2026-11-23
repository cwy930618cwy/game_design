# GameFramework — 文件拆解（仿 `.uproject` 清单格式）

> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework`（**65** 个头文件）
> 以及 `...\Public\GameFramework`（**4** 个头文件）
> 格式参照：`..\..\02_Runtime177模块清单_仿uproject格式.md`（编号 + 中文注释独占一行 + 写在 `"Name"` 正上方）
>
> ⚠️ 教学整理清单，`Enabled: true` 只是格式占位，不是真实开关。

---

## 一、一句话

★ GameFramework 是 UE 游戏循环的骨架：`AActor` → `APawn` → `AController` → `AGameMode` → `AGameState`，Lyra 的 `ALyraCharacter` / `ALyraPlayerController` / `ALyraGameMode` 全是从这里长出来的。

---

## 二、`Classes/GameFramework` 的 65 个头文件

```jsonc
"Engine/Classes/GameFramework": [
	// ── 1 ── Actor 基础（4）──
	{
		// 场景里一切物体的基类：BeginPlay / Tick / 网络复制 / 组件挂载
		"Name": "Actor.h",
		"Enabled": true
	},
	{
		// Actor 编辑器里的颜色标记处理
		"Name": "ActorPrimitiveColorHandler.h",
		"Enabled": true
	},
	{
		// AInfo 基类：不渲染、只存数据的 Actor（如 AGameState / APlayerState 的祖先）
		"Name": "Info.h",
		"Enabled": true
	},
	{
		// 伤害类型基类：UDamageType
		"Name": "DamageType.h",
		"Enabled": true
	},

	// ── 2 ── Pawn 与角色（5）──
	{
		// 可被 Controller 操纵的实体基类：APawn
		"Name": "Pawn.h",
		"Enabled": true
	},
	{
		// 人形角色：ACharacter，自带 Capsule + SkeletalMesh + CharacterMovement
		"Name": "Character.h",
		"Enabled": true
	},
	{
		// 默认 Pawn（无网格的问题解决者）：ADefaultPawn
		"Name": "DefaultPawn.h",
		"Enabled": true
	},
	{
		// 观战者 Pawn：ASpectatorPawn
		"Name": "SpectatorPawn.h",
		"Enabled": true
	},
	{
		// 角色移动的网络复制逻辑
		"Name": "CharacterMovementReplication.h",
		"Enabled": true
	},

	// ── 3 ── 移动组件（9）──
	{
		// 移动组件基类：UMovementComponent
		"Name": "MovementComponent.h",
		"Enabled": true
	},
	{
		// Pawn 专用移动组件基类
		"Name": "PawnMovementComponent.h",
		"Enabled": true
	},
	{
		// 角色移动组件：走 / 跑 / 跳 / 游泳（Lyra 移动系统核心）
		"Name": "CharacterMovementComponent.h",
		"Enabled": true
	},
	{
		// 悬浮移动组件
		"Name": "FloatingPawnMovement.h",
		"Enabled": true
	},
	{
		// 导航移动组件（沿导航网格走）
		"Name": "NavMovementComponent.h",
		"Enabled": true
	},
	{
		// 观战者移动组件
		"Name": "SpectatorPawnMovement.h",
		"Enabled": true
	},
	{
		// 投射物移动组件
		"Name": "ProjectileMovementComponent.h",
		"Enabled": true
	},
	{
		// 旋转移动组件
		"Name": "RotatingMovementComponent.h",
		"Enabled": true
	},
	{
		// 根运动源（动画驱动的位移）
		"Name": "RootMotionSource.h",
		"Enabled": true
	},

	// ── 4 ── Controller 与输入（12）──
	{
		// 控制器基类：AController，拥有 Pawn 并处理输入
		"Name": "Controller.h",
		"Enabled": true
	},
	{
		// 玩家控制器：APlayerController，输入 → Pawn 的桥梁（Lyra 核心基类）
		"Name": "PlayerController.h",
		"Enabled": true
	},
	{
		// 作弊管理器：UCheatManager
		"Name": "CheatManager.h",
		"Enabled": true
	},
	{
		// 作弊管理器的宏定义
		"Name": "CheatManagerDefines.h",
		"Enabled": true
	},
	{
		// 玩家输入配置：UPlayerInput
		"Name": "PlayerInput.h",
		"Enabled": true
	},
	{
		// 输入设置：UInputSettings（项目 Input 配置对应的对象）
		"Name": "InputSettings.h",
		"Enabled": true
	},
	{
		// 输入设备蓝图函数库
		"Name": "InputDeviceLibrary.h",
		"Enabled": true
	},
	{
		// 输入设备属性定义
		"Name": "InputDeviceProperties.h",
		"Enabled": true
	},
	{
		// 输入设备属性句柄
		"Name": "InputDevicePropertyHandle.h",
		"Enabled": true
	},
	{
		// 输入设备子系统
		"Name": "InputDeviceSubsystem.h",
		"Enabled": true
	},
	{
		// 玩家静音列表
		"Name": "PlayerMuteList.h",
		"Enabled": true
	},
	{
		// 触屏界面：UTouchInterface
		"Name": "TouchInterface.h",
		"Enabled": true
	},

	// ── 5 ── GameMode 与世界状态（8）──
	{
		// 游戏模式基类（简版）：AGameModeBase
		"Name": "GameModeBase.h",
		"Enabled": true
	},
	{
		// 游戏模式（完整版，带比赛状态机）：AGameMode
		"Name": "GameMode.h",
		"Enabled": true
	},
	{
		// 游戏状态基类：AGameStateBase（所有人可见的全局数据）
		"Name": "GameStateBase.h",
		"Enabled": true
	},
	{
		// 游戏状态（完整版）：AGameState
		"Name": "GameState.h",
		"Enabled": true
	},
	{
		// 玩家状态：APlayerState（每个玩家一份，复制到所有客户端）
		"Name": "PlayerState.h",
		"Enabled": true
	},
	{
		// 游戏会话：AGameSession（登录 / 建房）
		"Name": "GameSession.h",
		"Enabled": true
	},
	{
		// 网络管理器：AGameNetworkManager
		"Name": "GameNetworkManager.h",
		"Enabled": true
	},
	{
		// 世界设置：AWorldSettings（关卡的 GameMode 覆盖等）
		"Name": "WorldSettings.h",
		"Enabled": true
	},

	// ── 6 ── HUD 与消息（6）──
	{
		// HUD 基类：AHUD（Canvas 绘制）
		"Name": "HUD.h",
		"Enabled": true
	},
	{
		// HUD 命中区域（鼠标点击检测）
		"Name": "HUDHitBox.h",
		"Enabled": true
	},
	{
		// 本地消息：ULocalMessage
		"Name": "LocalMessage.h",
		"Enabled": true
	},
	{
		// 调试文字绘制信息
		"Name": "DebugTextInfo.h",
		"Enabled": true
	},
	{
		// 引擎消息类型
		"Name": "EngineMessage.h",
		"Enabled": true
	},
	{
		// 简易准星
		"Name": "SimpleReticle.h",
		"Enabled": true
	},

	// ── 7 ── 存档与设置（3）──
	{
		// 存档对象基类：USaveGame
		"Name": "SaveGame.h",
		"Enabled": true
	},
	{
		// 异步存档/读档节点
		"Name": "AsyncActionHandleSaveGame.h",
		"Enabled": true
	},
	{
		// 用户设置：UGameUserSettings（画质 / 音量等）
		"Name": "GameUserSettings.h",
		"Enabled": true
	},

	// ── 8 ── 力反馈（3）──
	{
		// 力反馈效果资产
		"Name": "ForceFeedbackEffect.h",
		"Enabled": true
	},
	{
		// 力反馈衰减
		"Name": "ForceFeedbackAttenuation.h",
		"Enabled": true
	},
	{
		// 力反馈参数
		"Name": "ForceFeedbackParameters.h",
		"Enabled": true
	},

	// ── 9 ── 体积（6）──
	{
		// 体积基类：AVolume
		"Name": "Volume.h",
		"Enabled": true
	},
	{
		// 物理体积：APhysicsVolume（浮力 / 阻力）
		"Name": "PhysicsVolume.h",
		"Enabled": true
	},
	{
		// 默认物理体积
		"Name": "DefaultPhysicsVolume.h",
		"Enabled": true
	},
	{
		// 坠落死亡体积（掉出地图）
		"Name": "KillZVolume.h",
		"Enabled": true
	},
	{
		// 伤害体积（进入持续掉血）
		"Name": "PainCausingVolume.h",
		"Enabled": true
	},
	{
		// 相机阻挡体积
		"Name": "CameraBlockingVolume.h",
		"Enabled": true
	},

	// ── 10 ── 相机与关卡（3）──
	{
		// 弹簧臂组件：USpringArmComponent（第三人称相机挂点）
		"Name": "SpringArmComponent.h",
		"Enabled": true
	},
	{
		// 玩家出生点：APlayerStart
		"Name": "PlayerStart.h",
		"Enabled": true
	},
	{
		// 关卡可见性更新信息
		"Name": "UpdateLevelVisibilityLevelInfo.h",
		"Enabled": true
	},

	// ── 11 ── 轻量实例（4）──
	{
		// 轻量实例蓝图函数库
		"Name": "LightWeightInstanceBlueprintFunctionLibrary.h",
		"Enabled": true
	},
	{
		// 轻量实例管理器
		"Name": "LightWeightInstanceManager.h",
		"Enabled": true
	},
	{
		// 轻量实例化静态网格管理器
		"Name": "LightWeightInstanceStaticMeshManager.h",
		"Enabled": true
	},
	{
		// 轻量实例子系统
		"Name": "LightWeightInstanceSubsystem.h",
		"Enabled": true
	},

	// ── 12 ── 网络与在线（2）──
	{
		// 网络复制用的结构体
		"Name": "OnlineReplStructs.h",
		"Enabled": true
	},
	{
		// 在线会话相关结构
		"Name": "OnlineSession.h",
		"Enabled": true
	}
]
```

---

## 三、`Public/GameFramework` 的 4 个头文件（网络序列化器）

```jsonc
"Engine/Public/GameFramework": [
	{
		// 角色网络序列化打包位
		"Name": "CharacterNetworkSerializationPackedBitsNetSerializer.h",
		"Enabled": true
	},
	{
		// PlayerState 数量限制配置
		"Name": "PlayerStateCountLimiterConfig.h",
		"Enabled": true
	},
	{
		// 根运动源组的网络序列化器
		"Name": "RootMotionSourceGroupNetSerializer.h",
		"Enabled": true
	},
	{
		// 唯一网络 ID 复制的网络序列化器
		"Name": "UniqueNetIdReplNetSerializer.h",
		"Enabled": true
	}
]
```

---

## 四、复刻时最常翻的几个

| 头文件 | 什么时候查 |
|---|---|
| `Actor.h` | 任何 Actor 的生命周期 / 复制 / 组件接口 |
| `Pawn.h` / `Character.h` | 写角色、移动、 possession |
| `PlayerController.h` | 输入处理、相机管理、UI 层栈入口 |
| `GameModeBase.h` / `GameMode.h` | 比赛规则、登录与出生 |
| `PlayerState.h` | GAS 的 ASC 就挂在这（Lyra 的做法） |
| `CharacterMovementComponent.h` | 移动速度 / 跳跃 / 地面检测 |

---

## 五、注意

- 这 69 个文件里**真正要熟的只有 6 个**（见上表），剩下的是具体实现或边角功能，知道存在即可。
- `GameMode` 与 `GameModeBase`、`GameState` 与 `GameStateBase` 是「完整版 / 简版」两对，Lyra 用的是 Base 版为主。
- `Private\GameFramework` 里是对应 `.cpp` 实现，查函数体时才翻。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework`（65 个文件实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Public\GameFramework`（4 个文件实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\GameFramework`（实现文件，未逐项列）
