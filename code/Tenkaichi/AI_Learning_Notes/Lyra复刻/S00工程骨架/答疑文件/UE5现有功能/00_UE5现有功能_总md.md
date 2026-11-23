# 00 — UE5 引擎全景：里面到底有什么（总 md）

> 依据：`D:\ue5\Epic Games\UE_5.4` 真实目录扫描（2026-11-23）
> 规模：Engine 子树 **197,578** 文件；`Source` 5 大类 / `Runtime` **177** 模块 / `Editor` **134** 模块；`Plugins` **61** 个分类目录；`Content` **51** 个目录。
> 定位：**总 md** —— 先看这张地图，再决定钻哪一块。本文件是答疑类，**不进教学台账**。
> 写法参照：`S00工程骨架\01_Tenkaichi.uproject复制文件.md`（先讲语义，再分类列清单）。

---

## 一、先记住 7 条「目录语义」（不然看再多也是黑盒）

| 路径 | 是什么 | 你碰不碰 |
|---|---|---|
| `Engine\Source` | 引擎**全部 C++ 源码**（最值钱的部分） | ✅ 只读查基类定义 |
| `Engine\Plugins` | 可插拔功能包，每个有 `.uplugin` | ✅ 只读；和工程插件对照看 |
| `Engine\Content` | 引擎自带资源（材质/字体/图标/地图模板） | 👀 知道有就行 |
| `Engine\Config` | **默认 ini**（BaseEngine.ini 等） | ✅ 复刻 ini 时的源头 |
| `Engine\Binaries` | 编译好的 dll / exe | ❌ 永不碰（机器生成的） |
| `Engine\Intermediate` / `Saved` / `DerivedDataCache` | 编译中间产物 / 缓存 | ❌ 永不碰（删了引擎自己会重建） |
| `Engine\Shaders` / `Documentation` / `Programs` | 着色器源码 / 官方文档 / 独立工具 | 👀 需要时再查 |

> 🔑 **一句话**：引擎不是黑盒，它是一套「**源码 + 插件 + 默认配置 + 一堆生成物**」。
> 真正要读的只有前 4 行，剩下 9 成文件是编译产物和资源，不用管。

---

## 二、全景树（从大到小）

```
D:\ue5\Epic Games\UE_5.4\                    ← 引擎安装根
├─ Engine\                                   ← 引擎本体（197,578 文件）
│  ├─ Source\                                ← ★ C++ 源码（59,466 文件）
│  │  ├─ Runtime\      177 模块              ← 游戏运行时代码（最常查）
│  │  ├─ Editor\       134 模块              ← 编辑器代码
│  │  ├─ Developer\                          ← 开发工具/命令行工具
│  │  ├─ Programs\                           ← 独立程序（UnrealBuildTool 等）
│  │  └─ ThirdParty\                         ← 第三方库（物理/压缩/图像）
│  ├─ Plugins\                               ← ★ 61 个分类目录（75,648 文件）
│  ├─ Content\                               ← 引擎自带资源（28,209 文件）
│  ├─ Config\                                ← ★ 默认 ini（78 文件）
│  ├─ Shaders\                               ← 着色器源码（1,010 文件）
│  ├─ Binaries\ / Intermediate\ / Saved\     ← 生成物，别碰
│  ├─ Documentation\ / Extras\ / Platforms\  ← 文档 / 示例 / 平台适配
│  └─ Build\ / Programs\ / DerivedDataCache\
├─ Templates\                                ← 新建项目时的模板（2,696 文件）
├─ Samples\                                  ← 示例项目（267 文件）
├─ FeaturePacks\                             ← 功能包（10 个 .upack）
└─ ToolComplete.txt
```

---

## 三、Engine 根目录 14 个子目录逐个讲

| 目录 | 干什么 | 复刻时 |
|---|---|---|
| `Source` | 引擎 C++ 源码，分 Runtime / Editor / Developer / Programs / ThirdParty | ✅ **只读查基类** |
| `Plugins` | 引擎插件，按功能分 61 个类目 | ✅ **只读查插件** |
| `Content` | 引擎自带 uasset / 贴图 / 字体 / 地图 | 👀 知道即可 |
| `Config` | `BaseEngine.ini` / `BaseGame.ini` 等默认配置 | ✅ **ini 源头** |
| `Shaders` | `.usf` / `.ush` 着色器源码 | 👀 图形方向再看 |
| `Binaries` | 已编译 dll / exe / pdb | ❌ 别碰 |
| `Intermediate` | UHT 生成代码、obj、lib | ❌ 别碰 |
| `Saved` | 日志、配置缓存、自动保存 | ❌ 别碰 |
| `DerivedDataCache` | 资源派生数据缓存 | ❌ 别碰 |
| `Documentation` | `.udn` 官方文档（673 篇） | 👀 查 API 可用 |
| `Build` | 构建脚本 / 平台工具链配置 | 👀 编译报错再看 |
| `Extras` | 第三方示例代码、工具 | 👀 一般不用 |
| `Platforms` | 特定平台适配代码 | 👀 跨平台再看 |
| `Programs` | 独立工具工程（4 个 .uproject） | 👀 一般不用 |

---

## 四、`Engine\Source` —— 引擎源码的 5 大类

| 类目 | 作用 | 复刻相关性 |
|---|---|---|
| `Runtime` | **游戏运行时**全部模块（177 个）：Core / Engine / Renderer / UMG… | ★★★ 最常查 |
| `Editor` | **编辑器**模块（134 个）：细节面板、蓝图编辑器、内容浏览器… | ★★ 偶尔查 UPROPERTY 的编辑器行为 |
| `Developer` | 开发工具与命令行工具：AssetRegistry 工具、Profiler… | ★ |
| `Programs` | 独立程序：UnrealBuildTool、AutomationTool 等 | ★ 编译链报错时查 |
| `ThirdParty` | 第三方库：PhysX/Chaos 依赖、 zlib、libcurl、OpenSSL… | ❌ 基本不碰 |

### 4.1 `Runtime` 的 177 个模块（按功能分 12 类）

**① 核心地基（5）**
`Core` `CoreUObject` `Engine` `ApplicationCore` `Launch`

**② 渲染与图像（17）**
`Renderer` `RenderCore` `RHI` `RHICore` `D3D12RHI` `VulkanRHI` `OpenGLDrv` `NullDrv` `ImageCore` `ImageWrapper` `ImageWriteQueue` `ColorManagement` `OpenColorIOWrapper` `TextureUtilitiesCommon` `MaterialShaderQualitySettings` `CUDA` `NNE`

**③ UI、窗口与输入（10）**
`Slate` `SlateCore` `SlateRHIRenderer` `SlateNullRenderer` `UMG` `AdvancedWidgets` `AppFramework` `WidgetCarousel` `InputCore` `InputDevice`

**④ 音频与媒体（27）**
`AudioExtensions` `AudioMixer` `AudioMixerCore` `AudioAnalyzer` `AudioCaptureCore` `AudioCaptureImplementations` `AudioLink` `AudioPlatformConfiguration` `SignalProcessing` `SoundFieldRendering` `NonRealtimeAudioRenderer` `SynthBenchmark` `AdpcmAudioDecoder` `BinkAudioDecoder` `OpusAudioDecoder` `RadAudioCodec` `UEWavComp` `VorbisAudioDecoder` `AVEncoder` `AVIWriter` `Media` `MediaAssets` `MediaUtils` `MoviePlayer` `MoviePlayerProxy` `GameplayMediaEncoder` `IESFile`

**⑤ 动画与布料（9）**
`AnimationCore` `AnimGraphRuntime` `ClothingSystemRuntimeCommon` `ClothingSystemRuntimeInterface` `ClothingSystemRuntimeNv` `LiveLinkAnimationCore` `LiveLinkInterface` `LiveLinkMessageBusFramework` `TimeManagement`

**⑥ 物理与几何（12）**
`PhysicsCore` `GeometryCore` `GeometryFramework` `FieldNotification` `MeshDescription` `SkeletalMeshDescription` `StaticMeshDescription` `RawMesh` `MeshConversion` `MeshConversionEngineTypes` `MeshUtilitiesCommon` `MRMesh`

**⑦ 玩法与场景（15）**
`GameplayTags` `GameplayTasks` `GameplayDebugger` `AIModule` `NavigationSystem` `Navmesh` `Foliage` `Landscape` `LevelSequence` `MovieScene` `MovieSceneTracks` `MovieSceneCapture` `CinematicCamera` `VirtualProduction` `GameMenuBuilder`

**⑧ 网络与在线（19）**
`Net` `Networking` `Sockets` `Messaging` `MessagingCommon` `MessagingRpc` `SessionMessages` `SessionServices` `Online` `CoreOnline` `FriendsAndChat` `PacketHandlers` `NetworkReplayStreaming` `AutomationMessages` `AutomationWorker` `ClientPilot` `ExternalRPCRegistry` `IPC` `Overlay`

**⑨ 文件与序列化（18）**
`Serialization` `Json` `JsonUtilities` `Cbor` `XmlParser` `PakFile` `SandboxFile` `StreamingFile` `VirtualFileCache` `NetworkFile` `NetworkFileSystem` `OodleDataCompression` `StorageServerClient` `RuntimeAssetCache` `InstallBundleManager` `NullInstallBundleManager` `CookOnTheFly` `StreamingPauseRendering`

**⑩ 平台与设备（16）**
`Android` `Apple` `IOS` `Linux` `Unix` `Windows` `PlatformThirdPartyHelpers` `CEF3Utils` `WebBrowser` `WebBrowserTexture` `Portal` `Advertising` `EyeTracker` `HeadMountedDisplay` `AugmentedReality` `RSA`

**⑪ 工具与基础设施（27）**
`AssetRegistry` `DeveloperSettings` `BuildSettings` `Projects` `PropertyPath` `TypedElementFramework` `TypedElementRuntime` `UniversalObjectLocator` `InteractiveToolsFramework` `Experimental` `Toolbox` `Analytics` `CrashReportCore` `HardwareSurvey` `TraceLog` `AutomationTest` `PreLoadScreen` `PerfCounters` `BlueprintRuntime` `UnrealGame` `UELibrary` `VectorVM` `CoreVerseVM` `EngineMessages` `EngineSettings` `SymsLib` `UEJpegComp`

**⑫ 数据与导入（2）**
`Datasmith` `Interchange`

> 复刻时真正会反复查的只有 **①③④⑦⑨** 这几类：CoreUObject（UCLASS/反射）、UMG（Widget 基类）、GameplayTags、Serialization。`Renderer`、`D3D12RHI` 这类知道存在即可。

---

## 五、`Engine\Plugins` —— 61 个分类目录

> 你已经有一份 70 项插件的逐项详解：`答疑文件\插件详解\00_插件详解_索引.md`，那是**`.uproject` 里启用的插件**。这里是**引擎自带插件按文件夹分类**，两个视角不同，别混。

| 分类 | 代表插件 | 分类 | 代表插件 |
|---|---|---|---|
| `Runtime` | 运行时插件大头 | `Editor` | 编辑器扩展 |
| `2D` | Paper2D | `AI` | 行为树 / StateTree |
| `Animation` | 动画 Warping / Locomotion | `AudioGameplay` | 音频玩法 |
| `AudioGameplayVolume` | 音频区域 | `AudioInsights` | 音频性能分析 |
| `BlueprintFileUtils` | 蓝图文件工具 | `Bridge` | LiveLink 桥接 |
| `Cameras` | 相机 | `ChaosCloth` | 布料模拟 |
| `ChaosClothAsset` | 布料资产 | `ChaosVD` |  Chaos 可视化调试 |
| `CmdLinkServer` | 命令行链接服务 | `Compositing` | 合成 |
| `Compression` | 压缩 | `Developer` | 开发工具 |
| `DiscoveryBeaconReceiver` | 局域网发现 | `EnhancedInput` | ★ 增强输入 |
| `Enterprise` | 企业版功能 | `Experimental` | 实验性功能 |
| `Fab` | Fab 资产市场 | `FastBuildController` | 快速构建 |
| `FX` | Niagara 特效 | `Importers` | 导入器 |
| `Interchange` | 新导入框架 | `JsonBlueprintUtilities` | JSON 蓝图工具 |
| `LightWeightInstancesEditor` | HISM 编辑 | `Marketplace` | 市场 |
| `Media` | 媒体播放 | `MegascansPlugin` | Quixel 扫描资产 |
| `MemoryUsageQueries` | 内存查询 | `MeshPainting` | 网格绘制 |
| `Messaging` | 消息总线 | `MovieScene` | 过场动画 |
| `NetcodeUnitTest` | 网络单测 | `NNE` | 神经网络引擎 |
| `Online` | 在线服务 | `PCG` | ★ 程序化内容生成 |
| `PCGExternalDataInterop` | PCG 外部数据 | `PCGGeometryScriptInterop` | PCG 几何脚本 |
| `Performance` | 性能 | `Portal` | 门户集成 |
| `Protocols` | 协议 | `RenderGraphInsights` | 渲染图分析 |
| `ScriptGeneratorPlugin` | 脚本生成 | `ScriptPlugin` | 脚本宿主 |
| `Slate` | UI 框架 | `Tests` | 测试 |
| `TraceUtilities` | Trace 工具 | `UbaController` | 分布式构建 |
| `VirtualProduction` | 虚拟制片 | `Web` | Web 集成 |
| `WorldMetrics` | 世界指标 | `XGEController` | 分布式编译 |

---

## 六、`Engine\Content` —— 51 个资源目录（知道即可）

按用途归四类：

- **给编辑器用**：`Editor*` 系列（Editor / EditorResources / EditorMaterials / EditorMeshes / EditorSounds / EditorShapes / EditorShellMaterials / EditorSky…）、`Slate` / `SlateDebug` / `SlateFileDialogs`
- **给引擎渲染用**：`EngineMaterials` / `EngineMeshes` / `EngineFonts` / `EngineSky` / `EngineVolumetrics` / `Renderer` / `BufferVisualization` / `Engine_MI_Shaders`
- **给项目起步用**：`StarterContent` / `BasicShapes` / `Maps` / `MapTemplates` / `MaterialTemplates` / `TemplateResources` / `Tutorial` / `VREditor`
- **给流程用**：`Automation` / `FunctionalTesting` / `Localization` / `Internationalization` / `Sequencer` / `WorldPartition` / `Animation` / `ArtTools`

---

## 七、跟本复刻线的关系（哪些读、哪些永不碰）

| 动作 | 对象 | 说明 |
|---|---|---|
| ✅ **只读** | `Engine\Source\Runtime` | 查基类定义：如 `UCommonActivatableWidget` 在 `Engine\Plugins\Runtime\CommonUI\...` |
| ✅ **只读** | `Engine\Plugins` | 查引擎插件源码与 `.uplugin` |
| ✅ **只读** | `Engine\Config` | `Base*.ini` 是 Lyra ini 的源头 |
| ✅ **只读** | `E:\ue5\LyraStarterGame` | 参考源（一比一照抄的对象） |
| ❌ **不写** | 整个 `D:\ue5\Epic Games\UE_5.4` | 引擎是只读权威源，一个字都不改 |
| ❌ **不碰** | `Binaries` / `Intermediate` / `Saved` / `DerivedDataCache` | 生成物，与源码无关 |

> 铁律回顾：**只换根不改名，代码零差异，注释全中文，AI 只教不写。**

---

## 八、下一步可以钻的方向

| 想搞懂什么 | 下一份 md 该写什么 |
|---|---|
| 「Runtime 那 177 个模块各自干嘛」 | 按第 4.1 节 12 类，一类一份详解 |
| 「插件怎么分类、哪些是 Lyra 启用的」 | 看 `插件详解\00_插件详解_索引.md`（已有 70 项） |
| 「ini 配置 chain 是怎么来的」 | 看 `Config四个ini文件讲解.md`（已有） |
| 「某个具体基类（如 UCommonActivatableWidget）」 | 用户点名后做两件套，一轮一个 |

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4`（根目录、`Engine\` 第一层实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source`（5 类；`Runtime` 177 模块、`Editor` 134 模块，逐目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins`（61 个分类目录实查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Content`（51 个目录实查）
- `E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra复刻\S00工程骨架\01_Tenkaichi.uproject复制文件.md`（写法参照）
