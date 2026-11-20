# 04 — `Source\LyraGame\LyraGame.Build.cs`（S00 精简版）—— 总 md

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（**91 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.Build.cs`
> **✅ 本节是 S00 能编译的关键**

---

## 一、这个文件解决什么问题（为什么）

`Build.cs` 是**模块的依赖清单**。UBT 编译 `LyraGame` 模块时读它，回答四件事：

1. **这个模块叫什么** —— `public class LyraGame : ModuleRules`
2. **头文件从哪找** —— `PublicIncludePaths.Add("LyraGame")`，让 `#include "Character/LyraCharacter.h"` 能直接写
3. **依赖哪些引擎模块** —— `PublicDependencyModuleNames` / `PrivateDependencyModuleNames`
4. **特殊编译选项** —— `PublicDefinitions`、`SetupGameplayDebuggerSupport`、`SetupIrisSupport`

🔑 **一句话比喻**：`Build.cs` 是「材料清单」—— 盖这栋楼（模块）需要哪些建材（引擎模块），少一样就停工（编译失败）。

---

## 二、为什么 S00 必须用「精简版」

Lyra 原版依赖 **22 个公共模块 + 25 个私有模块**，其中一大半来自我们还没建的插件：

| 依赖模块 | 来自哪个插件 | S00 有吗 |
|---|---|---|
| `ModularGameplayActors` | `Plugins\ModularGameplayActors` | ❌ S01 |
| `AsyncMixin` | `Plugins\AsyncMixin` | ❌ S01 |
| `GameplayMessageRuntime` | `Plugins\GameplayMessageRouter` | ❌ S01 |
| `CommonLoadingScreen` | `Plugins\CommonLoadingScreen` | ❌ S02 |
| `CommonGame` / `CommonUser` | `Plugins\CommonGame` / `CommonUser` | ❌ S03 |
| `UIExtension` | `Plugins\UIExtension` | ❌ S03 |
| `GameSettings` / `GameSubtitles` | `Plugins\GameSettings` / `GameSubtitles` | ❌ S04 |

**如果 S00 就写全量依赖** → UBT 报 `Unable to find module 'CommonGame'`，一行都编不过。

所以 S00 用**精简版**：只留「引擎自带 + 已启用引擎插件」的模块。

> ⚠️ **S06 第一件事**：把本文件整份替换成 Lyra 完整版（91 行）。届时 11 个插件全部就位，依赖能全部满足。

---

## 三、内部结构全景

```
LyraGame.Build.cs（S00 精简版）
└─ class LyraGame : ModuleRules                    [5-91]
   └─ LyraGame(ReadOnlyTargetRules)                [7-90]
      ├─ PCHUsage = UseExplicitOrSharedPCHs        [9]
      ├─ PublicIncludePaths  → "LyraGame"          [11-15]
      ├─ PrivateIncludePaths → 空                   [17-20]
      ├─ PublicDependencyModuleNames（6 个）        [22-47]
      │  ├─ Core / CoreUObject / Engine
      │  └─ GameplayTags / GameplayTasks / GameplayAbilities
      ├─ PrivateDependencyModuleNames（3 个）       [49-77]
      │  └─ InputCore / DeveloperSettings / EnhancedInput
      ├─ DynamicallyLoadedModuleNames → 空          [79-82]
      ├─ PublicDefinitions                          [84-85]
      │  └─ SHIPPING_DRAW_DEBUG_ERROR=1
      ├─ SetupGameplayDebuggerSupport(Target)       [88]
      └─ SetupIrisSupport(Target)                   [89]
```

---

## 四、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraGame.Build.cs` | ❌ 无（纯 UBT 配置） | ✅ UBT 直接识别 | ❌ 不需要 |

**关键依赖表**

| 项 | 作用 |
|---|---|
| `ModuleRules` / `ReadOnlyTargetRules` | UBT 基类，来自 `UnrealBuildTool` |
| `PCHUsageMode.UseExplicitOrSharedPCHs` | 优先共享 PCH，显著加快编译 |
| `PublicIncludePaths` 里的 `"LyraGame"` | 相对 `Source\` 目录，等于 `Source\LyraGame\` |
| `GameplayAbilities` | 来自 `.uproject` 已启用的 `GameplayAbilities` 插件 |
| `EnhancedInput` | 来自 `.uproject` 已启用的 `EnhancedInput` 插件 |
| `SetupGameplayDebuggerSupport` | UBT 内置方法，自动加 `GameplayDebugger` 依赖 |
| `SetupIrisSupport` | UBT 内置方法，自动加 Iris 网络复制依赖 |

**S00 精简版 vs Lyra 完整版对照**：

| 项 | Lyra 完整版 | S00 精简版 | 何时收敛 |
|---|---|---|---|
| `PublicDependencyModuleNames` | 22 个 | **6 个** | S06 补全 |
| `PrivateDependencyModuleNames` | 25 个 | **3 个** | S06 补全 |
| `PublicIncludePaths` | `"LyraGame"` | 同 | 一致 |
| `PCHUsage` | `UseExplicitOrSharedPCHs` | 同 | 一致 |
| `PublicDefinitions` | `SHIPPING_DRAW_DEBUG_ERROR=1` | 同 | 一致 |
| `SetupGameplayDebuggerSupport` | 有 | 有 | 一致 |
| `SetupIrisSupport` | 有 | 有 | 一致 |

**S06 要补回的 16 个公共依赖**：

```
CoreOnline, ApplicationCore, PhysicsCore, AIModule, ModularGameplay,
ModularGameplayActors, DataRegistry, ReplicationGraph, GameFeatures,
SignificanceManager, Hotfix, CommonLoadingScreen, Niagara, AsyncMixin,
ControlFlows, PropertyPath
```

**S06 要补回的 22 个私有依赖**：

```
Slate, SlateCore, RenderCore, NetCore, RHI, Projects, Gauntlet, UMG,
CommonUI, CommonInput, GameSettings, CommonGame, CommonUser, GameSubtitles,
GameplayMessageRuntime, AudioMixer, NetworkReplayStreaming, UIExtension,
ClientPilot, AudioModulation, EngineSettings, DTLSHandlerComponent
```

---

## 五、怎么教（推进节奏）

本轮两件套：

| # | 产物 | 路径 |
|---|---|---|
| ① | 总 md（本文件） | `S00工程骨架\04_LyraGame.Build.cs_总md.md` |
| ② | 复制文件（代码 + 每行上方中文注释） | `S00工程骨架\04_LyraGame.Build.cs复制文件.md` |

> ⚠️ **建完这个文件后，旧的 `Source\Tenkaichi\` 模块要删掉**（`Tenkaichi.Build.cs` / `Tenkaichi.cpp` / `Tenkaichi.h`），否则 `.uproject` 里没有 `Tenkaichi` 模块、UBT 会报错。这一步由你自己操作。

---

**下一步**：第 5 轮 `Source\LyraGame\LyraGameModule.cpp`（20 行，模块入口）。