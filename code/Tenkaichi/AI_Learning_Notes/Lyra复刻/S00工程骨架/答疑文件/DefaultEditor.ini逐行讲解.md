# `Config\DefaultEditor.ini` — 逐行讲解（你自己这份 2 行）

> 对象：`E:\ue5\game_design\code\Tenkaichi\Config\DefaultEditor.ini`（**2 行**）
> 原文件**一个字都没动**，这份 md 只是把每行搬到下面、在**上方**加一行 `;` 中文注释。
> 代码行 = 你文件里的原样；`;` 开头的行 = 我加的讲解，你的文件里没有。

---

## 全文逐行注释

```ini
; ─────────────────────────────────────────────
; L1：段头。UCollisionProfile = 「项目设置 → 引擎 → 碰撞」
;
; 🔴 这里放错文件了！
;     UCollisionProfile 的类声明是：
;         Engine\Classes\Engine\CollisionProfile.h:159
;         UCLASS(config=Engine, defaultconfig, MinimalAPI)
;                                    ↑ config=Engine
;     config=Engine 的意思是「只从 DefaultEngine.ini 读」。
;
;     引擎读碰撞配置的代码是写死的：
;         Engine\Private\Collision\CollisionProfile.cpp:274
;         GConfig->GetSection(TEXT("/Script/Engine.CollisionProfile"), false, GEngineIni);
;                                                                           ↑ GEngineIni
;     GEngineIni 就是 DefaultEngine.ini。
;
;     DefaultEditor.ini 对应的全局变量是 GEditorIni，只在编辑器启动时读，
;     而且只读 UCLASS(config=Editor) 的类（比如 UUnrealEdOptions）。
;
; ✅ 结论：这 2 行要整体搬到 DefaultEngine.ini 末尾，本文件恢复成空文件。
; ─────────────────────────────────────────────
[/Script/Engine.CollisionProfile]
; L2：新增一个碰撞预设（Profile）
;
;      Name="WaterBodyCollision"               预设名字；Water 插件启动时按这个名字去查，查不到就报错
;      CollisionEnabled=QueryOnly              只参与查询（射线/重叠检测），不参与物理模拟
;      bCanModify=False                        用户不能在项目设置里改它（插件锁定）
;      ObjectTypeName=""                       物体类型留空；Lyra 原文也是空串
;                                              （如果点报错弹窗的「是」，引擎会自动写成 "WorldStatic"，就与 Lyra 不一致了）
;      CustomResponses=(...)                   对各个碰撞通道的响应：
;            WorldDynamic = Overlap            与世界动态物体重叠
;            Pawn         = Overlap            与角色重叠
;            Visibility   = Ignore             不挡可见性射线（子弹射线不被水面挡住）
;            Camera       = Ignore             不挡相机射线（镜头穿水面不抖）
;            PhysicsBody  = Overlap            与物理体重叠
;            Vehicle      = Overlap            与载具重叠
;            Destructible = Overlap            与可破坏物重叠
;      HelpMessage="Default Water Collision Profile (Created by Water Plugin)"
;                                              说明文字，写在项目设置 UI 里给人看
+Profiles=(Name="WaterBodyCollision",CollisionEnabled=QueryOnly,bCanModify=False,ObjectTypeName="",CustomResponses=((Channel="WorldDynamic",Response=ECR_Overlap),(Channel="Pawn",Response=ECR_Overlap),(Channel="Visibility",Response=ECR_Ignore),(Channel="Camera",Response=ECR_Ignore),(Channel="PhysicsBody",Response=ECR_Overlap),(Channel="Vehicle",Response=ECR_Overlap),(Channel="Destructible",Response=ECR_Overlap)),HelpMessage="Default Water Collision Profile (Created by Water Plugin)")
```

---

## 这个文件为什么只有 2 行（而且放错位置）

### 3 个事实

| # | 事实 | 来源 |
|---|---|---|
| 1 | `UCollisionProfile` 是 `config=Engine` | `CollisionProfile.h:159` |
| 2 | 读碰撞配置写死用 `GEngineIni` | `CollisionProfile.cpp:274` |
| 3 | `GEngineIni` = `DefaultEngine.ini`；`GEditorIni` = `DefaultEditor.ini` | 引擎全局变量定义 |

### 结论

**`[/Script/Engine.CollisionProfile]` 段写进 `DefaultEditor.ini` → 引擎永远读不到 → 水体报错照旧出现。**

你现在这版启动编辑器，「碰撞描述文件设置不包括水体碰撞描述文件的条目」这条错误**还会再弹一次**。改错文件，问题没有解决。

---

## 怎么改（就两步）

### 第 1 步：把这 2 行搬到 `DefaultEngine.ini` 末尾

```ini
; 追加到 E:\ue5\game_design\code\Tenkaichi\Config\DefaultEngine.ini 的最后
[/Script/Engine.CollisionProfile]
+Profiles=(Name="WaterBodyCollision",CollisionEnabled=QueryOnly,bCanModify=False,ObjectTypeName="",CustomResponses=((Channel="WorldDynamic",Response=ECR_Overlap),(Channel="Pawn",Response=ECR_Overlap),(Channel="Visibility",Response=ECR_Ignore),(Channel="Camera",Response=ECR_Ignore),(Channel="PhysicsBody",Response=ECR_Overlap),(Channel="Vehicle",Response=ECR_Overlap),(Channel="Destructible",Response=ECR_Overlap)),HelpMessage="Default Water Collision Profile (Created by Water Plugin)")
```

### 第 2 步：把 `DefaultEditor.ini` 清空

删掉这 2 行即可，文件保留为空（或直接删掉文件，引擎会自动重建）。

> 📌 严格一比一的话，Lyra 的 `DefaultEditor.ini` 有 85 行内容，但那些段（蓝图默认类 / UMG / 本地化 / GameFeature 模板）都属于 S18~S20。**S00 阶段的 `DefaultEditor.ini` 就该是空的。**

---

## 顺便：Lyra 的 `DefaultEditor.ini` 是怎么分流的

Lyra 那 85 行 / 10 个段，全部是 `config=Editor` 或编辑器专用段：

| Lyra 段 | Lyra 行 | 归属阶段 | 干什么的 |
|---|---|---|---|
| `[/Script/UnrealEd.UnrealEdOptions]` | 4-12 | S11 / S08 | 「新建蓝图」对话框置顶哪些类（`LyraCharacter` / `LyraGameMode` / `LyraGameplayAbility` / `GameplayEffect`…） |
| `[UI]` `!ContentDirectories=ClearArray` | 16-17 | S18 | 清空「永远打包的 UI 目录」（向后兼容开关） |
| `[/Script/CommonUI.CommonUIEditorSettings]` | 19-21 | S18 | CommonUI 编辑器预览用的 TextStyle / ButtonStyle 模板 |
| `[/Script/UMGEditor.UMGEditorProjectSettings]` | 25-45 | S18 | 藏掉引擎原生 Widget（强制用 CommonUI 版）+ 13 档调试分辨率 |
| `[/Script/UnrealEd.BlueprintEditorProjectSettings]` | 47-49 | S10 | 允许 PIE 期间重编译哪些 GameplayCue 基类 |
| `[/Script/Localization.LocalizationSettings]` | 51-68 | S19 / S22 | 本地化收集目标（18 行长记录，本文件最大段） |
| `[/Script/SourceControl.SourceControlPreferences]` | 70-72 | S23 | Perforce changelist 标签（`#jira` / `#robomerge`） |
| `[/Script/UnrealEd.EditorProjectAppearanceSettings]` | 74-75 | S06 | 属性面板显示可搜索名 |
| `[/Script/GameFeaturesEditor.GameFeaturesEditorSettings]` | 77-81 | S20 | GameFeature 插件模板列表 |
| `[TargetDomain]` | 83-84 | S22 | 迭代脚本白名单（`+IterativeClassScriptPackageAllowList=Allow,<ProjectRoot>`） |

> ✅ 注意：Lyra 的这 85 行里**没有半行碰撞配置**。碰撞全在 `DefaultEngine.ini:138-207`（70 行）。这就是最直接的证据 —— 你放错本子了。

---

## 一句话

**`DefaultEditor.ini` 现在只有 2 行，而且放错文件了：`UCollisionProfile` 是 `config=Engine`，只认 `DefaultEngine.ini`。搬到那边去、把本文件清空，水体报错才会真正消失。Lyra 的 85 行编辑器配置全在 S18~S22 才补。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Config\DefaultEditor.ini:1-2`（你这份全文，逐行对应上面注释）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\Engine\CollisionProfile.h:159`（`UCLASS(config=Engine, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Private\Collision\CollisionProfile.cpp:274`（`GConfig->GetSection(..., GEngineIni)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Water\Source\Editor\Private\WaterEditorModule.cpp:339-355`（`CheckForWaterCollisionProfile`，报错来源）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Water\Source\Editor\Private\WaterEditorModule.cpp:357-381`（`AddWaterCollisionProfile`，点弹窗「是」执行的代码，会把 `ObjectTypeName` 写成 `WorldStatic`）
- `E:\ue5\LyraStarterGame\Config\DefaultEditor.ini:1-85`（Lyra 全文，10 段，**零碰撞配置**）
- `E:\ue5\LyraStarterGame\Config\DefaultEngine.ini:138-207`（Lyra 碰撞配置 70 行全在 Engine.ini）