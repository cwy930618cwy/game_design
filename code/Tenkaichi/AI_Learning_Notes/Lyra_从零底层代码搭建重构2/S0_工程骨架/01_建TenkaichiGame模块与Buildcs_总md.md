# 阶段一（L1 模块地基）第 1 步 —— 建 TenkaichiGame 模块 + Build.cs 依赖（总 md）

> **定位**：本文件是「自下而上底层代码搭建」阶段一（模块地基）第 1 步的**总 md**。
> 只讲「这一步要解决什么问题 + 全景 + 要动的文件清单」，**不贴具体代码**。具体 `.h`/`.cpp` 会拆成小 md 逐个教（铁律 20）。
>
> **依据源码**（铁律 00 先查后教，均已实际读取）：
> - `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（106 行）
> - `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp`（20 行）
> - `E:\ue5\LyraStarterGame5.6\LyraStarterGame\LyraStarterGame.uproject`（模块 `LyraGame` 挂载处）

---

## 一、这一步要解决什么问题

现在你的工程里，模块叫 `Tenkaichi`（UE 建项目时自动生成的默认名），里面是 UE 模板给的几行"最小能跑"代码：

```
Source/Tenkaichi/
├── Tenkaichi.Build.cs   ← 只有 Core/CoreUObject/Engine/InputCore/EnhancedInput 几个依赖
├── Tenkaichi.cpp        ← 模块入口（直接用了引擎默认的 FDefaultGameModuleImpl）
└── Tenkaichi.h          ← 一个空头文件
```

**问题**：这套东西是 UE 模板的"通用骨架"，**不是 Lyra 的样子**。具体差在四点：

1. **模块名不对**：Lyra 叫 `LyraGame`，按铁律 24 你应该叫 `TenkaichiGame`（`Game` 要保留），但你现在叫 `Tenkaichi`。
2. **模块数量不对**：Lyra 的 `.uproject` 挂了 **2 个模块**——`LyraGame`（Runtime）+ `LyraEditor`（Editor）。你现在只有 1 个 `Tenkaichi`。一比一还原（铁律 23）要求这两个模块都要建出来，不能只建 Game 漏掉 Editor。
3. **依赖不对**：Lyra 的 `Build.cs` 挂了二十多个模块依赖（GameplayTags、GameplayAbilities、GameFeatures、ModularGameplay 等），你现在只有 5 个。地基没这些依赖，后面 L2~L7 的代码根本编译不过。
4. **模块入口写法不对**：Lyra 自己定义了一个 `FLyraGameModule` 类（继承 `FDefaultGameModuleImpl`），而不是直接用默认实现。你现在直接用 `FDefaultGameModuleImpl`，没自己的模块类。

**这一步的目标**：把这些改成 Lyra 的样子——建出 `TenkaichiGame` + `TenkaichiEditor` 两个模块、按 Lyra 写好 `Build.cs` 依赖、写好自己的模块入口类。这是整个「自下而上」的第一块地基，后面所有层都建立在这个模块之上。

---

## 二、Lyra 的模块结构长什么样（全景）

打开 Lyra 的 `Source/LyraGame/` 目录，模块入口相关文件只有两个（注意：**没有 `LyraGame.h` / `LyraGame.cpp`**）：

```
Source/LyraGame/
├── LyraGame.Build.cs      ← 模块依赖清单（106 行，核心）
├── LyraGameModule.cpp     ← 模块入口（20 行，很短）
├── LyraLogChannels.h/.cpp ← 日志通道（第 1 步后续会教）
├── LyraGameplayTags.h/.cpp← GameplayTags（阶段一后续会教）
└── ...（AbilitySystem/Character/System 等大量子目录，那是 L3~L7 的事）
```

对比一下，你现在多了一个 `Tenkaichi.h` 和一个 `Tenkaichi.cpp`。Lyra 里模块入口只有一个 `LyraGameModule.cpp`，没有单独的 `LyraGame.h/.cpp`。

**所以这一步你要做的，本质是「重命名 + 对齐」**：

| 你现在的文件 | 应该变成（对应 Lyra） |
|-------------|---------------------|
| `Source/Tenkaichi/Tenkaichi.Build.cs` | `Source/TenkaichiGame/TenkaichiGame.Build.cs` |
| `Source/Tenkaichi/Tenkaichi.cpp` | `Source/TenkaichiGame/TenkaichiGameModule.cpp`（改名 + 改内容） |
| `Source/Tenkaichi/Tenkaichi.h` | ❌ 删掉（Lyra 没有这个文件） |

> ⚠️ 注意：文件夹名也要从 `Tenkaichi` 改成 `TenkaichiGame`（铁律 09 目录对应）。

---

## 三、除了 Source 目录，还有三个地方要同步改（改名是连锁的）

模块名从 `Tenkaichi` 改成 `TenkaichiGame`，不是只改文件夹，下面这些地方全都要跟着改，否则编译会"找不到模块"：

| 文件 | 现在 | 要改成 |
|------|------|--------|
| `Tenkaichi.uproject` | `"Name": "Tenkaichi"` | `"Name": "TenkaichiGame"` |
| `Source/Tenkaichi.Target.cs` | `ExtraModuleNames.Add("Tenkaichi")` | `ExtraModuleNames.Add("TenkaichiGame")` |
| `Source/TenkaichiEditor.Target.cs` | `ExtraModuleNames.Add("Tenkaichi")` | `ExtraModuleNames.Add("TenkaichiGame")` |

> 这一条链，Lyra 里对应的是：`.uproject` 里挂了 2 个模块 `"Name": "LyraGame"` + `"Name": "LyraEditor"`，`LyraGame.Target.cs` / `LyraEditor.Target.cs` 里 `ExtraModuleNames.AddRange(new string[] { "LyraGame" })`（Editor 版多一个 `"LyraEditor"`）。逻辑完全一样，只是名字换成 `TenkaichiGame` / `TenkaichiEditor`。
>
> ⚠️ **完整还原提醒（铁律 23 + 25）**：Lyra 实际有 **10 个 Target 文件**，完整清单如下（`Source/` 目录真实列出）：
>
> ```
> LyraClient.Target.cs        LyraGame.Target.cs          LyraServer.Target.cs
> LyraEditor.Target.cs        LyraGameEOS.Target.cs       LyraServerEOS.Target.cs
>                             LyraGameSteam.Target.cs     LyraServerSteam.Target.cs
>                             LyraGameSteamEOS.Target.cs  LyraServerSteamEOS.Target.cs
> ```
>
> 即：`LyraGame` / `LyraEditor` / `LyraClient` / `LyraServer` 四个基础目标 + EOS / Steam / SteamEOS 三种平台的派生组合。且 `LyraGame.Target.cs` 里还有约 280 行的 `ApplySharedLyraTargetSettings`（证书校验、GameFeature 插件扫描、Shipping 配置等）+ `ConfigureGameFeaturePlugins`。**这些都必须一比一完整还原**，会拆成多个小 md 逐个写全，不会省略。

---

## 四、`Build.cs` 里每一行依赖为什么有（全景预览，细节后续拆小 md 讲）

Lyra 的 `LyraGame.Build.cs` 核心结构（`Source/LyraGame/LyraGame.Build.cs`）：

```
PublicDependencyModuleNames（22 个，公开依赖）：
  Core, CoreOnline, CoreUObject, ApplicationCore, Engine, PhysicsCore,
  GameplayTags, GameplayTasks, GameplayAbilities, AIModule,
  ModularGameplay, ModularGameplayActors, DataRegistry, ReplicationGraph,
  GameFeatures, SignificanceManager, Hotfix, CommonLoadingScreen,
  Niagara, AsyncMixin, ControlFlows, PropertyPath

PrivateDependencyModuleNames（26 个，私有依赖）：
  InputCore, Slate, SlateCore, RenderCore, DeveloperSettings,
  EnhancedInput, NetCore, RHI, Projects, Gauntlet, UMG, CommonUI,
  CommonInput, GameSettings, CommonGame, CommonUser, GameSubtitles,
  GameplayMessageRuntime, AudioMixer, NetworkReplayStreaming,
  UIExtension, ClientPilot, AudioModulation, EngineSettings,
  DTLSHandlerComponent, Json

（列表之外，还有 2 个后补私有依赖）：
  ExternalRpcRegistry, HTTPServer   ← 在 AddRange 之后单独 Add 的
```

> **为什么要分 Public / Private**（这一步必讲清楚的核心概念，后续拆小 md 展开）：
> - **PublicDependencyModuleNames（公开依赖）**：不仅本模块能用，**引用本模块的其他模块也能"看到"这些依赖**。比如后面阶段四的 Character 代码要用 GAS，GAS（GameplayAbilities）就必须是公开依赖，否则别的模块引用你时会缺头文件。
> - **PrivateDependencyModuleNames（私有依赖）**：只本模块内部用，不暴露给引用者。比如 UMG/Slate 这种 UI 实现细节，自己用就行，没必要让所有引用你的模块都背上。
> - 还有 `PublicIncludePaths`（第 11~15 行，把 `LyraGame` 目录加进公开头文件搜索路径）、`PrivateIncludePaths`（第 17~20 行，当前为空）、`DynamicallyLoadedModuleNames`（第 80~83 行，动态加载，当前是空的）等，都会逐个讲。

> ⚠️ **完整还原提醒（铁律 23）**：`LyraGame.Build.cs` 除了依赖列表，还有几处**必须一比一照搬**、不能漏（这些是依赖列表之外的真实代码）：
> - `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;`（第 9 行，PCH 用法）
> - `PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");`（第 86 行，Shipping/Test 构建时禁止 DrawDebug 的宏）
> - `ExternalRpcRegistry` + `HTTPServer` 两个私有依赖，以及 Shipping 下的 `WITH_RPC_REGISTRY=0` / `WITH_HTTPSERVER_LISTENERS=0` 条件宏（第 88~101 行）
> - `SetupGameplayDebuggerSupport(Target);`（第 103 行）和 `SetupIrisSupport(Target);`（第 104 行）两个方法调用

> ⚠️ **铁律 23（不做简化）提醒**：这一步 Lyra 的 `Build.cs` 有 22 公开 + 26 私有 + 2 后补依赖，**一个都不能少**，要一比一完整还原。我不会因为"现在还用不到 Niagara/UMG"就帮你删掉——因为它们对应 Lyra 的真实依赖，删了就和 Lyra 对不上，后面阶段编译也过不了。这些依赖会拆成小 md 逐个讲清"它是什么、为什么 Lyra 要挂它"。

---

## 五、模块入口 `LyraGameModule.cpp` 干嘛的（全景）

Lyra 的 `Source/LyraGame/LyraGameModule.cpp` 全文只有 20 行：

```cpp
#include "Modules/ModuleManager.h"

class FLyraGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");
```

它做了三件事（细节后续拆小 md 讲）：
1. **定义自己的模块类** `FLyraGameModule`，继承引擎的 `FDefaultGameModuleImpl`（默认游戏模块实现）。
2. **重写两个生命周期回调**：`StartupModule()`（模块加载时触发）和 `ShutdownModule()`（模块卸载时触发）。现在都是空的，等后面阶段要往里加初始化逻辑。
3. **用 `IMPLEMENT_PRIMARY_GAME_MODULE` 宏注册**：告诉 UE「这是主游戏模块」，三个参数分别是「模块类名、模块名、模块名字符串」。

对应到你，就是 `FTenkaichiGameModule` + `TenkaichiGame`。

---

## 六、这一步的完整清单（你要动的东西，串起来）

按铁律 01「只教不写代码」，**这些都是你自己动手改**，我把清单列清楚，具体每个文件的写法再拆小 md 逐个数：

1. 把文件夹 `Source/Tenkaichi/` 重命名为 `Source/TenkaichiGame/`
2. 把 `Tenkaichi.Build.cs` 改名 `TenkaichiGame.Build.cs`，内容按 Lyra 重写（22 公开 + 26 私有 + 2 后补依赖 + `PublicDefinitions` + `SetupGameplayDebuggerSupport`/`SetupIrisSupport`，一比一）
3. 把 `Tenkaichi.cpp` 改名 `TenkaichiGameModule.cpp`，内容改成 `FTenkaichiGameModule` 类 + `IMPLEMENT_PRIMARY_GAME_MODULE`
4. 删掉 `Tenkaichi.h`（Lyra 没有这个文件）
5. `.uproject` 里模块名改成 `TenkaichiGame`，并补上第二个模块 `TenkaichiEditor`（对应 Lyra 的 `LyraEditor`）；且 `TenkaichiGame` 要一比一带上 `AdditionalDependencies`（`DeveloperSettings`、`Engine`）——Lyra 的 `LyraGame` 模块真实带了这个，不能省略（铁律 23/25）
6. `Tenkaichi.Target.cs` 和 `TenkaichiEditor.Target.cs` 里的 `ExtraModuleNames` 改成 `TenkaichiGame`（Editor 版加 `TenkaichiEditor`）
7. Target 文件要还原成 Lyra 的 **10 个**（`TenkaichiGame`/`TenkaichiEditor`/`TenkaichiClient`/`TenkaichiServer` + EOS/Steam/SteamEOS 组合），并一比一写全 `ApplySharedLyraTargetSettings`（约 280 行）+ `ConfigureGameFeaturePlugins`

---

## 七、下一步（等确认）

本总 md 只讲全景。你看完确认「懂了 / 下一步」后，我会按铁律 20 **逐个拆小 md**，节奏如下（每个都等你反馈再进下一个）：

1. 先教「模块改名 + 目录对应」（文件夹改名、`.uproject` 加 2 个模块、Target.cs 改名、删 `Tenkaichi.h`）
2. 再教「`TenkaichiGame.Build.cs` 依赖」——这是大头，会拆成多个小 md（公开依赖逐个讲、私有依赖逐个讲、`PublicIncludePaths` / `PublicDefinitions` / `SetupGameplayDebuggerSupport` / `SetupIrisSupport` 等）
3. 再教「`TenkaichiGameModule.cpp` 模块入口」
4. 再教「10 个 Target.cs 完整还原」——`ApplySharedLyraTargetSettings` + `ConfigureGameFeaturePlugins` 一比一写全

> 现在请先看这个总 md，确认理解到位。有任何疑问（比如"为什么不能叫 Tenkaichi 一定要叫 TenkaichiGame"、"为什么要删 Tenkaichi.h"）随时问，我再展开。确认后说「下一步」，我从「模块改名」开始拆小 md。