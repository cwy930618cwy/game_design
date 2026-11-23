# `Config\DefaultEngine.ini` — 逐行讲解（你自己这份 84 行）

> 对象：`E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini`（**84 行**）
> 原文件**一个字都没动**，这份 md 只是把每行搬到下面、在**上方**加一行 `;` 中文注释。
> 看的时候：**代码行 = 你文件里的原样**；`;` 开头的行 = 我加的讲解，你的文件里没有。

---

## 全文逐行注释

```ini
; ─────────────────────────────────────────────
; L1-L2：两个空行。引擎保存 ini 时会自动留，没用，不用管
; ─────────────────────────────────────────────


; ─────────────────────────────────────────────
; L3：段头。UGameMapsSettings，管「工程默认地图 / 默认 GameMode / GameInstance 类」
;     段名格式 = /Script/模块名.类名；这里 = 引擎 Engine 模块的 UGameMapsSettings
; ─────────────────────────────────────────────
[/Script/EngineSettings.GameMapsSettings]
; L4：双击工程时先加载哪张地图
;     /Engine/Maps/Templates/OpenWorld = 引擎自带的大世界模板图（不是我们工程里的图）
;     这是建工程选「OpenWorld 模板」的默认值，暂时能跑，后期要换成自己的地图
GameDefaultMap=/Engine/Maps/Templates/OpenWorld

; ─────────────────────────────────────────────
; L6：段头。UWindowsTargetSettings，管「Windows 平台怎么打包」
;     注意这个段很杂：图形 API + 音频参数都在里面，因为都是"Windows 平台设置"
; ─────────────────────────────────────────────
[/Script/WindowsTargetPlatform.WindowsTargetSettings]
; L7：默认图形 API = DX12（可选项还有 DX11 / Vulkan）
DefaultGraphicsRHI=DefaultGraphicsRHI_DX12
; L8：DX12 编译的着色器模型 = SM6（Shader Model 6，DX12 必须配 SM6）
D3D12TargetedShaderFormats=PCD3D_SM6
; L9：DX11 编译的着色器模型 = SM5（DX11 最高只到 SM5）
D3D11TargetedShaderFormats=PCD3D_SM5
; L10：着色器编译器 = Default（用引擎默认，即 DXC/FXC）
Compiler=Default

; ── 以下是音频参数（L11-L31），现在全是模板默认值，一个都没调过 ──
; L11：音频采样率 = 48000 Hz（48kHz，游戏标准）
AudioSampleRate=48000
; L12：音频回调缓冲帧数 = 1024（越大越省 CPU、延迟越高）
AudioCallbackBufferFrameSize=1024
; L13：音频缓冲队列数 = 1（越小延迟越低，但容易爆音）
AudioNumBuffersToEnqueue=1
; L14：最大声道数 = 0（0 = 不限制，交给硬件）
AudioMaxChannels=0
; L15：音频源解码线程数 = 4（同时并行解码 4 个音源）
AudioNumSourceWorkers=4
; L16：空间化插件 = 空（没启用任何空间音频插件）
SpatializationPlugin=
; L17：声源数据覆盖插件 = 空
SourceDataOverridePlugin=
; L18：混响插件 = 空（所以现在混响是引擎原生的）
ReverbPlugin=
; L19：遮挡插件 = 空
OcclusionPlugin=
; L20：压缩覆盖设置——不改压缩时长(False) / 时长阈值 5 秒 / 最大随机分支 0 / SoundCue 质量档 0
CompressionOverrides=(bOverrideCompressionTimes=False,DurationThreshold=5.000000,MaxNumRandomBranches=0,SoundCueQualityIndex=0)
; L21：音频缓存 = 65536 KB（64 MB）
CacheSizeKB=65536
; L22：单块最大覆盖大小 = 0（0 = 不限制）
MaxChunkSizeOverrideKB=0
; L23：是否按设备重采样 = False（用上面的采样率，不重采样）
bResampleForDevice=False
; L24：最高采样率档 = 48000
MaxSampleRate=48000.000000
; L25：高采样率档 = 32000
HighSampleRate=32000.000000
; L26：中采样率档 = 24000
MedSampleRate=24000.000000
; L27：低采样率档 = 12000
LowSampleRate=12000.000000
; L28：最低采样率档 = 8000
MinSampleRate=8000.000000
; L29：压缩质量系数 = 1.0（1.0 = 标准；越大质量越高、体积越大）
CompressionQualityModifier=1.000000
; L30：自动流式加载阈值 = 0（0 = 不启用）
AutoStreamingThreshold=0.000000
; L31：SoundCue 打包质量档 = -1（-1 = 不覆盖，用默认）
SoundCueCookQualityIndex=-1

; ─────────────────────────────────────────────
; L33：段头。URendererSettings = 「项目设置 → 引擎 → 渲染」整个面板
;      这个段最要紧，Lumen / 阴影 / 距离场全在这里
; ─────────────────────────────────────────────
[/Script/Engine.RendererSettings]
; L34：移动端——启用「无预计算光照时的级联阴影着色器」= True（只影响手机，PC 无关）
r.Mobile.EnableNoPrecomputedLightingCSMShader=True
; L35：空行（引擎保存时自动插的，无作用）

; L36：生成网格距离场 = True（Lumen 软件光追 / 距离场阴影 / Mesh DF 全依赖它）
r.GenerateMeshDistanceFields=True
; L37：空行

; L38：动态全局光照方法 = 1 → Lumen（0=关，1=Lumen，2=SSGI）
r.DynamicGlobalIlluminationMethod=1
; L39：空行

; L40：反射方法 = 1 → Lumen（0=关，1=Lumen，2=SSR 屏幕空间反射）
r.ReflectionMethod=1
; L41：空行

; L42：虚拟阴影贴图 = 1 → 开启（Lumen 配套阴影，替代传统静态阴影）
r.Shadow.Virtual.Enable=1
; L43：空行

; L44：自动曝光——扩展默认亮度范围 = True（HDR 曝光，避免暗部死黑）
r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True
; L45：空行

; L46：局部曝光——高光对比度系数 = 0.8
r.DefaultFeature.LocalExposure.HighlightContrastScale=0.8
; L47：空行

; L48：局部曝光——阴影对比度系数 = 0.8
r.DefaultFeature.LocalExposure.ShadowContrastScale=0.8

; ⚠️ 这里少了一行（下面问题清单第 2 条）：
;     r.AllowStaticLighting=False    ← 关了它，「光照需要重建」提示才不出现

; ─────────────────────────────────────────────
; L50：段头。ULinuxTargetSettings，管 Linux 打包
; ─────────────────────────────────────────────
[/Script/LinuxTargetPlatform.LinuxTargetSettings]
; L51：Linux 用 Vulkan + SM6（不打包 Linux 就完全用不到这行）
TargetedRHIs=SF_VULKAN_SM6

; ─────────────────────────────────────────────
; L53：段头。UHardwareTargetingSettings = 「项目设置 → 目标硬件」
; ─────────────────────────────────────────────
[/Script/HardwareTargeting.HardwareTargetingSettings]
; L54：目标硬件类别 = Desktop（桌面；另有 Mobile / 不限）
TargetedHardwareClass=Desktop
; L55：已应用的硬件类别 = Desktop（改了 L54 后点「应用」按钮，这里才会同步）
AppliedTargetedHardwareClass=Desktop
; L56：默认图形性能 = Maximum（最高档，会带一批画质 CVar 的初始值）
DefaultGraphicsPerformance=Maximum
; L57：已应用的图形性能 = Maximum（同上，点「应用」才写）
AppliedDefaultGraphicsPerformance=Maximum

; ─────────────────────────────────────────────
; L59：段头。UWorldPartitionEditorSettings，管大世界（World Partition）关卡编辑器行为
; ─────────────────────────────────────────────
[/Script/WorldPartitionEditor.WorldPartitionEditorSettings]
; L60：关卡转换命令 = UWorldPartitionConvertCommandlet
;      Commandlet = 无界面命令行批处理；这个是「把传统关卡转成大世界分区关卡」用
CommandletClass=Class'/Script/UnrealEd.WorldPartitionConvertCommandlet'

; ─────────────────────────────────────────────
; L62：段头。UUserInterfaceSettings，管全局 UI 缩放 / DPI
; ─────────────────────────────────────────────
[/Script/Engine.UserInterfaceSettings]
; L63：禁止自动创建控件变量 = False（False = 允许蓝图勾「是变量」自动生成变量）
bAuthorizeAutomaticWidgetVariableCreation=False
; L64：字体 DPI 预设 = Standard（可选 Small / Standard / Large，影响编辑器字体大小）
FontDPIPreset=Standard
; L65：字体 DPI 数值 = 72（数值越大字越大）
FontDPI=72

; ─────────────────────────────────────────────
; L67：段头。UEngine = 「引擎用哪个类去实现哪个角色」，全工程最重要的替换点
; ─────────────────────────────────────────────
[/Script/Engine.Engine]
; L68：⚠️ 模板残留！把旧模块 TP_Blank 重定向到 /Script/Tenkaichi
;      TP_Blank = 建工程时选的空白模板的模块名
;      /Script/Tenkaichi = 旧模块名，这个模块已经不存在了（现在叫 LyraGame）
;      作用：老资产若引用 /Script/TP_Blank.Xxx，自动改成指向 /Script/Tenkaichi.Xxx
+ActiveGameNameRedirects=(OldGameName="TP_Blank",NewGameName="/Script/Tenkaichi")
; L69：⚠️ 同上，只是不带 /Script/ 前缀的写法也要写一遍（引擎两种写法都认）
+ActiveGameNameRedirects=(OldGameName="/Script/TP_Blank",NewGameName="/Script/Tenkaichi")

; ─────────────────────────────────────────────
; L71：段头。UAndroidFileServerRuntimeSettings，管「安卓真机文件推送」（调试用）
; ─────────────────────────────────────────────
[/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings]
; L72：启用插件 = True
bEnablePlugin=True
; L73：允许网络连接 = True
bAllowNetworkConnection=True
; L74：安全令牌 = 自动生成的 GUID（每个工程不同，正常现象）
SecurityToken=BAC44A2E46D09B79EEEF3882E3E5B425
; L75：打进发行包 = False（发行版不带这个调试插件）
bIncludeInShipping=False
; L76：允许发行版被外部启动 = False
bAllowExternalStartInShipping=False
; L77：编译 AFS 工程 = False
bCompileAFSProject=False
; L78：使用压缩 = False
bUseCompression=False
; L79：记录文件 = False
bLogFiles=False
; L80：上报统计 = False
bReportStats=False
; L81：连接方式 = USBOnly（只走 USB，另有 WiFi 选项）
ConnectionType=USBOnly
; L82：手动填 IP = False
bUseManualIPAddress=False
; L83：手动 IP 地址 = 空
ManualIPAddress=
; L84：文件最后一个空行
```

---

## 问题清单（就 3 条）

| # | 行号 | 问题 | 怎么改 |
|---|---|---|---|
| 1 | **L68-L69** | `NewGameName="/Script/Tenkaichi"` —— 这个模块已经不存在了（`.uproject` 里只有 `LyraGame`，`Source\` 里也只有 `LyraGame` / `LyraEditor`）。属于建工程时的模板残留，Lyra 原文也没有这两行 | **整段删掉这 2 行** |
| 2 | **L48 之后** | 缺 `r.AllowStaticLighting=False`。不写的话默认值是 `True` → 只要场景里有一个 Static/Stationary 光源，编辑器就弹「光照需要重建」 | **加 1 行**：`r.AllowStaticLighting=False` |
| 3 | **文件末尾** | 缺 `[/Script/Engine.CollisionProfile]` 段。你现在把它写在 `DefaultEditor.ini` 里 —— **那个文件引擎根本不读碰撞配置** | 把 `DefaultEditor.ini` 里那 2 行**搬到这里**（末尾） |

### 第 3 条的证据（为什么必须搬）

```
Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274
    GConfig->GetSection(TEXT("/Script/Engine.CollisionProfile"), false, GEngineIni);
                                                                       ↑ 写死 GEngineIni
```

`GEngineIni` = `DefaultEngine.ini`。`UCollisionProfile` 类头上也写着 `UCLASS(config=Engine, defaultconfig)`（`CollisionProfile.h:159`）。
所以碰撞配置**只认 `DefaultEngine.ini`**，写进 `DefaultEditor.ini` → 静默失效 → 水体报错照旧。

---

## 一句话

**9 个段、84 行。只有 3 处要动：删 L68-69（Tenkaichi 重定向）、加 `r.AllowStaticLighting=False`、把碰撞段从 `DefaultEditor.ini` 搬过来。其余 81 行保持原样。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini:1-84`（你这份全文，逐行对应上面注释）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:16-26`（Lyra 的 `[/Script/Engine.Engine]`，无 `ActiveGameNameRedirects`）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:88`（`r.AllowStaticLighting=False` 原文位置）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274`（`GEngineIni` 硬绑定）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\CollisionProfile.h:159`（`UCLASS(config=Engine, defaultconfig)`）