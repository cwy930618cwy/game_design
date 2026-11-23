# 00B — 第二步：Config 四个 ini 校验

> 这是 S00 动手前**第二步**要做的事。
> 目标：**编辑器一启动，`Output Log` 里一条 `Error` 都没有。**
> 依据：本机 `D:\ue5\Epic Games\UE_5.4` 源码实查 + `E:\ue5\LyraStarterGame\Config` 实查

---

## 第 0 步：先建立 2 个认知（2 分钟）

### 认知 1：ini 是「四本登记簿」，写错本子 = 白写

```
Config\
├─ DefaultEngine.ini   ← 引擎级（地图 / 渲染 / 碰撞 / 物理 / 音频 / 类替换）
├─ DefaultGame.ini     ← 项目级（AssetManager / 打包 / GAS 全局）
├─ DefaultEditor.ini   ← 只在编辑器生效（蓝图默认类 / UMG / 本地化）
└─ DefaultInput.ini    ← 输入（轴配置 / 默认 Input 类）
```

每个可配置的类头上都写着 `UCLASS(config=XXX, defaultconfig)`：

| 类 | 类头 | 位置 | 只认哪个文件 |
|---|---|---|---|
| `UCollisionProfile` | `config=Engine` | `Engine\Classes\Engine\CollisionProfile.h:159` | **`DefaultEngine.ini`** |
| `UAssetManagerSettings` | `config = Game` | `Engine\Classes\Engine\AssetManagerSettings.h:66` | **`DefaultGame.ini`** |
| `UInputSettings` | `config=Input` | `Engine\Classes\GameFramework\InputSettings.h:21` | **`DefaultInput.ini`** |
| `UGameMapsSettings` | `config=Engine` | `EngineSettings\Classes\GameMapsSettings.h:94` | **`DefaultEngine.ini`** |
| `URendererSettings` | `config=Engine` | `Engine\Classes\Engine\RendererSettings.h:297` | **`DefaultEngine.ini`** |
| `UUnrealEdOptions` | `Config=Editor` | `Editor\UnrealEd\Classes\Preferences\UnrealEdOptions.h:84` | **`DefaultEditor.ini`** |

🔑 **写错本子不会报错，只会静默失效** —— 引擎根本不去那个文件里找。

### 认知 2：ini 只有 5 种写法

| 写法 | 含义 |
|---|---|
| `[段名]` | 段头，格式 `/Script/<模块>.<类>` |
| `键=值` | 覆盖（有就改，没有就加） |
| `+键=值` | 往数组里**追加**一项 |
| `-键=值` | 从数组里**删除**一项（必须字面完全一致） |
| `!键=ClearArray` | 清空整个数组 |
| `;` 开头 | 注释 |

---

## 第 1 步：修报错 ① —— `GameFeatureData`（3 分钟）

### 现象

启动编辑器，`Output Log` 里出现：

```
LogGameFeatures: Error: Asset manager settings do not include a rule for assets of type GameFeatureData,
                            which is required for game feature plugins to function
LoadErrors: Error: 资产管理器设置不包含类型为GameFeatureData的资产的条目…
             是否将条目添加到PrimaryAssetTypesToScan？
```

> 这两条是**同一条报错**（英文日志 + 中文弹窗）。

### 为什么会报

`.uproject` 启用了 `GameFeatures` 插件。它启动时要确认：资产管理器知不知道「GameFeatureData 这种资产」怎么扫、怎么打包。查不到 → 报错。

引擎源码（`GameFeaturesSubsystem.cpp:494`）：

```cpp
FPrimaryAssetRules GameDataRules = UAssetManager::Get().GetPrimaryAssetRules(DummyGameFeatureDataAssetId);
if (GameDataRules.IsDefault())
{
    // 有 UE 工程就报错（模板工程不报）
    UE_CLOG(bHasProject, LogGameFeatures, Error, TEXT("Asset manager settings do not include a rule for assets of type %s, ..."));
}
```

### 怎么修：往 `Config\DefaultGame.ini` 末尾加 2 行

```ini
[/Script/Engine.AssetManagerSettings]
+PrimaryAssetTypesToScan=(PrimaryAssetType="GameFeatureData",AssetBaseClass="/Script/GameFeatures.GameFeatureData",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/Game/Unused")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))
```

这 2 行与 Lyra `DefaultGame.ini:44` **一字不差**。

### ⚠️ 只加这 1 条，别抄 Lyra 的其余 7 条

Lyra 一共 8 条扫描规则，另外 7 条里 5 条指向 `/Script/LyraGame.*` 的类：

| 第几条 | 类型 | 类 | 现在能加吗 |
|---|---|---|---|
| 1 | `Map` | `/Script/Engine.World` | 可以，但暂时不需要 |
| 2 | `LyraGameData` | `/Script/LyraGame.LyraGameData` | ❌ 类还没建（S07） |
| 3 | `PrimaryAssetLabel` | `/Script/Engine.PrimaryAssetLabel` | 可以，但暂时不需要 |
| 4 | **`GameFeatureData`** | `/Script/GameFeatures.GameFeatureData` | ✅ **就是这条** |
| 5 | `LyraExperienceDefinition` | `/Script/LyraGame.LyraExperienceDefinition` | ❌ 类还没建（S08） |
| 6 | `LyraUserFacingExperienceDefinition` | `/Script/LyraGame....` | ❌ 类还没建（S08） |
| 7 | `LyraLobbyBackground` | `/Script/LyraGame....` | ❌ 类还没建（S08） |
| 8 | `LyraExperienceActionSet` | `/Script/LyraGame....` | ❌ 类还没建（S08） |

> 📌 **早抄的后果**：引擎找不到 `/Script/LyraGame.LyraGameData` 这个类 → 多一批「找不到类」警告。
> **S07 建完 `ULyraAssetManager` 后**，把 Lyra `DefaultGame.ini:38-54` 整段一次性抄过来。

### 修完怎么验证

重启编辑器 → 搜 `GameFeatureData` → **0 条**。

---

## 第 2 步：修报错 ② —— `WaterBodyCollision`（5 分钟，最容易踩坑的一步）

### 现象

```
LoadErrors: Error: 碰撞描述文件设置不包括水体碰撞描述文件的条目，
             水碰撞必须使用该描述文件才能正常工作。
             将条目添加到DefaultEngine.ini？
```

### 为什么会报

`.uproject` 启用了 `Water` 插件。它的编辑器模块启动时检查碰撞配置里有没有 `WaterBodyCollision`（`WaterEditorModule.cpp:347`）：

```cpp
const FName WaterCollisionProfileName = GetDefault<UWaterRuntimeSettings>()->GetDefaultWaterCollisionProfileName();
FCollisionResponseTemplate WaterBodyCollisionProfile;
if (!UCollisionProfile::Get()->GetProfileTemplate(WaterCollisionProfileName, WaterBodyCollisionProfile))
{
    // 有 UE 工程就报错 + 弹窗
    FMessageLog("LoadErrors").Error()->AddToken(...)
}
```

### ⚠️ 关键：这条配置**必须**写在 `DefaultEngine.ini`，不能写在 `DefaultEditor.ini`

证据有两条：

**证据 ① 类头声明**（`CollisionProfile.h:159`）：

```cpp
UCLASS(config=Engine, defaultconfig, MinimalAPI, meta=(DisplayName="Collision"))
      ↑ config=Engine
```

**证据 ② 读取代码写死 ini**（`CollisionProfile.cpp:274`）：

```cpp
const FConfigSection* Configs = GConfig->GetSection(TEXT("/Script/Engine.CollisionProfile"), false, GEngineIni);
                                                                                          ↑ GEngineIni
```

`GEngineIni` = `DefaultEngine.ini`。`DefaultEditor.ini` 对应的是 `GEditorIni`，**引擎读碰撞配置时压根不看它**。

> ❌ **你现在把它写在 `DefaultEditor.ini` 里 → 报错不会消失。** 改对了文件才有效。

### 怎么修：两个动作

**动作 A**：把下面 2 行**追加到 `Config\DefaultEngine.ini` 文件末尾**

```ini
[/Script/Engine.CollisionProfile]
+Profiles=(Name="WaterBodyCollision",CollisionEnabled=QueryOnly,bCanModify=False,ObjectTypeName="",CustomResponses=((Channel="WorldDynamic",Response=ECR_Overlap),(Channel="Pawn",Response=ECR_Overlap),(Channel="Visibility",Response=ECR_Ignore),(Channel="Camera",Response=ECR_Ignore),(Channel="PhysicsBody",Response=ECR_Overlap),(Channel="Vehicle",Response=ECR_Overlap),(Channel="Destructible",Response=ECR_Overlap)),HelpMessage="Default Water Collision Profile (Created by Water Plugin)")
```

**动作 B**：把 `Config\DefaultEditor.ini` 里那 2 行**删掉**，文件留空

### 逐字段解释那行长配置

| 字段 | 值 | 含义 |
|---|---|---|
| `Name` | `"WaterBodyCollision"` | 预设名字；Water 插件按这个名字查，名字必须一模一样 |
| `CollisionEnabled` | `QueryOnly` | 只参与查询（射线 / 重叠），不做物理模拟 |
| `bCanModify` | `False` | 锁定，用户在项目设置里改不了 |
| `ObjectTypeName` | `""` | 物体类型留空（Lyra 原文就是空串） |
| `CustomResponses` | 7 条 | 对各碰撞通道的响应，见下表 |
| `HelpMessage` | `"Default Water Collision Profile (Created by Water Plugin)"` | 项目设置里显示给人看的说明 |

`CustomResponses` 逐条：

| 通道 | 响应 | 为什么这样设 |
|---|---|---|
| `WorldDynamic` | `Overlap` | 与世界动态物体重叠（不阻挡） |
| `Pawn` | `Overlap` | 角色进水里不卡住 |
| `Visibility` | `Ignore` | 子弹射线**不**被水面挡住 |
| `Camera` | `Ignore` | 相机穿过水面不抖 |
| `PhysicsBody` | `Overlap` | 物理体可以掉进水里 |
| `Vehicle` | `Overlap` | 载具可以开进水里 |
| `Destructible` | `Overlap` | 可破坏物可以落水 |

### ⚠️ 千万别点弹窗里的「是」

点「是」会执行 `WaterEditorModule.cpp:357 AddWaterCollisionProfile()`，它自动写进去的是：

```cpp
WaterBodyCollisionProfile.ObjectType = ECollisionChannel::ECC_WorldStatic;   // ← 会被写成 WorldStatic
```

而 Lyra 原文是 `ObjectTypeName=""`（空串）。**点弹窗 = 与 Lyra 不一致。手动抄才准。**

### ⚠️ 也别为了消错去关 `Water` 插件

关掉确实不报了，但：
- 违反「一比一」（Lyra 原样启用）
- 后面关卡里但凡有水体全都会挂

### 修完怎么验证

重启编辑器 → 搜 `碰撞描述文件` → **0 条**。

---

## 第 3 步：修报错 ③ —— 「光照需要重建」（2 分钟，可选但建议做）

### 现象

视口左上角红色提示：**光照需要重建（1 未构建对象）**

### 为什么会这样

`Config\DefaultEngine.ini` 的 `[/Script/Engine.RendererSettings]` 段里**没有** `r.AllowStaticLighting=False`。
它的默认值是 `True` → 场景里只要有一个 Static / Stationary 光源，编辑器就要求烘焙。

### 怎么修：往 `[/Script/Engine.RendererSettings]` 段里加 1 行

```ini
r.AllowStaticLighting=False
```

Lyra 原文在 `DefaultEngine.ini:88` 就是这么写的（Lyra 全用 Lumen，完全不需要烘焙）。

> 📌 这条不算「报错」，只是一条红色提示，不影响运行。**加上更干净，不加也能跑。**

---

## 第 4 步：删掉 2 行错误重定向（1 分钟）

### 位置

`Config\DefaultEngine.ini` 第 67-69 行：

```ini
[/Script/Engine.Engine]
+ActiveGameNameRedirects=(OldGameName="TP_Blank",NewGameName="/Script/Tenkaichi")
+ActiveGameNameRedirects=(OldGameName="/Script/TP_Blank",NewGameName="/Script/Tenkaichi")
```

### 为什么错

| 问题 | 说明 |
|---|---|
| `NewGameName="/Script/Tenkaichi"` | **`Tenkaichi` 模块已经不存在了**（现在叫 `LyraGame`） |
| 来历 | 建工程时选了「TP_Blank 空白模板」自动写的 |
| Lyra 原文 | Lyra `DefaultEngine.ini:16-26` 的 `[/Script/Engine.Engine]` 段**完全没有**这两行 |

### 怎么修

**把这两行删掉**，`[/Script/Engine.Engine]` 段暂时留空（S07 会往里补 9 行类替换）。

> ⚠️ 不删也不崩，但违反「工程里零 `Tenkaichi` 字样」铁律，且是无效重定向。

---

## 第 5 步：四个 ini 的「现在该有什么」清单（对照表）

| 文件 | S00 现在必须有 | S00 现在**不该有**（会引发新报错） |
|---|---|---|
| `DefaultEngine.ini` | 渲染 / RHI / 硬件档位 / 默认地图 | `[/Script/Engine.Engine]` 的 `GameEngine=` 等 9 行（S07/S12/S19/S22） |
| `DefaultEngine.ini` | `r.AllowStaticLighting=False` | — |
| `DefaultEngine.ini` | `[/Script/Engine.CollisionProfile]` + `WaterBodyCollision` | Lyra 的其余 4 个自定义 Profile（S06） |
| `DefaultGame.ini` | `GeneralProjectSettings`（`ProjectID`） | — |
| `DefaultGame.ini` | `[/Script/Engine.AssetManagerSettings]` + `GameFeatureData` 1 条 | 其余 7 条扫描规则（S07/S08） |
| `DefaultEditor.ini` | **空文件** | 任何内容（碰撞段已搬走） |
| `DefaultInput.ini` | 模板默认值不动 | `LyraInputComponent`（S13） |

---

## 第 6 步：改完的自检脚本（1 分钟）

把下面整段复制到 PowerShell：

```powershell
$eng = "E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini"
$gam = "E:\ue5\game_design\code\Tenkaichi\Config\DefaultGame.ini"
$edt = "E:\ue5\game_design\code\Tenkaichi\Config\DefaultEditor.ini"

"==== 1. DefaultGame.ini 有没有 GameFeatureData 规则 ===="
if ((Get-Content $gam -Raw) -match 'PrimaryAssetType="GameFeatureData"') { "  ✅ 有" } else { "  ❌ 缺" }

"==== 2. DefaultEngine.ini 有没有 WaterBodyCollision ===="
if ((Get-Content $eng -Raw) -match 'Name="WaterBodyCollision"') { "  ✅ 有" } else { "  ❌ 缺（或在 DefaultEditor.ini 里）" }

"==== 3. DefaultEngine.ini 有没有 r.AllowStaticLighting=False ===="
if ((Get-Content $eng -Raw) -match 'r\.AllowStaticLighting=False') { "  ✅ 有" } else { "  ⚠️ 缺（会有『光照需要重建』提示）" }

"==== 4. DefaultEngine.ini 有没有错误的 Tenkaichi 重定向 ===="
if ((Get-Content $eng -Raw) -match 'NewGameName="/Script/Tenkaichi"') { "  ❌ 还有，要删" } else { "  ✅ 已清理" }

"==== 5. DefaultEditor.ini 是不是空的 ===="
$edtContent = (Get-Content $edt -Raw -EA SilentlyContinue)
if ([string]::IsNullOrWhiteSpace($edtContent)) { "  ✅ 空" } else { "  ❌ 还有内容，碰撞段要搬走" }
```

### 期望输出

```
==== 1. DefaultGame.ini 有没有 GameFeatureData 规则 ====
  ✅ 有
==== 2. DefaultEngine.ini 有没有 WaterBodyCollision ====
  ✅ 有
==== 3. DefaultEngine.ini 有没有 r.AllowStaticLighting=False ====
  ✅ 有
==== 4. DefaultEngine.ini 有没有错误的 Tenkaichi 重定向 ====
  ✅ 已清理
==== 5. DefaultEditor.ini 是不是空的 ====
  ✅ 空
```

---

## 附录：其余「无害提示」不用管

启动日志里这些**不是错误**，别去修：

| 提示 | 原因 |
|---|---|
| `Failed to load 'aqProf.dll'` / `'VtuneApi.dll'` | 没装 GPU/CPU 分析工具 |
| `Failed to load 'WinPixGpuCapturer.dll'` | 没装 PIX |
| `UTS: Unable to launch the Unreal Trace Server ... 拒绝访问` | 权限问题，不影响 |
| `No default SoundConcurrencyObject specified` | 引擎默认行为，S17 配音频时再管 |
| `LocRes '...zh/XXX.locres' could not be opened` | 没装中文语言包，只影响中文 UI 文本 |

---

## 一句话总结

**第 1 步：`DefaultGame.ini` 加 `GameFeatureData` 1 条（消 GameFeatures 报错）；第 2 步：`WaterBodyCollision` 搬进 `DefaultEngine.ini`、`DefaultEditor.ini` 清空（消碰撞报错，别点弹窗的「是」）；第 3 步：加 `r.AllowStaticLighting=False`（消光照提示）；第 4 步：删 `TP_Blank → /Script/Tenkaichi` 两行；第 5~6 步：跑自检脚本，5 项全 ✅。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameFeatures\Source\GameFeatures\Private\GameFeaturesSubsystem.cpp:487-496`（`GameFeatureData` 规则检查）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Water\Source\Editor\Private\WaterEditorModule.cpp:339-355`（`CheckForWaterCollisionProfile`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Water\Source\Editor\Private\WaterEditorModule.cpp:357-381`（`AddWaterCollisionProfile`，弹窗「是」执行的代码）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274`（`GEngineIni` 硬绑定）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\CollisionProfile.h:159`（`UCLASS(config=Engine, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\AssetManagerSettings.h:66`（`UCLASS(config = Game, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\InputSettings.h:21`（`UCLASS(config=Input, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Editor\UnrealEd\Classes\Preferences\UnrealEdOptions.h:84`（`UCLASS(Config=Editor)`）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:38-54`（AssetManager 完整段 8 条规则）
- `E:\ue5\LyraStarterGame\Config\DefaultGame.ini:44`（`GameFeatureData` 单条原文）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:88`（`r.AllowStaticLighting=False`）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:138-207`（碰撞段 70 行）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:175`（`WaterBodyCollision` 原文）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:16-26`（`[/Script/Engine.Engine]` 9 行类替换，无 `ActiveGameNameRedirects`）
- `E:\ue5\game_design\code\Tenkaichi\Saved\Logs\Tenkaichi.log:914,919,965`（本次实查的三条报错）