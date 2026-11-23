# 答疑 — `Config\` 四个 ini 文件干什么的（逐个讲解）

> 触发：`Config\` 里改完后要弄明白这 4 个文件分别管什么、为什么有些段必须写在指定文件里。
> 对照：`E:\ue5\LyraStarterGame\Config`（Lyra 原文 4 个文件，实查）
> 依据：`D:\ue5\Epic Games\UE_5.4`（引擎 UCLASS(config=…) 声明实查）

---

## 一、四个文件总览

| 文件 | 我们（行/字节） | Lyra（行/字节） | 管什么 | 谁在读 |
|---|---|---|---|---|
| `DefaultEngine.ini` | 84 / 2.4KB | 369 / 31.2KB | 引擎级设置：默认地图、渲染、碰撞、物理、音频、RHI、类的全局替换 | 引擎启动（`GEngineIni`） |
| `DefaultGame.ini` | 13 / 581B | 226 / 12.2KB | 项目级设置：工程名、AssetManager 资产扫描、GAS 全局、打包 | 引擎启动（`GGameIni`） |
| `DefaultEditor.ini` | 2 / 522B | 85 / 28.6KB | **只在编辑器里生效**：蓝图默认类、UMG/CommonUI 编辑器偏好、本地化 | 编辑器启动（`GEditorIni`） |
| `DefaultInput.ini` | 85 / 9.0KB | 51 / 3.6KB | 输入：轴配置、鼠标捕获、默认 PlayerInput/InputComponent 类、调试快捷键 | 引擎启动（`GInputIni`） |

🔑 **一句话**：四个 ini 是**四本不同的登记簿**，引擎按「类声明的 config 名」去找对应的那本，**写错本子 = 白写**。

---

## 二、为什么「写错文件」会静默失效（本节决定一切）

每个可配置的 UCLASS 头上都写着 `config=XXX`：

| 类 | 声明原文 | 位置 | 实际读取的文件 |
|---|---|---|---|
| `UCollisionProfile` | `UCLASS(config=Engine, defaultconfig)` | `Engine\Classes\Engine\CollisionProfile.h:159` | **`DefaultEngine.ini`** |
| `UGameMapsSettings` | `UCLASS(config=Engine, defaultconfig)` | `EngineSettings\Classes\GameMapsSettings.h:94` | **`DefaultEngine.ini`** |
| `URendererSettings` | `UCLASS(config=Engine, defaultconfig)` | `Engine\Classes\Engine\RendererSettings.h:297` | **`DefaultEngine.ini`** |
| `UAssetManagerSettings` | `UCLASS(config=Game, defaultconfig)` | `Engine\Classes\Engine\AssetManagerSettings.h:66` | **`DefaultGame.ini`** |
| `UInputSettings` | `UCLASS(config=Input, defaultconfig)` | `Engine\Classes\GameFramework\InputSettings.h:21` | **`DefaultInput.ini`** |
| `UUnrealEdOptions` | `UCLASS(Config=Editor)` | `Editor\UnrealEd\Classes\Preferences\UnrealEdOptions.h:84` | **`DefaultEditor.ini`** |

碰撞配置的读取代码是写死的：

```
Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274
const FConfigSection* Configs = GConfig->GetSection(TEXT("/Script/Engine.CollisionProfile"), false, GEngineIni);
                                                                                              ↑ 写死 GEngineIni
```

`GEngineIni` = `DefaultEngine.ini`。**所以 `[/Script/Engine.CollisionProfile]` 写进 `DefaultEditor.ini` 完全不生效**，不报错、不提示，就是读不到。

---

## 三、逐文件讲解

### 3.1 `DefaultEngine.ini`（引擎的"总控台"）

我们当前 84 行，段结构：

```
DefaultEngine.ini
├─ [/Script/EngineSettings.GameMapsSettings]        [3-4]    默认地图
│  └─ GameDefaultMap=/Engine/Maps/Templates/OpenWorld     ← 引擎模板图，Lyra 不是这个
├─ [/Script/WindowsTargetPlatform.WindowsTargetSettings] [6-31]  DX12/SM6 + 音频缓冲
├─ [/Script/Engine.RendererSettings]                [33-48]   Lumen + 虚拟阴影 + 距离场
├─ [/Script/LinuxTargetPlatform.LinuxTargetSettings] [50-51]  Vulkan SM6
├─ [/Script/HardwareTargeting.HardwareTargetingSettings] [53-57]  桌面 / 最高画质
├─ [/Script/WorldPartitionEditor.WorldPartitionEditorSettings] [59-60]  WP 转换命令
├─ [/Script/Engine.UserInterfaceSettings]           [62-65]   DPI 与控件授权
├─ [/Script/Engine.Engine]                          [67-69]   ★ TP_Blank 类重定向（模板残留）
└─ [/Script/AndroidFileServerEditor.AndroidFileServerRuntimeSettings] [71-83]  安卓文件服务
```

**关键差异（Lyra 有、我们没有）**：

| Lyra 段 | Lyra 行号 | 作用 | 缺了会怎样 |
|---|---|---|---|
| `[/Script/Engine.Engine]` 里的 `GameEngine=` / `AssetManagerClassName=` / `WorldSettingsClassName=` / `LocalPlayerClassName=` / `GameUserSettingsClassName=` | 18-25 | **把引擎默认类换成 Lyra 的子类** | 现在为空 → 跑的是引擎原生 GameEngine，Lyra 的资产/本地玩家/设置全不生效 |
| `[/Script/Engine.CollisionProfile]` | 138-207 | 碰撞 Profile 全套（14 个基础 + 5 个 Lyra 自定义 + 4 个追踪通道） | **`WaterBodyCollision` 找不到 → 就是你看到的水体报错** |
| `[/Script/Engine.PhysicsSettings]` | 299-327 | 物理子步进 / 表面类型 | 物理表现与 Lyra 不一致 |
| `[/Script/Engine.AudioSettings]` | 221-242 | 默认 SoundClass / Submix | 现在报 `No default SoundConcurrencyObject` |
| `[ConsoleVariables]` / `[SystemSettings]` | 123-136 | 网络与手柄死区 | 联机手感差异 |
| `[/Script/Engine.GarbageCollectionSettings]` | 56-57 | GC 剔除 | 内存策略不同 |
| `[Core.Log]` | 59-60 | `LogHotfixManager=Log` | 热更日志看不到 |

**当前 `[/Script/Engine.Engine]` 段是个 bug**：

```ini
[/Script/Engine.Engine]
+ActiveGameNameRedirects=(OldGameName="TP_Blank",NewGameName="/Script/Tenkaichi")
+ActiveGameNameRedirects=(OldGameName="/Script/TP_Blank",NewGameName="/Script/Tenkaichi")
```

这是**建工程时选的模板留下的**（TP_Blank = 空白模板）。它说「把 `/Script/TP_Blank` 这个模块重定向到 `/Script/Tenkaichi`」——但我们现在**根本没有 `Tenkaichi` 模块**（模块已改名为 `LyraGame`）。Lyra 原文也没有这两行。

> ⚠️ 建议：这两行**删掉**。留着不崩，但属于「工程里出现 Tenkaichi 字样」，违反零改名铁律。

---

### 3.2 `DefaultGame.ini`（项目的"业务登记簿"）

我们当前 13 行：

```
DefaultGame.ini
├─ [/Script/EngineSettings.GeneralProjectSettings]  [2-3]   工程 ID
├─ [StartupActions]                                 [5-7]   StarterContent 资源包
├─ [SectionsToSave]                                 [9-10]  保存哪些段
└─ [/Script/Engine.AssetManagerSettings]            [12-13] ★ AssetManager 扫描规则（上次加的）
```

**逐段说明**：

| 段 | 作用 | 与 Lyra 的差别 |
|---|---|---|
| `GeneralProjectSettings` | `ProjectID` 是工程唯一标识，UE 自动生成 | Lyra 还多 `Description` / `ProjectName=Lyra`；我们的 `ProjectID` 是自己的，保留 |
| `[StartupActions]` + `InsertPack=StarterContent.upack` | 建工程时勾了「初学者内容包」，编辑器启动时自动挂载 | **Lyra 没有这段**（Lyra 不用 StarterContent）。留着不报错，但属于模板痕迹 |
| `[/Script/Engine.AssetManagerSettings]` | 告诉 AssetManager：哪些资产类型要扫描、怎么命名、怎么打包 | 我们只有 1 条（`GameFeatureData`）；Lyra 有 **8 条**（见下） |

**Lyra 的 8 条扫描规则**（`DefaultGame.ini:41-48`）：

```
├─ Map                            /Script/Engine.World                     通用关卡
├─ LyraGameData                   /Script/LyraGame.LyraGameData            ★ S07 才有这个类
├─ PrimaryAssetLabel              /Script/Engine.PrimaryAssetLabel         分包标签
├─ GameFeatureData                /Script/GameFeatures.GameFeatureData     ← 我们现在只有这条
├─ LyraExperienceDefinition       /Script/LyraGame.LyraExperienceDefinition ★ S08
├─ LyraUserFacingExperienceDefinition  /Script/LyraGame.LyraUserFacingExperienceDefinition ★ S08
├─ LyraLobbyBackground           /Script/LyraGame.LyraLobbyBackground     ★ S08
└─ LyraExperienceActionSet       /Script/LyraGame.LyraExperienceActionSet ★ S08
```

> 📌 **为什么现在只能加 1 条**：其余 7 条里 5 条指向 `/Script/LyraGame.*`，那些类 S07/S08 才建。现在抄进来 → 引擎找不到类 → 反而多一批警告。**S07 建完 `ULyraAssetManager` 后一次性补齐**。

**Lyra 还有、我们完全没有的段**（等对应阶段补）：

| Lyra 段 | Lyra 行号 | 补的时机 |
|---|---|---|
| `[/Script/LyraGame.LyraPlayerController]` | 6-11 | S12 Player |
| `[/Script/GameplayAbilities.AbilitySystemGlobals]` | 12-25 | S10 GAS |
| `[/Script/GameFeatures.GameFeaturesSubsystemSettings]` | 31-32 | S20 GameFeatures |
| `[/Script/LyraGame.LyraAssetManager]` | 34-36 | S07 资产 |
| `[/Script/LyraGame.LyraUIManagerSubsystem]` / `LyraUIMessaging` | 56-62 | S18 UI |
| `[/Script/CommonLoadingScreen.CommonLoadingScreenSettings]` | 63-66 | S02 插件 |
| `[/Script/CommonInput.CommonInputSettings]` / `CommonUI.CommonUISettings` | 67-78 | S13 输入 / S18 UI |
| `[/Script/UnrealEd.ProjectPackagingSettings]`（132 行） | 79-187 | S23 打包 |
| `[/Script/LyraGame.LyraContextEffectsSettings]` 等 4 段 | 201-226 | S17 表现 / S21 网络 |

---

### 3.3 `DefaultEditor.ini`（**只在编辑器里生效**）★ 当前放错了东西

我们当前只有 2 行：

```ini
[/Script/Engine.CollisionProfile]
+Profiles=(Name="WaterBodyCollision",...)
```

**这是错的。** 原因见第二节：`UCollisionProfile` 是 `config=Engine`，只从 `GEngineIni`（`DefaultEngine.ini`）读。写在 `DefaultEditor.ini` 里，编辑器**一辈子读不到**，水体报错会照旧出现。

> ✅ 正确做法：把这两行**整体搬到 `DefaultEngine.ini` 末尾**，然后删掉 `DefaultEditor.ini` 里的内容。

**Lyra 的 `DefaultEditor.ini`（85 行）实际装的是**：

| Lyra 段 | Lyra 行号 | 作用 |
|---|---|---|
| `[/Script/UnrealEd.UnrealEdOptions]` | 4-12 | 「新建蓝图」对话框里置顶哪些类（`LyraCharacter` / `LyraGameMode` / `LyraGameplayAbility` / `GameplayEffect`…） |
| `[UI]` `!ContentDirectories=ClearArray` | 16-17 | 清空"永远打包的 UI 目录"（向后兼容开关） |
| `[/Script/CommonUI.CommonUIEditorSettings]` | 19-21 | CommonUI 编辑器预览用哪个 TextStyle / ButtonStyle 模板 |
| `[/Script/UMGEditor.UMGEditorProjectSettings]` | 25-45 | 藏掉引擎原生 Widget（强制用 CommonUI 版）+ 13 档调试分辨率 |
| `[/Script/UnrealEd.BlueprintEditorProjectSettings]` | 47-49 | 允许 PIE 期间重编译哪些 GameplayCue 基类 |
| `[/Script/Localization.LocalizationSettings]` | 51-68 | 本地化收集目标（**最大的段，18 行长记录**） |
| `[/Script/SourceControl.SourceControlPreferences]` | 70-72 | Perforce changelist 标签 |
| `[/Script/UnrealEd.EditorProjectAppearanceSettings]` | 74-75 | 属性面板显示可搜索名 |
| `[/Script/GameFeaturesEditor.GameFeaturesEditorSettings]` | 77-81 | GameFeature 插件模板列表 |
| `[TargetDomain]` | 83-84 | 迭代脚本白名单 |

> 📌 这些段分别属于：`UnrealEdOptions` 要等 S11/S08 类建好才有意义；`CommonUIEditorSettings` 归 S18；`LocalizationSettings` 归 S19/S22；`GameFeaturesEditorSettings` 归 S20。**S00 阶段 `DefaultEditor.ini` 就该是空文件或不存在。**

---

### 3.4 `DefaultInput.ini`（输入总线）

我们当前 85 行，是**模板生成的完整轴配置**：

```
DefaultInput.ini
├─ [/Script/Engine.InputSettings]
│  ├─ -AxisConfig=…  × 7          [2-8]    先删掉 7 个引擎默认轴（Gamepad_LeftX/Y、RightX/Y、MouseX/Y/2D）
│  ├─ +AxisConfig=…  × 51         [9-60]   再重新加（手柄 + 鼠标 + Vive + MixedReality + Oculus + ValveIndex）
│  ├─ bAltEnterTogglesFullscreen … [61-74] 一堆输入行为开关
│  ├─ DefaultViewportMouseCaptureMode=CapturePermanently_IncludingInitialMouseDown  [75]
│  ├─ FOVScale=0.011110 / DoubleClickTime=0.200000  [77-78]
│  ├─ DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput       [79]
│  ├─ DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent [80]  ← Lyra 是 LyraInputComponent
│  ├─ DefaultTouchInterface=/Engine/MobileResources/…/DefaultVirtualJoysticks  [81]
│  └─ -ConsoleKeys=Tilde / +ConsoleKeys=Tilde / +ConsoleKeys=Caret            [82-84]
```

**Lyra 的 `DefaultInput.ini`（51 行）长这样**：

```
defaultInput.ini（Lyra）
├─ [/Script/Engine.PlayerInput]                    [1-10]   ★ 8 条调试快捷键（Insert 传送 / K 自杀 / PageUp 切目标…）
├─ [/Script/Engine.InputSettings]                  [12-34]
│  ├─ +AxisConfig × 10                             [13-23]  只留手柄 + 鼠标，没有 VR 那 40 条
│  ├─ DefaultTouchInterface=None                   [31]     ★ 不是 DefaultVirtualJoysticks
│  ├─ DefaultInputComponentClass=/Script/LyraGame.LyraInputComponent  [33]  ★ 换成 Lyra 子类
│  └─ bFilterInputByPlatformUser=True              [34]     ★ 我们是 False
├─ [/Script/CommonUI.CommonUIInputSettings]        [36-41]   ★ UI 返回键映射 + 模拟光标加速曲线
├─ [/Script/OpenXRInput.OpenXRInputSettings]       [43-45]   ★ XR 输入
└─ [/Script/EnhancedInput.EnhancedInputDeveloperSettings] [47-50]  ★ 用户自定义按键（LyraInputUserSettings）
```

**三个必须知道的关键差异**：

| 项 | 我们 | Lyra | 影响 |
|---|---|---|---|
| `DefaultInputComponentClass` | `EnhancedInput.EnhancedInputComponent` | `LyraGame.LyraInputComponent` | Lyra 的 `ULyraInputComponent` 带「按住/松开」绑定与输入配置注入；**S13 建完类后必须改** |
| `DefaultTouchInterface` | `DefaultVirtualJoysticks` | `None` | 我们是模板值，触屏摇杆会浮出来 |
| `[/Script/Engine.PlayerInput]` 调试快捷键 | 无 | 8 条 | `Insert` 传送、`K` 自爆等作弊键，Lyra 开发期必备 |

> 📌 **S00 阶段不动 `DefaultInput.ini`**，因为它整体属于 S13（Input 输入系统）。现在改一半反而乱。

---

## 四、必须先修的两处

| # | 问题 | 改法 |
|---|---|---|
| 1 | `[/Script/Engine.CollisionProfile]` 写在 `DefaultEditor.ini`，**永远读不到** | 整段搬到 `DefaultEngine.ini` 末尾；`DefaultEditor.ini` 清空（或直接删掉该文件） |
| 2 | `DefaultEngine.ini:68-69` 的 `ActiveGameNameRedirects=TP_Blank → /Script/Tenkaichi` | **删掉这 2 行**：`/Script/Tenkaichi` 模块已不存在，Lyra 原文也没有 |

> 修 #1 之后再启动编辑器，`碰撞描述文件设置不包括水体碰撞描述文件` 才会真正消失。**你现在这版跑起来，那条报错还会出现**——不是没改，是改错了本子。

---

## 五、四个文件的「归属阶段」总表（谁什么时候补齐）

| 文件 | S00 应有内容 | 后续补齐 |
|---|---|---|
| `DefaultEngine.ini` | 渲染/RHI/硬件档位/默认地图 + `CollisionProfile` 的 `WaterBodyCollision` | **S06** 补全 5 个 Lyra 碰撞 Profile + 4 个追踪通道；**S07** 补 `[/Script/Engine.Engine]` 的类替换 6 行；**S11** 补 `PhysicsSettings`；**S17** 补 `AudioSettings` |
| `DefaultGame.ini` | `GeneralProjectSettings` + `GameFeatureData` 1 条扫描 | **S07** 补齐 8 条扫描规则 + 5 个 `bShouldXXX`；**S08** 补 Experience 段；**S20** 补 GameFeatures 段 |
| `DefaultEditor.ini` | **空** | **S18**（CommonUI/UMG 编辑器）、**S19/S22**（本地化）、**S20**（GameFeature 模板） |
| `DefaultInput.ini` | 模板默认值不动 | **S13** 整体替换为 Lyra 51 行版 |

---

## 六、一句话总结

**四个 ini 对应四本登记簿：`Engine`=引擎总控（渲染/碰撞/物理/类替换）、`Game`=项目业务（资产扫描/打包）、`Editor`=只管编辑器（蓝图默认类/UMG/本地化）、`Input`=输入。引擎按类头上 `config=XXX` 去对应本子读，写错本子静默失效——你现在的 `WaterBodyCollision` 就写在 `DefaultEditor.ini` 里，必须搬到 `DefaultEngine.ini` 才生效。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\CollisionProfile.h:159`（`UCLASS(config=Engine, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274`（`GConfig->GetSection(TEXT("/Script/Engine.CollisionProfile"), false, GEngineIni)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\AssetManagerSettings.h:66`（`UCLASS(config = Game, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\InputSettings.h:21`（`UCLASS(config=Input, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\EngineSettings\Classes\GameMapsSettings.h:94`（`UCLASS(config=Engine, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\RendererSettings.h:297`（`UCLASS(config=Engine, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Editor\UnrealEd\Classes\Preferences\UnrealEdOptions.h:84`（`UCLASS(Config=Editor)`）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:16-26,47-51,78-136,138-207,221-242,299-327,367-368`（各段原文）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:1-5,38-54`（工程信息 + AssetManager 8 条）
- `E:\ue5\LyraStarterGame\Config\DefaultEditor.ini:1-85`（全文）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:1-51`（全文）
- `E:\ue5\game_design\code\Tenkaichi\Config\`（四个文件实查：84 / 13 / 2 / 85 行）