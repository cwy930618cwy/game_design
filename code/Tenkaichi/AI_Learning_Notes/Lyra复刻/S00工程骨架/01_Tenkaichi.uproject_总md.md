# 01 — `Tenkaichi.uproject` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（**347 行：2 个 Module / 76 项插件**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject`
> **✅ 本节可独立被引擎识别**（但 `Source\LyraGame\` 还没建，需等第 4~9 轮）
>
> ⚠️ **本版已重写（2026-11-23）**：旧版误写「61 项插件」，实际是 **70 项**。已按本机 UE 5.4 逐个核对。

---

## 一、这个文件解决什么问题（为什么）

`.uproject` 是**工程的身份证**。引擎双击它时只读这一个文件，就知道三件事：

1. **用哪个引擎版本** —— `EngineAssociation: "5.4"`
2. **有哪些代码模块** —— `Modules` 里每一项对应 `Source\<模块名>\` 目录
3. **开哪些插件** —— `Plugins` 里每一项是一个开关

🔑 **一句话比喻**：`.uproject` 是「住户名单 + 开关面板」—— 写了 `LyraGame`，引擎就去 `Source\LyraGame\` 敲门；敲不到门（目录不存在）就报错。

---

## 二、内部结构全景

```
Tenkaichi.uproject（318 行 / 70 项插件）
├─ FileVersion: 3                                ← 格式版本，UE5 固定写 3
├─ EngineAssociation: "5.4"                      ← 绑定 UE 5.4
├─ Category: "Samples"                           ← 启动器分类（纯展示）
├─ Description: ""                               ← 描述（纯展示）
├─ Modules [1 项]
│  └─ LyraGame
│     ├─ Type: "Runtime"                         ← 运行时模块
│     ├─ LoadingPhase: "Default"                 ← 引擎启动时加载
│     └─ AdditionalDependencies                  ← UHT 前置依赖
│        ├─ DeveloperSettings
│        └─ Engine
├─ Plugins [70 项]                               ← 59 引擎 + 9 工程 + 2 豁免
│  ├─ 引擎自带 59 项
│  │  ├─ GAS 系：GameplayAbilities / GameplayTasks 相关
│  │  ├─ UI 系：CommonUI / UMG 相关
│  │  ├─ 输入系：EnhancedInput
│  │  ├─ 框架系：GameFeatures / ModularGameplay / DataRegistry / ReplicationGraph
│  │  ├─ 特效系：Niagara / Volumetrics / Water / Metasound
│  │  └─ 工具系：Gauntlet / MovieRenderPipeline / GeometryScripting / AssetSearch …
│  ├─ 工程插件 9 项（源码 S01~S05 建）
│  │  ├─ S01：AsyncMixin / ModularGameplayActors / GameplayMessageRouter
│  │  ├─ S02：CommonLoadingScreen / CommonUser
│  │  ├─ S03：CommonGame / UIExtension
│  │  └─ S04：GameSettings / GameSubtitles
│  ├─ 本机缺失但豁免 2 项
│  │  ├─ D3DExternalGPUStatistics  ← "Optional": true
│  │  └─ PlayFabParty              ← 白名单只有 XB1/XSX，Win64 跳过
│  └─ 禁用 9 项（本机也缺，但 Enabled:false 不查）
│     ├─ MagicLeap / MagicLeapMedia / MagicLeapPassableWorld
│     ├─ OpenXRHMD / SteamVR / GearVR / LuminPlatformFeatures / MLSDK
│     └─ RuntimePhysXCooking
└─ EpicSampleNameHash: "451731683"               ← 示例工程哈希，原样保留
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `Tenkaichi.uproject` | ❌ 无 | ✅ 引擎可直接识别 | ❌ 不需要 |

**关键依赖表**

| 项 | 作用 |
|---|---|
| `Modules[0].Name = "LyraGame"` | 决定引擎去 `Source\LyraGame\` 找 `LyraGame.Build.cs` |
| `Plugins` 里的 `GameplayAbilities` | GAS 插件，`Build.cs` 依赖它 |
| `Plugins` 里的 `EnhancedInput` | 增强输入插件，`Build.cs` 依赖它 |
| `Plugins` 里的 `ModularGameplay` | 模块化 Gameplay，`ALyraPawn` 的基类链来源 |
| `Plugins` 里的 `CommonUI` | CommonUI 框架，S03 插件依赖它 |
| `Plugins` 里的 `GameFeatures` | Experience 系统靠它加载玩法插件 |

### 插件语义（决定「会不会报错」）

| 写法 | 含义 | 本机缺失时 |
|---|---|---|
| `"Enabled": true` | 必须有 | ❌ **报错** `X plugin is referenced by target but not found` |
| `"Enabled": false` | 禁用 | ✅ 无害 |
| `"Optional": true` | 可选 | ✅ 无害 |
| `"PlatformAllowList"` / `"SupportedTargetPlatforms"` | 平台白名单 | ✅ 当前平台不匹配就跳过 |

> ⚠️ **完整 70 项逐条校验表 + 自查脚本见 `00_S00开工必读_插件校验与配置前置.md` 第二节。**

### S00 阶段的三处渐进式偏差

| 项 | Lyra 原版 | 我们 S00 | 何时收敛 |
|---|---|---|---|
| `Modules` | `LyraGame` + `LyraEditor` | 只有 `LyraGame` | **S22** 补回 `LyraEditor` |
| `Plugins` | 76 项 | **70 项** | `PocketWorlds` → S05；5 个示例玩法 → S23 |
| 已废弃插件 | 9 项 `Enabled:false` | 原样保留 9 项 | 不收敛（一比一） |

> **为什么是 70 不是 76**：Lyra 的 76 项里有 6 个在 `Plugins\GameFeatures\` 和 `Plugins\PocketWorlds\` 下，
> 源码要到 S05 / S23 才复刻。现在写进 `.uproject` → 引擎找不到 `.uplugin` → 报
> `X plugin is referenced by target but not found`。
>
> **顺序铁律：先建插件目录（含 `.uplugin`）→ 再往 `.uproject` 里加清单。**

> 📌 数字对账（2026-11-23 复核）：源 `.uproject` = **347 行 / 2 Module / 76 插件**；
> 我们 S00 = **318 行 / 1 Module / 70 插件**。复制文件 md 的代码块 500+ 行只因逐行加了中文注释。
> **旧版讲义写的「61 项」是错的，已修正。**

---

## 四、怎么教（推进节奏）

本轮两件套：

| # | 产物 | 路径 |
|---|---|---|
| ① | 总 md（本文件） | `S00工程骨架\01_Tenkaichi.uproject_总md.md` |
| ② | **复制文件（带逐行中文注释）** | `S00工程骨架\01_Tenkaichi.uproject复制文件.md` |

> ⚠️ JSON 不支持注释，所以复制文件里的 `//` 注释**敲的时候要删掉**。文件末尾有「敲完自查」表 + 校验脚本。
>
> ⚠️ **开工前必看**：`00_S00开工必读_插件校验与配置前置.md` —— 那里讲了为什么 9 个插件本机没有却不报错、以及 `Config\` 4 个 ini 各欠什么。

---

## 五、本节已知的「预期报错」

| 现象 | 原因 | 何时消失 |
|---|---|---|
| 编译报 `Could not find definition for module 'ModularGameplayActors'` | `Build.cs` 依赖的插件模块 S01 才建 | S05 末 |
| `Asset manager settings do not include a rule for assets of type GameFeatureData` | `DefaultGame.ini` 缺规则 | 现在就修 |
| `碰撞描述文件设置不包括水体碰撞描述文件的条目` | `DefaultEngine.ini` 缺 `WaterBodyCollision` | 现在就修 |
| IDE 里引擎头文件全红 | UBT 编不过 → IntelliSense 拿不到路径 | S05 末 |

---

**下一步**：第 2 轮 `Source\LyraGame.Target.cs`（279 行，Lyra 最长的 Target 文件）。