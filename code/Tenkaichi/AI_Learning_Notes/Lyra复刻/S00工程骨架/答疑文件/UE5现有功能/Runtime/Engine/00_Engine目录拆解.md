# Engine — 目录拆解（仿 `.uproject` 清单格式）

> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine` 真实扫描
> `Classes` 38 个 / `Public` 41 个 / `Private` 56 个 / `Internal` 3 个子目录
> 格式参照：`..\02_Runtime177模块清单_仿uproject格式.md`（编号 + 中文注释独占一行 + 写在 `"Name"` 正上方）
>
> ⚠️ 同样是教学整理清单，`Enabled: true` 只是格式占位，**不是真实开关**。

---

## 一、一句话

★ Engine 模块内部按「**声明放 Classes、辅助头放 Public、实现放 Private、内部头放 Internal**」四分。你查基类定义主要翻 `Classes`。

---

## 二、`Engine/Classes` 的 38 个子目录（UCLASS 声明都在这）

```jsonc
"Engine/Classes": [
	// ── 1 ── 游戏对象与框架（6）──
	{
		// 游戏对象基类：AActor / APawn / APlayerController / AGameModeBase
		"Name": "GameFramework",
		"Enabled": true
	},
	{
		// 引擎主对象：UEngine / UGameEngine / UWorld 定义
		"Name": "Engine",
		"Enabled": true
	},
	{
		// 组件体系：UActorComponent / USceneComponent / UPrimitiveComponent
		"Name": "Components",
		"Enabled": true
	},
	{
		// 各种接口类：IInputInterface / IConsoleManager 等
		"Name": "Interfaces",
		"Enabled": true
	},
	{
		// 蓝图可调用节点库：KismetSystemLibrary / KismetMathLibrary
		"Name": "Kismet",
		"Enabled": true
	},
	{
		// 引擎内部固有类型定义
		"Name": "Intrinsic",
		"Enabled": true
	},

	// ── 2 ── 相机与输入反馈（2）──
	{
		// 相机基类与相机组件：UCameraComponent / ACameraActor
		"Name": "Camera",
		"Enabled": true
	},
	{
		// 触觉反馈（手柄震动）设备接口
		"Name": "Haptics",
		"Enabled": true
	},

	// ── 3 ── AI（1）──
	{
		// AI 在 Engine 层的支撑类（行为树本体在 AIModule 插件）
		"Name": "AI",
		"Enabled": true
	},

	// ── 4 ── 动画（1）──
	{
		// 动画资产基类：UAnimationAsset / USkeletalMesh 动画相关
		"Name": "Animation",
		"Enabled": true
	},

	// ── 5 ── 物理与车辆（4）──
	{
		// 物理场景与刚体：FPhysScene / UBodySetup
		"Name": "PhysicsEngine",
		"Enabled": true
	},
	{
		// 物理场（力场）系统
		"Name": "PhysicsField",
		"Enabled": true
	},
	{
		// 物理材质资产：UPhysicalMaterial
		"Name": "PhysicalMaterials",
		"Enabled": true
	},
	{
		// 载具模拟：UVehicleMovementComponent
		"Name": "Vehicles",
		"Enabled": true
	},

	// ── 6 ── 渲染与视觉（10）──
	{
		// 材质系统：UMaterial / UMaterialInstance
		"Name": "Materials",
		"Enabled": true
	},
	{
		// 粒子系统（Cascade 老系统）：UParticleSystem
		"Name": "Particles",
		"Enabled": true
	},
	{
		// 大气与天空：USkyLightComponent / UAtmosphericFogComponent
		"Name": "Atmosphere",
		"Enabled": true
	},
	{
		// 向量场：UVectorFieldComponent
		"Name": "VectorField",
		"Enabled": true
	},
	{
		// 体积纹理（Volume Texture）
		"Name": "VT",
		"Enabled": true
	},
	{
		// 稀疏体积纹理
		"Name": "SparseVolumeTexture",
		"Enabled": true
	},
	{
		// Lightmass 光照烘焙相关
		"Name": "Lightmass",
		"Enabled": true
	},
	{
		// 曲线资产：UCurveFloat / UCurveVector
		"Name": "Curves",
		"Enabled": true
	},
	{
		// 分布（Distribution）类型，曲线/参数的旧式封装
		"Name": "Distributions",
		"Enabled": true
	},
	{
		// 场景图层：ULayer / ULayerManager
		"Name": "Layers",
		"Enabled": true
	},

	// ── 7 ── 音频（2）──
	{
		// 音频组件基类：UAudioComponent
		"Name": "Audio",
		"Enabled": true
	},
	{
		// 声音资产：USoundBase / USoundWave / USoundCue
		"Name": "Sound",
		"Enabled": true
	},

	// ── 8 ── 网络（1）──
	{
		// 网络封包处理器接口
		"Name": "PacketHandlers",
		"Enabled": true
	},

	// ── 9 ── Slate UI（1）──
	{
		// Engine 里的 Slate 控件封装（Slate 本体在 Runtime\Slate）
		"Name": "Slate",
		"Enabled": true
	},

	// ── 10 ── 编辑器与工具（8）──
	{
		// 蓝图图表节点：UEdGraph / UEdGraphNode
		"Name": "EdGraph",
		"Enabled": true
	},
	{
		// 编辑器框架在 Engine 层的支撑类
		"Name": "EditorFramework",
		"Enabled": true
	},
	{
		// 命令行工具（Commandlet）基类
		"Name": "Commandlets",
		"Enabled": true
	},
	{
		// 资产导出器：UExporter
		"Name": "Exporters",
		"Enabled": true
	},
	{
		// 网格顶点颜色绘制工具
		"Name": "MeshVertexPainter",
		"Enabled": true
	},
	{
		// 调试辅助类
		"Name": "Debug",
		"Enabled": true
	},
	{
		// 设备配置：UDeviceProfile
		"Name": "DeviceProfiles",
		"Enabled": true
	},
	{
		// 数据驱动 CVar 配置
		"Name": "DataDrivenCVars",
		"Enabled": true
	},

	// ── 11 ── 调试与测试（2）──
	{
		// 可视化日志：Visual Logger
		"Name": "VisualLogger",
		"Enabled": true
	},
	{
		// 引擎层测试类
		"Name": "Tests",
		"Enabled": true
	}
]
```

---

## 三、`Engine/Public` 的 41 个子目录（辅助头 / 子系统）

```jsonc
"Engine/Public": [
	// ── 1 ── 游戏对象与框架（9）──
	{
		// 引擎主头文件与全局辅助
		"Name": "Engine",
		"Enabled": true
	},
	{
		// 游戏框架辅助头（Actor / Controller 的 inline 实现）
		"Name": "GameFramework",
		"Enabled": true
	},
	{
		// 子系统：UGameInstanceSubsystem / UWorldSubsystem / ULocalPlayerSubsystem
		"Name": "Subsystems",
		"Enabled": true
	},
	{
		// 蓝图相关辅助头
		"Name": "Blueprint",
		"Enabled": true
	},
	{
		// Actor 分区（ActorPartition 模式）
		"Name": "ActorPartition",
		"Enabled": true
	},
	{
		// 关卡实例：ALevelInstance
		"Name": "LevelInstance",
		"Enabled": true
	},
	{
		// 新一代对象元素系统（TypedElement 的引擎侧）
		"Name": "Elements",
		"Enabled": true
	},
	{
		// 字段通知（属性变更回调）
		"Name": "FieldNotification",
		"Enabled": true
	},
	{
		// 通用对象定位器（软引用 / 跨关卡定位）
		"Name": "UniversalObjectLocators",
		"Enabled": true
	},

	// ── 2 ── 世界分区与流式加载（5）──
	{
		// 世界分区：UWorldPartition
		"Name": "WorldPartition",
		"Enabled": true
	},
	{
		// 实例化静态网格的分区支持
		"Name": "ISMPartition",
		"Enabled": true
	},
	{
		// Actor 编辑器上下文
		"Name": "ActorEditorContext",
		"Enabled": true
	},
	{
		// 打包关卡 Actor：APackedLevelActor
		"Name": "PackedLevelActor",
		"Enabled": true
	},
	{
		// 资源流式加载控制
		"Name": "Streaming",
		"Enabled": true
	},

	// ── 3 ── 实例化与 HLOD（3）──
	{
		// 实例化静态网格组件：UInstancedStaticMeshComponent
		"Name": "InstancedStaticMesh",
		"Enabled": true
	},
	{
		// 实例通用数据结构
		"Name": "Instances",
		"Enabled": true
	},
	{
		// 分层细节层级：HLOD
		"Name": "HLOD",
		"Enabled": true
	},

	// ── 4 ── AI 与动画（2）──
	{
		// Engine 侧 AI 辅助头
		"Name": "AI",
		"Enabled": true
	},
	{
		// 动画辅助头（运行时动画压缩等）
		"Name": "Animation",
		"Enabled": true
	},

	// ── 5 ── 渲染与材质（7）──
	{
		// 材质辅助头
		"Name": "Materials",
		"Enabled": true
	},
	{
		// 渲染相关引擎侧接口
		"Name": "Rendering",
		"Enabled": true
	},
	{
		// 着色器管线辅助头
		"Name": "Shader",
		"Enabled": true
	},
	{
		// HLSL 语法树（材质表达式编译）
		"Name": "HLSLTree",
		"Enabled": true
	},
	{
		// 稀疏体积纹理辅助头
		"Name": "SparseVolumeTexture",
		"Enabled": true
	},
	{
		// 体积纹理辅助头
		"Name": "VT",
		"Enabled": true
	},
	{
		// 按需着色器编译（ODSC）
		"Name": "ODSC",
		"Enabled": true
	},

	// ── 6 ── 物理（1）──
	{
		// 物理辅助头（碰撞查询接口等）
		"Name": "Physics",
		"Enabled": true
	},

	// ── 7 ── 网络（2）──
	{
		// 网络复制辅助头
		"Name": "Net",
		"Enabled": true
	},
	{
		// 网络封包处理器辅助头
		"Name": "PacketHandlers",
		"Enabled": true
	},

	// ── 8 ── 音频（1）──
	{
		// 音频辅助头
		"Name": "Audio",
		"Enabled": true
	},

	// ── 9 ── 性能与调试（4）──
	{
		// 统计上报与分析辅助头
		"Name": "Analytics",
		"Enabled": true
	},
	{
		// 性能相关辅助头
		"Name": "Performance",
		"Enabled": true
	},
	{
		// 性能剖析辅助头
		"Name": "ProfilingDebugging",
		"Enabled": true
	},
	{
		// 可视化日志辅助头
		"Name": "VisualLogger",
		"Enabled": true
	},

	// ── 10 ── 国际化（1）──
	{
		// 引擎侧国际化辅助头
		"Name": "Internationalization",
		"Enabled": true
	},

	// ── 11 ── 编辑器与工具（4）──
	{
		// 命令行工具辅助头
		"Name": "Commandlets",
		"Enabled": true
	},
	{
		// 蓝图图表辅助头
		"Name": "EdGraph",
		"Enabled": true
	},
	{
		// 曲线辅助头
		"Name": "Curves",
		"Enabled": true
	},
	{
		// 杂项工具头
		"Name": "Misc",
		"Enabled": true
	},

	// ── 12 ── Slate UI（1）──
	{
		// Engine 侧 Slate 辅助头
		"Name": "Slate",
		"Enabled": true
	},

	// ── 13 ── 测试（1）──
	{
		// 引擎层测试辅助头
		"Name": "Tests",
		"Enabled": true
	}
]
```

---

## 四、剩下两个目录（不逐项列）

| 目录 | 数量 | 说明 |
|---|---|---|
| `Private` | 56 个子目录 | 具体实现 `.cpp`，查实现细节时才翻；结构与 Classes/Public 大致对应 |
| `Internal` | 3 个子目录 | `Kismet` / `PhysicsEngine` / `Streaming`，模块内部专用头，外部不能 include |

---

## 五、注意

- **查类定义优先翻 `Classes`**：`Actor.h` 在 `Classes\GameFramework\`，不在 `Public\`（上一条已经踩过这个坑）。
- `Public` 里的同名目录（如 `GameFramework`、`Materials`）是**辅助头**，和 `Classes` 里的**类声明**不是一回事。
- `Internal` 里的头外部模块不能 include，看到了也别照抄。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes`（38 个子目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Public`（41 个子目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private`（56 个子目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Internal`（3 个子目录实查）
