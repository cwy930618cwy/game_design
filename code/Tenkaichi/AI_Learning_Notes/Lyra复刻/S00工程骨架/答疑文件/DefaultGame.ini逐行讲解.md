# `Config\DefaultGame.ini` — 逐行讲解（你自己这份 13 行）

> 对象：`E:\ue5\game_design\code\Tenkaichi\Config\DefaultGame.ini`（**13 行**）
> 原文件**一个字都没动**，这份 md 只是把每行搬到下面、在**上方**加一行 `;` 中文注释。
> 代码行 = 你文件里的原样；`;` 开头的行 = 我加的讲解，你的文件里没有。

---

## 全文逐行注释

```ini
; ─────────────────────────────────────────────
; L1：空行（引擎保存时自动留，无作用）
; ─────────────────────────────────────────────

; ─────────────────────────────────────────────
; L2：段头。UGeneralProjectSettings = 「项目设置 → 项目 → 描述」
;     这个类头写的是 UCLASS(config=Game)，所以只从 DefaultGame.ini 读
; ─────────────────────────────────────────────
[/Script/EngineSettings.GeneralProjectSettings]
; L3：工程唯一 ID（32 位 GUID，建工程时自动生成，不要改）
;     引擎用它区分不同工程；拷工程给别人时一般会重生成
ProjectID=E24B20864CB79A09512673892480A740

; ─────────────────────────────────────────────
; L5：段头 [StartupActions] —— 不是 UCLASS 段，是引擎的「启动动作」区
;     作用：编辑器启动时自动执行这些动作（这里是自动挂载资源包）
; ─────────────────────────────────────────────
[StartupActions]
; L6：允许添加资源包 = True
bAddPacks=True
; L7：要插入的资源包
;     PackSource="StarterContent.upack" = 初学者内容包（Content\StarterContent\ 那 267 个文件就是它）
;     PackName="StarterContent" = 包名
;     ⚠️ 这是建工程时勾了「包含初学者内容」留下的；Lyra 原文没有这段
InsertPack=(PackSource="StarterContent.upack",PackName="StarterContent")

; ─────────────────────────────────────────────
; L9：段头 [SectionsToSave] —— 不是 UCLASS 段，管「哪些段要写回 ini」
; ─────────────────────────────────────────────
[SectionsToSave]
; L10：把 [StartupActions] 段标记为「需要保存」
;      作用：编辑器如果改了启动动作，会把 [StartupActions] 写回这个文件
+Section=StartupActions

; ─────────────────────────────────────────────
; L12：段头。UAssetManagerSettings = 「项目设置 → 游戏 → 资产管理器」
;      这个类头写的是 UCLASS(config = Game, defaultconfig)，所以只从 DefaultGame.ini 读
;      （引擎头文件：Engine\Classes\Engine\AssetManagerSettings.h:66）
; ─────────────────────────────────────────────
[/Script/Engine.AssetManagerSettings]
; L13：资产扫描规则 —— 告诉 AssetManager「GameFeatureData 这种资产怎么找、怎么打包」
;      PrimaryAssetType="GameFeatureData"                    资产类型名
;      AssetBaseClass="/Script/GameFeatures.GameFeatureData" 这个类型对应的 C++ 类
;      bHasBlueprintClasses=False                            不按蓝图子类扫描
;      bIsEditorOnly=False                                   不是只在编辑器用（要打包进游戏）
;      Directories=((Path="/Game/Unused"))                   扫描目录（Lyra 用 /Game/Unused 当一个"不扫内容"的占位）
;      SpecificAssets=                                       额外指定的单个资产（空）
;      Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook)
;            Priority=-1        优先级最低（其他规则可覆盖）
;            ChunkId=-1         不指定 pak 分块
;            bApplyRecursively=True  递归扫子目录
;            CookRule=AlwaysCook     打包时一定要 cook（不能因为"没被引用"就跳过）
;      ⚠️ 这 1 条是上次为消 GameFeatures 报错加的，与 Lyra 原文一字不差
+PrimaryAssetTypesToScan=(PrimaryAssetType="GameFeatureData",AssetBaseClass="/Script/GameFeatures.GameFeatureData",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/Game/Unused")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))
```

---

## 四个段各管一件事（一句话版）

| 段 | 行号 | 管什么 | 要不要动 |
|---|---|---|---|
| `[/Script/EngineSettings.GeneralProjectSettings]` | L2-L3 | 工程 ID | ✅ 不动（每工程不同，正常） |
| `[StartupActions]` | L5-L7 | 编辑器启动时自动挂 StarterContent 资源包 | ⚠️ 模板痕迹，Lyra 没有；删了 `Content\StarterContent\` 会失效 |
| `[SectionsToSave]` | L9-L10 | 上面那段要写回 ini | ✅ 不动（跟 L5-L7 成对） |
| `[/Script/Engine.AssetManagerSettings]` | L12-L13 | 资产扫描规则 | ✅ 不动（已与 Lyra 一致） |

---

## 唯一的判断点：StarterContent 要不要留

`[StartupActions]` + `[SectionsToSave]` 这两段（L5-L10）是**建工程时勾了「初学者内容包」**留下的：

- **留着**：`Content\StarterContent\`（267 个文件，模型/材质/音频/粒子）能正常用，做测试关卡很方便
- **删掉**：那 267 个文件会变成「孤立资产」（引擎不再自动挂载这个包），但文件本身还在
- **Lyra 原文**：`DefaultGame.ini:1-226` 里**完全没有**这两段

> 📌 现在**不用动**。等 S23（示例玩法）阶段真要一比一时再删。删之前先想好 `Content\StarterContent\` 里有没有你要用的东西。

---

## 与 Lyra 的差距：还有 213 行没来

Lyra 的 `DefaultGame.ini` 有 **226 行 / 19 个段**，你这里 **13 行 / 4 个段**。

差的 15 个段什么时候补：

| Lyra 段 | Lyra 行 | 补的时机 | 干什么的 |
|---|---|---|---|
| `[/Script/LyraGame.LyraPlayerController]` | 6-11 | S12 | 玩家控制器默认值（视角俯仰限制等） |
| `[/Script/GameplayAbilities.AbilitySystemGlobals]` | 12-25 | S10 | GAS 全局配置（默认 GE、Cue 管理类等） |
| `[/Script/Engine.GameNetworkManager]` | 26-30 | S21 | 网络管理（带宽、超时） |
| `[/Script/GameFeatures.GameFeaturesSubsystemSettings]` | 31-32 | S20 | `GameFeaturesManagerClassName=LyraGameFeaturePolicy` |
| `[/Script/LyraGame.LyraAssetManager]` | 34-36 | **S07** | `LyraGameDataPath` / `DefaultPawnData` |
| **`[/Script/Engine.AssetManagerSettings]` 剩下 7 条规则 + 5 个开关** | 41-54 | **S07** | 见下面「8 条规则」表 |
| `[/Script/LyraGame.LyraUIManagerSubsystem]` | 56-58 | S18 | `DefaultUIPolicyClass` |
| `[/Script/LyraGame.LyraUIMessaging]` | 59-62 | S18 | UI 消息（对话框/通知） |
| `[/Script/CommonLoadingScreen.CommonLoadingScreenSettings]` | 63-66 | **S02** | `DefaultScreenWidgetClass` / `DefaultWidgetLoadedScreenWidgetClass` |
| `[/Script/CommonInput.CommonInputSettings]` | 67-74 | **S13** | 输入类映射表 |
| `[/Script/CommonUI.CommonUISettings]` | 75-78 | S18 | 默认 UI 材质/字体 |
| **`[/Script/UnrealEd.ProjectPackagingSettings]`** | 79-187 | S23 | 打包设置（**109 行，最大的一段**） |
| `[CheatScript.DebugAsyncLoading]` / `[CheatScript.DebugUI]` | 188-200 | S21 | 作弊脚本 |
| `[/Script/LyraGame.LyraContextEffectsSettings]` | 201-203 | **S17** | 情境特效 |
| `[/Script/ShooterCoreRuntime.ShooterCoreRuntimeSettings]` | 204-206 | S23 | ShooterCore 示例插件 |
| `[/Script/LyraGame.LyraAudioSettings]` | 207-218 | **S17** | 音频（音量曲线、混音类） |
| `[/Script/LyraGame.LyraReplicationGraphSettings]` | 219-226 | **S21** | 复制图（网络裁剪规则） |

### AssetManager 的 8 条规则（你只有第 4 条）

| # | PrimaryAssetType | AssetBaseClass | 现在有？ |
|---|---|---|---|
| 1 | `Map` | `/Script/Engine.World` | ❌ |
| 2 | `LyraGameData` | `/Script/LyraGame.LyraGameData` | ❌（**S07 才有这个类**） |
| 3 | `PrimaryAssetLabel` | `/Script/Engine.PrimaryAssetLabel` | ❌ |
| 4 | **`GameFeatureData`** | `/Script/GameFeatures.GameFeatureData` | ✅ **你有这条** |
| 5 | `LyraExperienceDefinition` | `/Script/LyraGame.LyraExperienceDefinition` | ❌（S08） |
| 6 | `LyraUserFacingExperienceDefinition` | `/Script/LyraGame.LyraUserFacingExperienceDefinition` | ❌（S08） |
| 7 | `LyraLobbyBackground` | `/Script/LyraGame.LyraLobbyBackground` | ❌（S08） |
| 8 | `LyraExperienceActionSet` | `/Script/LyraGame.LyraExperienceActionSet` | ❌（S08） |

> ⚠️ **现在不能补**：第 2、5、6、7、8 条指向 `/Script/LyraGame.*` 的类，S07/S08 才建。现在抄进来 → 引擎找不到类 → 反而多一批警告。
>
> 📌 **S07 建完 `ULyraAssetManager` 后**，把 Lyra `:38-54` 整段（含 5 个 `bShouldXXX` 开关）一次性抄过来，与你现在这条合并成完整版。

---

## 一句话

**13 行 / 4 段：工程 ID（不动）、StarterContent（模板痕迹，暂时留）、SectionsToSave（跟上面成对）、AssetManager 1 条扫描规则（已与 Lyra 一致）。没有要改的地方。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Config\DefaultGame.ini:1-13`（你这份全文，逐行对应上面注释）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:1-226`（Lyra 全文 19 段）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:44`（`GameFeatureData` 单条原文，与你 L13 一字不差）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:38-54`（AssetManager 完整段：8 条规则 + 5 个开关）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\AssetManagerSettings.h:66`（`UCLASS(config = Game, defaultconfig)`）