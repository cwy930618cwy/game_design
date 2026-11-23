# Runtime 177 个模块清单（仿 `.uproject` 插件数组写法）

> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime` 真实扫描（177 个模块目录）
> 参考写法：`S00工程骨架\01_Tenkaichi.uproject复制文件.md`（编号 + 中文注释独占一行 + 写在代码上方）
>
> ⚠️ **这是教学整理清单，不是真实配置文件**。引擎模块不像插件那样由 ini 的 `Enabled` 开关控制；
> 这里的 `"Enabled": true` 仅表示「引擎自带、默认可用」，照抄格式用，不要拿去当配置写进工程。

---

## 完整清单（177 项，按 12 类分组）

```jsonc
// 引擎运行时模块数组；以下为教学整理格式，非真实配置文件
"Modules": [
	// ── 1 ── 核心地基（5）──
	{
		// 最底层工具箱：容器、字符串、数学、委托、日志、平台抽象
		"Name": "Core",
		"Enabled": true
	},
	{
		// UObject 反射与垃圾回收：UCLASS / UPROPERTY 的底层支撑
		"Name": "CoreUObject",
		"Enabled": true
	},
	{
		// 游戏运行时主体：AActor / APawn / UWorld / 组件系统
		"Name": "Engine",
		"Enabled": true
	},
	{
		// 应用外壳：命令行参数、平台抽象、应用生命周期
		"Name": "ApplicationCore",
		"Enabled": true
	},
	{
		// 程序入口：GuardedMain 与引擎主循环
		"Name": "Launch",
		"Enabled": true
	},

	// ── 2 ── 渲染与图像（17）──
	{
		// 渲染器主体：可见性剔除、光照、后处理
		"Name": "Renderer",
		"Enabled": true
	},
	{
		// 渲染核心：渲染资源管理、着色器绑定
		"Name": "RenderCore",
		"Enabled": true
	},
	{
		// 渲染硬件接口抽象层（Render Hardware Interface）
		"Name": "RHI",
		"Enabled": true
	},
	{
		// RHI 的公共实现基础
		"Name": "RHICore",
		"Enabled": true
	},
	{
		// DirectX 12 渲染后端
		"Name": "D3D12RHI",
		"Enabled": true
	},
	{
		// Vulkan 渲染后端
		"Name": "VulkanRHI",
		"Enabled": true
	},
	{
		// OpenGL 渲染后端（移动端 / 兼容设备）
		"Name": "OpenGLDrv",
		"Enabled": true
	},
	{
		// 空渲染后端，没有图形设备时使用
		"Name": "NullDrv",
		"Enabled": true
	},
	{
		// 图像格式与色彩空间基础类型
		"Name": "ImageCore",
		"Enabled": true
	},
	{
		// PNG / JPG / BMP 等图片编解码封装
		"Name": "ImageWrapper",
		"Enabled": true
	},
	{
		// 异步图片写入队列（截图 / 导出用）
		"Name": "ImageWriteQueue",
		"Enabled": true
	},
	{
		// 色彩管理：色彩空间转换与 OCIO 配置
		"Name": "ColorManagement",
		"Enabled": true
	},
	{
		// Open Color IO 第三方库封装
		"Name": "OpenColorIOWrapper",
		"Enabled": true
	},
	{
		// 贴图压缩与 mipmap 生成工具
		"Name": "TextureUtilitiesCommon",
		"Enabled": true
	},
	{
		// 材质着色器质量分级设置
		"Name": "MaterialShaderQualitySettings",
		"Enabled": true
	},
	{
		// CUDA 计算支持（NNE 的后端之一）
		"Name": "CUDA",
		"Enabled": true
	},
	{
		// 神经网络推理引擎（Neural Network Engine）
		"Name": "NNE",
		"Enabled": true
	},

	// ── 3 ── UI、窗口与输入（10）──
	{
		// UE 的 C++ UI 框架底层
		"Name": "Slate",
		"Enabled": true
	},
	{
		// Slate 核心：控件、布局、绘制、事件
		"Name": "SlateCore",
		"Enabled": true
	},
	{
		// Slate 的硬件渲染后端
		"Name": "SlateRHIRenderer",
		"Enabled": true
	},
	{
		// Slate 的空渲染后端
		"Name": "SlateNullRenderer",
		"Enabled": true
	},
	{
		// Unreal Motion Graphics：基于 Slate 的蓝图 UI 系统
		"Name": "UMG",
		"Enabled": true
	},
	{
		// 高级控件库（树、列表等）
		"Name": "AdvancedWidgets",
		"Enabled": true
	},
	{
		// 应用框架：窗口、菜单、多窗口管理
		"Name": "AppFramework",
		"Enabled": true
	},
	{
		// 轮播控件（CommonUI 的 WidgetCarousel 依赖它）
		"Name": "WidgetCarousel",
		"Enabled": true
	},
	{
		// 输入基础类型：按键、输入事件
		"Name": "InputCore",
		"Enabled": true
	},
	{
		// 输入设备抽象层
		"Name": "InputDevice",
		"Enabled": true
	},

	// ── 4 ── 音频与媒体（27）──
	{
		// 音频扩展接口：音频设备抽象
		"Name": "AudioExtensions",
		"Enabled": true
	},
	{
		// 音频混音器主模块
		"Name": "AudioMixer",
		"Enabled": true
	},
	{
		// 混音器核心算法与混音图
		"Name": "AudioMixerCore",
		"Enabled": true
	},
	{
		// 音频分析（FFT 等）
		"Name": "AudioAnalyzer",
		"Enabled": true
	},
	{
		// 音频采集抽象
		"Name": "AudioCaptureCore",
		"Enabled": true
	},
	{
		// 各平台音频采集的具体实现
		"Name": "AudioCaptureImplementations",
		"Enabled": true
	},
	{
		// 音频链接（跨应用音频同步）
		"Name": "AudioLink",
		"Enabled": true
	},
	{
		// 各平台音频配置
		"Name": "AudioPlatformConfiguration",
		"Enabled": true
	},
	{
		// 信号处理基础（DSP）
		"Name": "SignalProcessing",
		"Enabled": true
	},
	{
		// 声场渲染（环绕声 / 空间音频）
		"Name": "SoundFieldRendering",
		"Enabled": true
	},
	{
		// 离线音频渲染（烘焙用）
		"Name": "NonRealtimeAudioRenderer",
		"Enabled": true
	},
	{
		// 音频合成性能基准测试
		"Name": "SynthBenchmark",
		"Enabled": true
	},
	{
		// ADPCM 音频解码
		"Name": "AdpcmAudioDecoder",
		"Enabled": true
	},
	{
		// Bink 视频音频解码
		"Name": "BinkAudioDecoder",
		"Enabled": true
	},
	{
		// Opus 音频解码
		"Name": "OpusAudioDecoder",
		"Enabled": true
	},
	{
		// RAD 音频编解码（语音用）
		"Name": "RadAudioCodec",
		"Enabled": true
	},
	{
		// WAV 音频解码
		"Name": "UEWavComp",
		"Enabled": true
	},
	{
		// Vorbis / OGG 音频解码
		"Name": "VorbisAudioDecoder",
		"Enabled": true
	},
	{
		// 音视频编码器
		"Name": "AVEncoder",
		"Enabled": true
	},
	{
		// AVI 视频写入
		"Name": "AVIWriter",
		"Enabled": true
	},
	{
		// 媒体播放基础框架
		"Name": "Media",
		"Enabled": true
	},
	{
		// 媒体资产（MediaPlayer / MediaTexture）
		"Name": "MediaAssets",
		"Enabled": true
	},
	{
		// 媒体工具函数
		"Name": "MediaUtils",
		"Enabled": true
	},
	{
		// 电影 / 视频播放器
		"Name": "MoviePlayer",
		"Enabled": true
	},
	{
		// 播放器的代理封装
		"Name": "MoviePlayerProxy",
		"Enabled": true
	},
	{
		// 游戏画面录制编码
		"Name": "GameplayMediaEncoder",
		"Enabled": true
	},
	{
		// IES 灯光文件格式解析
		"Name": "IESFile",
		"Enabled": true
	},

	// ── 5 ── 动画与布料（9）──
	{
		// 动画基础类型与骨骼结构
		"Name": "AnimationCore",
		"Enabled": true
	},
	{
		// 动画蓝图节点运行时
		"Name": "AnimGraphRuntime",
		"Enabled": true
	},
	{
		// 布料模拟公共代码
		"Name": "ClothingSystemRuntimeCommon",
		"Enabled": true
	},
	{
		// 布料系统接口
		"Name": "ClothingSystemRuntimeInterface",
		"Enabled": true
	},
	{
		// Nvidia PhysX 布料实现
		"Name": "ClothingSystemRuntimeNv",
		"Enabled": true
	},
	{
		// LiveLink 动画数据核心
		"Name": "LiveLinkAnimationCore",
		"Enabled": true
	},
	{
		// LiveLink 接口（外部动捕数据接入）
		"Name": "LiveLinkInterface",
		"Enabled": true
	},
	{
		// LiveLink 消息总线框架
		"Name": "LiveLinkMessageBusFramework",
		"Enabled": true
	},
	{
		// 动画时间管理
		"Name": "TimeManagement",
		"Enabled": true
	},

	// ── 6 ── 物理与几何（12）──
	{
		// 物理基础类型与接口
		"Name": "PhysicsCore",
		"Enabled": true
	},
	{
		// 几何数据结构基础
		"Name": "GeometryCore",
		"Enabled": true
	},
	{
		// 几何框架（网格描述 / 几何脚本基础）
		"Name": "GeometryFramework",
		"Enabled": true
	},
	{
		// 字段通知（属性变更通知）
		"Name": "FieldNotification",
		"Enabled": true
	},
	{
		// 网格描述数据结构
		"Name": "MeshDescription",
		"Enabled": true
	},
	{
		// 骨骼网格描述
		"Name": "SkeletalMeshDescription",
		"Enabled": true
	},
	{
		// 静态网格描述
		"Name": "StaticMeshDescription",
		"Enabled": true
	},
	{
		// 旧版原始网格数据（兼容用）
		"Name": "RawMesh",
		"Enabled": true
	},
	{
		// 网格格式转换
		"Name": "MeshConversion",
		"Enabled": true
	},
	{
		// 网格转换涉及的引擎类型
		"Name": "MeshConversionEngineTypes",
		"Enabled": true
	},
	{
		// 网格工具函数
		"Name": "MeshUtilitiesCommon",
		"Enabled": true
	},
	{
		// MR 网格（Lumen / 几何体用）
		"Name": "MRMesh",
		"Enabled": true
	},

	// ── 7 ── 玩法与场景（15）──
	{
		// GameplayTag 系统（Lyra 的核心依赖）
		"Name": "GameplayTags",
		"Enabled": true
	},
	{
		// 玩法任务系统
		"Name": "GameplayTasks",
		"Enabled": true
	},
	{
		// 玩法调试器（游戏内调试叠加层）
		"Name": "GameplayDebugger",
		"Enabled": true
	},
	{
		// AI 模块：行为树、黑板、感知
		"Name": "AIModule",
		"Enabled": true
	},
	{
		// 导航系统（寻路）
		"Name": "NavigationSystem",
		"Enabled": true
	},
	{
		// 导航网格数据
		"Name": "Navmesh",
		"Enabled": true
	},
	{
		// 植被系统
		"Name": "Foliage",
		"Enabled": true
	},
	{
		// 地形系统
		"Name": "Landscape",
		"Enabled": true
	},
	{
		// 关卡序列（过场动画）
		"Name": "LevelSequence",
		"Enabled": true
	},
	{
		// 时间轴场景数据基础
		"Name": "MovieScene",
		"Enabled": true
	},
	{
		// 各种时间轴轨道实现
		"Name": "MovieSceneTracks",
		"Enabled": true
	},
	{
		// 序列录制捕获
		"Name": "MovieSceneCapture",
		"Enabled": true
	},
	{
		// 电影级相机
		"Name": "CinematicCamera",
		"Enabled": true
	},
	{
		// 虚拟制片（nDisplay 等）
		"Name": "VirtualProduction",
		"Enabled": true
	},
	{
		// 游戏菜单构建工具
		"Name": "GameMenuBuilder",
		"Enabled": true
	},

	// ── 8 ── 网络与在线（19）──
	{
		// 网络复制核心（Actor 同步、RPC）
		"Name": "Net",
		"Enabled": true
	},
	{
		// 网络底层设施
		"Name": "Networking",
		"Enabled": true
	},
	{
		// 套接字封装
		"Name": "Sockets",
		"Enabled": true
	},
	{
		// 消息总线
		"Name": "Messaging",
		"Enabled": true
	},
	{
		// 消息公共类型
		"Name": "MessagingCommon",
		"Enabled": true
	},
	{
		// 消息 RPC
		"Name": "MessagingRpc",
		"Enabled": true
	},
	{
		// 会话消息
		"Name": "SessionMessages",
		"Enabled": true
	},
	{
		// 会话服务
		"Name": "SessionServices",
		"Enabled": true
	},
	{
		// 在线服务基础接口
		"Name": "Online",
		"Enabled": true
	},
	{
		// 在线核心类型
		"Name": "CoreOnline",
		"Enabled": true
	},
	{
		// 好友与聊天
		"Name": "FriendsAndChat",
		"Enabled": true
	},
	{
		// 网络封包处理器
		"Name": "PacketHandlers",
		"Enabled": true
	},
	{
		// 网络回放流
		"Name": "NetworkReplayStreaming",
		"Enabled": true
	},
	{
		// 自动化测试消息
		"Name": "AutomationMessages",
		"Enabled": true
	},
	{
		// 自动化测试执行器
		"Name": "AutomationWorker",
		"Enabled": true
	},
	{
		// 自动化客户端驾驶
		"Name": "ClientPilot",
		"Enabled": true
	},
	{
		// 外部 RPC 注册
		"Name": "ExternalRPCRegistry",
		"Enabled": true
	},
	{
		// 进程间通信
		"Name": "IPC",
		"Enabled": true
	},
	{
		// 平台覆盖层（好友 / 邀请界面）
		"Name": "Overlay",
		"Enabled": true
	},

	// ── 9 ── 文件与序列化（18）──
	{
		// 序列化核心（FArchive）
		"Name": "Serialization",
		"Enabled": true
	},
	{
		// JSON 解析与写入
		"Name": "Json",
		"Enabled": true
	},
	{
		// JSON 工具函数（UStruct 与 JSON 互转）
		"Name": "JsonUtilities",
		"Enabled": true
	},
	{
		// CBOR 二进制 JSON 格式
		"Name": "Cbor",
		"Enabled": true
	},
	{
		// XML 解析
		"Name": "XmlParser",
		"Enabled": true
	},
	{
		// Pak 包文件读写
		"Name": "PakFile",
		"Enabled": true
	},
	{
		// 沙盒文件系统
		"Name": "SandboxFile",
		"Enabled": true
	},
	{
		// 流式文件读取
		"Name": "StreamingFile",
		"Enabled": true
	},
	{
		// 虚拟文件缓存
		"Name": "VirtualFileCache",
		"Enabled": true
	},
	{
		// 网络文件访问
		"Name": "NetworkFile",
		"Enabled": true
	},
	{
		// 网络文件系统实现
		"Name": "NetworkFileSystem",
		"Enabled": true
	},
	{
		// Oodle 压缩库
		"Name": "OodleDataCompression",
		"Enabled": true
	},
	{
		// 存储服务器客户端（分布式资产）
		"Name": "StorageServerClient",
		"Enabled": true
	},
	{
		// 运行时资产缓存
		"Name": "RuntimeAssetCache",
		"Enabled": true
	},
	{
		// 安装分包管理
		"Name": "InstallBundleManager",
		"Enabled": true
	},
	{
		// 空安装分包实现
		"Name": "NullInstallBundleManager",
		"Enabled": true
	},
	{
		// 边玩边 Cook（开发期）
		"Name": "CookOnTheFly",
		"Enabled": true
	},
	{
		// 流式暂停渲染
		"Name": "StreamingPauseRendering",
		"Enabled": true
	},

	// ── 10 ── 平台与设备（16）──
	{
		// Android 平台实现
		"Name": "Android",
		"Enabled": true
	},
	{
		// Apple 平台公共代码
		"Name": "Apple",
		"Enabled": true
	},
	{
		// iOS 平台实现
		"Name": "IOS",
		"Enabled": true
	},
	{
		// Linux 平台实现
		"Name": "Linux",
		"Enabled": true
	},
	{
		// Unix 平台公共代码
		"Name": "Unix",
		"Enabled": true
	},
	{
		// Windows 平台实现
		"Name": "Windows",
		"Enabled": true
	},
	{
		// 平台第三方库辅助
		"Name": "PlatformThirdPartyHelpers",
		"Enabled": true
	},
	{
		// Chromium Embedded Framework 工具
		"Name": "CEF3Utils",
		"Enabled": true
	},
	{
		// 网页浏览器控件
		"Name": "WebBrowser",
		"Enabled": true
	},
	{
		// 网页内容作为贴图
		"Name": "WebBrowserTexture",
		"Enabled": true
	},
	{
		// 平台门户集成
		"Name": "Portal",
		"Enabled": true
	},
	{
		// 广告 SDK 抽象
		"Name": "Advertising",
		"Enabled": true
	},
	{
		// 眼动追踪设备支持
		"Name": "EyeTracker",
		"Enabled": true
	},
	{
		// VR 头显基础接口
		"Name": "HeadMountedDisplay",
		"Enabled": true
	},
	{
		// AR 基础接口
		"Name": "AugmentedReality",
		"Enabled": true
	},
	{
		// RSA 加密工具
		"Name": "RSA",
		"Enabled": true
	},

	// ── 11 ── 工具与基础设施（27）──
	{
		// 资产注册表（不加载资产就能查元数据）
		"Name": "AssetRegistry",
		"Enabled": true
	},
	{
		// 开发者设置基类
		"Name": "DeveloperSettings",
		"Enabled": true
	},
	{
		// 构建设置
		"Name": "BuildSettings",
		"Enabled": true
	},
	{
		// 项目描述与配置读取
		"Name": "Projects",
		"Enabled": true
	},
	{
		// 属性路径解析
		"Name": "PropertyPath",
		"Enabled": true
	},
	{
		// 类型化元素框架
		"Name": "TypedElementFramework",
		"Enabled": true
	},
	{
		// 类型化元素运行时
		"Name": "TypedElementRuntime",
		"Enabled": true
	},
	{
		// 通用对象定位
		"Name": "UniversalObjectLocator",
		"Enabled": true
	},
	{
		// 交互工具框架（编辑器工具用）
		"Name": "InteractiveToolsFramework",
		"Enabled": true
	},
	{
		// 实验性功能容器
		"Name": "Experimental",
		"Enabled": true
	},
	{
		// 工具箱
		"Name": "Toolbox",
		"Enabled": true
	},
	{
		// 统计上报
		"Name": "Analytics",
		"Enabled": true
	},
	{
		// 崩溃报告核心
		"Name": "CrashReportCore",
		"Enabled": true
	},
	{
		// 硬件调查
		"Name": "HardwareSurvey",
		"Enabled": true
	},
	{
		// Trace 日志系统
		"Name": "TraceLog",
		"Enabled": true
	},
	{
		// 自动化测试框架
		"Name": "AutomationTest",
		"Enabled": true
	},
	{
		// 预加载屏幕
		"Name": "PreLoadScreen",
		"Enabled": true
	},
	{
		// 性能计数器
		"Name": "PerfCounters",
		"Enabled": true
	},
	{
		// 蓝图运行时
		"Name": "BlueprintRuntime",
		"Enabled": true
	},
	{
		// 通用游戏框架基础（GameMode 等基类）
		"Name": "UnrealGame",
		"Enabled": true
	},
	{
		// UE 通用蓝图函数库
		"Name": "UELibrary",
		"Enabled": true
	},
	{
		// Niagara 的向量虚拟机
		"Name": "VectorVM",
		"Enabled": true
	},
	{
		// Verse 语言虚拟机
		"Name": "CoreVerseVM",
		"Enabled": true
	},
	{
		// 引擎消息定义
		"Name": "EngineMessages",
		"Enabled": true
	},
	{
		// 引擎设置
		"Name": "EngineSettings",
		"Enabled": true
	},
	{
		// 符号解析库
		"Name": "SymsLib",
		"Enabled": true
	},
	{
		// JPEG 组件
		"Name": "UEJpegComp",
		"Enabled": true
	},

	// ── 12 ── 数据与导入（2）──
	{
		// Datasmith 数据导入框架
		"Name": "Datasmith",
		"Enabled": true
	},
	{
		// 新一代资产导入框架
		"Name": "Interchange",
		"Enabled": true
	}
]
```

---

## 看这份清单时注意

| 项 | 说明 |
|---|---|
| 编号 | 1~177 连续，按第 4.1 节 12 类的顺序排 |
| 注释 | 独占一行、写在 `"Name"` 正上方，和 `.uproject` 复制文件写法完全一致 |
| `Enabled` | 只是格式占位，**不是真实开关**；引擎模块没有 ini 开关这一说 |
| 用途 | 快速知道「某个模块是干嘛的」，不是拿去改配置 |

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime`（177 个模块目录实查，清单逐项对应）
- `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra复刻\S00工程骨架\01_Tenkaichi.uproject复制文件.md`（格式参照）
- `..\00_UE5现有功能_总md.md` 第 4.1 节（12 类分组依据）
