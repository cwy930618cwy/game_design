# 01 — `Tenkaichi.uproject` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（**347 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject`
> **✅ 本节可独立被引擎识别**（但 `Source\LyraGame\` 还没建，需等第 4~9 轮）

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
Tenkaichi.uproject
├─ FileVersion: 3                    ← 格式版本，UE5 固定写 3
├─ EngineAssociation: "5.4"          ← 绑定 UE 5.4
├─ Category: "Samples"               ← 启动器分类（纯展示）
├─ Description: ""                   ← 描述（纯展示）
├─ Modules [1 项]
│  └─ LyraGame
│     ├─ Type: "Runtime"             ← 运行时模块
│     ├─ LoadingPhase: "Default"     ← 引擎启动时加载
│     └─ AdditionalDependencies      ← UHT 前置依赖
│        ├─ DeveloperSettings
│        └─ Engine
├─ Plugins [61 项]                   ← 全部是引擎自带插件
│  ├─ 启用 49 项（GameplayAbilities / CommonUI / EnhancedInput / ModularGameplay …）
│  └─ 禁用 12 项（MagicLeap* / OpenXR* / SteamVR / GearVR / MLSDK …）
└─ EpicSampleNameHash: "451731683"   ← 示例工程哈希，原样保留
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
| `Plugins` 里的 `ModularGameplay` | 模块化 Gameplay，`ALyraPawn` 的基类来源 |
| `Plugins` 里的 `CommonUI` | CommonUI 框架，S03 插件依赖它 |

**S00 阶段的两处渐进式偏差**（S06 收敛）：

| 项 | Lyra 原版 | 我们 S00 | 何时收敛 |
|---|---|---|---|
| `Modules` | `LyraGame` + `LyraEditor` | 只有 `LyraGame` | **S22** 补回 `LyraEditor` |
| `Plugins` | 76 项（61 引擎 + 15 项目） | 61 项（全引擎） | S01~S05 补 10 个基础插件；S23 补 5 个示例玩法 |

> ⚠️ 写了不存在的模块/插件 → UBT 直接报 `Plugin 'X' could not be found`。所以只能「先建目录，再加清单」。

---

## 四、怎么教（推进节奏）

本轮两件套：

| # | 产物 | 路径 |
|---|---|---|
| ① | 总 md（本文件） | `S00工程骨架\01_Tenkaichi.uproject_总md.md` |
| ② | **复制文件（带逐行中文注释）** | `S00工程骨架\01_Tenkaichi.uproject复制文件.md` |

> ⚠️ JSON 不支持注释，所以复制文件里的 `//` 注释**敲的时候要删掉**。文件末尾有「敲完自查」表。

---

**下一步**：第 2 轮 `Source\LyraGame.Target.cs`（279 行，Lyra 最长的 Target 文件）。