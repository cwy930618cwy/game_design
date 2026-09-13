# 02 — Source 下的 `.cs` 文件全解：Target.cs 与 Build.cs 都是干嘛的

> **定位**：`00` 介绍了三件套，`01` 拆了 `.uproject`。现在走进代码目录 `Source/`，把里面的 12 个 `.cs` 文件一个不落讲清楚——**它们不是游戏逻辑，而是"告诉引擎怎么编译/打包"的构建脚本（C# / UBT）**。
>
> **提醒**：UE 里后缀 `.cs` 的不是游戏代码（游戏代码是 `.h/.cpp`），它们只参与"编译期"，不进游戏运行。

---

## 一、Source 目录全景：两种文件、三类角色

Lyra 的 `Source/` 长这样（你本地那份）：

```
Source/
├── LyraGame/                 ← 游戏逻辑代码模块（.h/.cpp，运行时）
│   └── LyraGame.Build.cs      ← 声明 LyraGame 模块能依赖谁
├── LyraEditor/               ← 编辑器工具代码模块（.h/.cpp，仅编辑器）
│   └── LyraEditor.Build.cs    ← 声明 LyraEditor 模块能依赖谁
│
├── LyraGame.Target.cs         ← 主目标（TargetType.Game）
├── LyraClient.Target.cs       ← 客户端目标
├── LyraServer.Target.cs       ← 专用服务器目标
├── LyraEditor.Target.cs       ← 编辑器目标
├── LyraGameEOS.Target.cs      ┐
├── LyraGameSteam.Target.cs    │
├── LyraGameSteamEOS.Target.cs │  ← CustomConfig 变体（Steam / EOS 的打包版）
├── LyraServerEOS.Target.cs    │
├── LyraServerSteam.Target.cs  │
└── LyraServerSteamEOS.Target.cs ┘
```

| 角色 | 是谁 | 回答什么问题 |
|---|---|---|
| **Target.cs（目标）** | 根目录那 10 个 | "要编出哪几种程序？"（编辑器/游戏/客户端/专服…） |
| **Build.cs（模块）** | `LyraGame/`、`LyraEditor/` 内各 1 个 | "每个模块能 include/链接谁？" |

> 💡 **场景记忆**：`Source/` 里 `.cs` 决定"能编出几种 exe"；`*.Build.cs` 决定"每种 exe 里代码能用哪些模块"；`uproject` 决定"这是不是个项目"。三者 = 编译三大件。

---

## 二、两个 `Build.cs`：模块的"依赖清单"

### ① `LyraGame/LyraGame.Build.cs`（游戏主模块）

类名 `LyraGame : ModuleRules`，核心只有一件事：**堆依赖**。

```csharp
PublicDependencyModuleNames.AddRange( /* 别人也能间接用 */ );
PrivateDependencyModuleNames.AddRange( /* 只有本模块能用 */ );
```

- **Public 依赖**（写头文件时 include，会传导）：`GameplayAbilities / GameplayTags / ModularGameplay / ModularGameplayActors / GameFeatures / ReplicationGraph / DataRegistry / Niagara / Hotfix…`
- **Private 依赖**（只在 .cpp 用）：`EnhancedInput / CommonUI / CommonGame / CommonUser / GameSettings / GameplayMessageRuntime / UIExtension / DTLSHandlerComponent…`
- 还配了**编译开关**：Shipping 下关掉 RPC 调试接口（防漏洞）、开 `Iris`（新网络复制）等。

### ② `LyraEditor/LyraEditor.Build.cs`（编辑器模块）

依赖全是编辑器系：`UnrealEd / EditorFramework / ToolMenus / DataValidation / MessageLog / GASEditor / SourceControl…`。

> 对比：`LyraGame` 引 UMG/CommonUI（运行时 UI），`LyraEditor` 引 UnrealEd/ToolMenus（编辑器按钮菜单）。**游戏包不需要 Editor 模块**——这就是为什么 `01` 篇里 Editor 只在编辑器目标编译。

> 💡 **场景记忆**：你想在 LyraGame 里 include `CommonUI.h`——若某天报错说找不到头文件，先回来看它有没有进 `LyraGame.Build.cs` 的依赖名单（就是你之前 `code.Build.cs` 加 GAS 三件套干的事）。

---

## 三、三个"基础 Target"：Type 决定编出什么程序

| Target.cs | `Type` | 编译产物 | Lyra 用它 |
|---|---|---|---|
| `LyraGame.Target.cs` | `Game` | 一个"游戏进程"（逻辑含客户端+服务器，常用于单机/测试） | 编辑器和多数平台的默认游戏目标 |
| `LyraClient.Target.cs` | `Client` | 纯客户端（不含服务器逻辑） | 打包分发给玩家 |
| `LyraServer.Target.cs` | `Server` | 专用服务器（无画面、无玩家输入） | 开线上专服 |
| `LyraEditor.Target.cs` | `Editor` | 编辑器程序 | 开发时用 |

每种结构都类似：

```csharp
public class LyraClientTarget : TargetRules
{
	public LyraClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;                        // ① 定类型
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });   // ② 要编的模块
		LyraGameTarget.ApplySharedLyraTargetSettings(this);      // ③ 套共享设置
	}
}
```

三个可记住的点：

1. **`Type`** 一个枚举就决定产物性质（看 `02` 全称的 `TargetType`）。
2. **`ExtraModuleNames`** 列的是要编译的模块名（对应 `Source/` 下的目录）。
3. **都调用 `ApplySharedLyraTargetSettings(this)`** —— 共享设置都集中放在 `LyraGameTarget` 这个类里（见第五节），避免 10 个文件各写一遍。

> 💡 **场景记忆**：打包 Steam 版要勾 `LyraClient`（发给玩家的是纯客户端）；跑 64 人服务器要编 `LyraServer` 并部署到云主机（没显卡也能跑，因为无渲染）。

---

## 四、`LyraEditor.Target.cs` 特例：为什么它最"胖"

```csharp
Type = TargetType.Editor;
ExtraModuleNames = { "LyraGame", "LyraEditor" };   // 多带一个编辑器模块
EnablePlugins.Add("RemoteSession");                // 真机预览用的远程会话插件
```

编辑器 = 游戏逻辑 + 编辑器工具都在一个进程里，所以模块列表 = `LyraGame + LyraEditor`。

---

## 五、CustomConfig 家族：一套代码，8 个打包变体

真正的**继承链**：

```
LyraGameTarget（主，Type=Game）
├── LyraGameEOSTarget        CustomConfig = "EOS"
├── LyraGameSteamTarget      CustomConfig = "Steam"
└── LyraGameSteamEOSTarget   CustomConfig = "SteamEOS"

LyraServerTarget（Type=Server）
├── LyraServerEOSTarget      CustomConfig = "EOS"
├── LyraServerSteamTarget    CustomConfig = "Steam"
└── LyraServerSteamEOSTarget CustomConfig = "SteamEOS"
```

这些子类**几乎不写逻辑**，只干一件事——给目标贴一个 **`CustomConfig` 标签**：

```csharp
public class LyraGameSteamEOSTarget : LyraGameTarget
{
	public LyraGameSteamEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "SteamEOS";   // Steam 发行 + EOS 联机跨平台
	}
}
```

`CustomConfig` 的名字会决定**读哪一套 `Config/*SteamEOS.ini`**、**用哪个联机后台**（EOS 还是 Steam）。这样同一份代码，换一个打包配置 = 换一个平台发行版。

> 💡 **场景记忆**：游戏上 Steam，直接用 `LyraGameSteam.Target`；还想和主机跨平台联机，改选 `LyraGameSteamEOS.Target`（Steam 发行、EOS 拉通跨平台房间）——代码一行不改，只换编译目标。

---

## 六、`LyraGame.Target.cs` 还藏了两个共享函数

主文件看着最长（283 行），但它真正的构造函数很短，大头是两个 **static 共享函数**，被所有 Target 复用：

### ① `ApplySharedLyraTargetSettings(Target)` —— 全 Lyra 统一编译规矩

| 设置 | 作用 |
|---|---|
| `DefaultBuildSettings = V5` / `IncludeOrderVersion = Latest` | 用最新引擎编译规范 |
| `ShadowVariableWarningLevel = Error` | 变量遮蔽警告直接算编译错误（保代码质量） |
| `bUseLoggingInShipping = true` | 正式版也允许留日志 |
| 证书校验 / ini 读取限制 | 正式包安全加固 |
| 非编辑器禁用 `OpenImageDenoise` | 减包体 |
| `ConfigureGameFeaturePlugins()` | 见下 |

### ② `ConfigureGameFeaturePlugins(Target)` —— 自动开关玩法包

它去扫描 `Plugins/GameFeatures/` 下所有 `.uplugin`，读每个描述里的开关字段（`EnabledByDefault`、`ExplicitlyLoaded`、`EditorOnly`、`RestrictToBranch`、`NeverBuild`…），然后统一调 `Target.EnablePlugins/DisablePlugins`。

> 效果：**打包时自动带上/剔除玩法包**，而不用人肉在编辑器里勾。你可以在构建机上设环境变量让所有 GameFeature 全编（`ShouldEnableAllGameFeaturePlugins` 那段注释）。

> 💡 **场景记忆**：发"免费版"不想含 TopDownArena，不必删文件——把那个玩法包 `.uplugin` 里标记条件改一下，构建脚本自动不编它。

---

## 七、总结图：Source/ 下 12 个 .cs 解剖（左：文件 / 右：它决定什么）

```
          Source/ 下的 12 个 .cs —— 全是"构建图纸"，不是游戏逻辑
 ════════════════════════════════════════════════════════════════════
  左：文件长什么样                               │ 右：它决定了什么
 ───────────────────────────────────────────────┼────────────────────
  ┌─ Build.cs ×2（藏在模块目录里）                │
  │ LyraGame/LyraGame.Build.cs                  │ LyraGame 模块能依赖谁
  │   class LyraGame : ModuleRules              │  Public: GAS/GameFeatures/
  │   Public/Private/DynamicallyLoaded          │     ReplicationGraph/...
  │     DependencyModuleNames                   │  Private: EnhancedInput/
  │                                             │     CommonUI/CommonGame/...
  │ LyraEditor/LyraEditor.Build.cs              │ LyraEditor 模块能依赖谁
  │   class LyraEditor : ModuleRules            │  UnrealEd/ToolMenus/
  │                                             │  → 只在编辑器目标里编译
  ├─ Target.cs ×10（全在 Source 根目录）          │
  │                                             │
  │ ① 基础目标 ×4（每种编一种程序）                │
  │  LyraGameTarget                             │ Type=Game   ← 默认游戏
  │  LyraClientTarget                           │ Type=Client ← 纯客户端包
  │  LyraServerTarget                           │ Type=Server ← 云主机专服
  │  LyraEditorTarget                           │ Type=Editor ← 开发编辑器
  │      ExtraModuleNames = {LyraGame,          │   → 比游戏目标多带一个
  │        LyraEditor}                          │      LyraEditor 模块
  │                                             │
  │ ② CustomConfig 变体 ×6（靠继承，只贴标签）     │
  │  LyraGameEOSTarget      ┐                   │ CustomConfig="EOS"
  │  LyraGameSteamTarget    ├ :LyraGameTarget   │ CustomConfig="Steam"
  │  LyraGameSteamEOSTarget ┘                   │ CustomConfig="SteamEOS"
  │  LyraServerEOSTarget    ┐                   │ 继承"专服"，再贴平台标签
  │  LyraServerSteamTarget  ├ :LyraServerTarget │ → 决定读哪套 *.ini
  │  LyraServerSteamEOSTarget┘                  │    用哪个联机后台
  │                                             │
  │ ③ 共享函数（都收在 LyraGameTarget 里）        │
  │  ApplySharedLyraTargetSettings()            │ 全 Lyra 统一编译规矩
  │  ConfigureGameFeaturePlugins()              │ 扫 Plugins/GameFeatures/
  │                                             │   自动 Enable/Disable
  └─────────────────────────────────────────────┘
 ───────────────────────────────────────────────┼────────────────────
  生产结果：                                    │
  Editor 目标 ──► UnrealEditor.exe（开发用）     │
  Game/Client ──► 玩家发行包                    │
  Server 目标 ──► 不带画面的专服程序             │
 ════════════════════════════════════════════════════════════════════
  一句话：Target.cs 说"要编出哪几种程序"，
        Build.cs 说"每种程序能用哪些模块"，
        共享规矩集中在 LyraGameTarget，10 个 Target 全来抄它。
```

---

## 八、和你本仓库 `code` 对照

你的 `code/Source/` 现在结构极简：

```
Source/
├── code/code.Build.cs        ← 只声明了 Core + GAS 三件套依赖（对应本文第二节）
├── code.Target.cs            ← 默认 Game 目标
├── codeEditor.Target.cs      ← 编辑器目标
└── AttributeSet/             ← 你写的教学代码（还没注册成模块）
```

> 等你要做真正的游戏时，沿着 Lyra 这条路径长出来：加 `LyraClient/Server` 这类目标、按平台加 `CustomConfig` 变体、把功能拆成多个模块——`Source/` 里的 `.cs` 就是"地基图纸"。
