# 04 — `Source/LyraGame/LyraGame.Build.cs` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（**91 行 / 79 代码行**，权威源 Lyra 5.4）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.Build.cs`
> **⚠️ 本节不能独立编译**——完整依赖 S01~S05 的插件模块，插件就位前 UBT 会报缺模块；编译里程碑顺延到 S05 末。
>
> ⚠️ **修订记录（2026-11-23）**：本讲义曾错把 **5.6 的 106 行版**（含 `ExternalRpcRegistry`/`HTTPServer`/Shipping 分支）当成 5.4 源，已改回权威源 5.4 的 **91 行 / 79 代码行 / 22 公共 + 25 私有**。总纲写的"照抄（91 行）"本来是对的。

---

## 一、这个文件解决什么问题（为什么）

🔑 **一句话比喻**：`Build.cs` 是模块的**采购清单**——UBT 拿着它去仓库领依赖（Core / UMG / GAS / 各种插件），清单少写一行，代码里对应的 `#include` 就找不到。

场景：你写 `#include "GameplayAbilities/..."`，编译器不会自己去仓库翻，它只看这张清单上有没有 `GameplayAbilities`。所以**清单必须和 Lyra 一字不差**。

核心设计三点：
1. **模块名 = 类名 = `LyraGame`**：零改名，与 Lyra 完全一致。
2. **公共依赖 22 个 + 私有依赖 25 个**：公共依赖会"传染"给依赖本模块的其他模块；私有依赖只在本模块 `.cpp` 里用。
3. **Shipping 下禁用 DrawDebug**：`SHIPPING_DRAW_DEBUG_ERROR=1`，防止调试绘制漏进发行包。

---

## 二、内部结构全景

```
LyraGame.Build.cs（91 行 / 79 代码行）
├─ 版权注释（1）                         // 中文注释
├─ using UnrealBuildTool;（3）
├─ public class LyraGame : ModuleRules（5-6）
└─ public LyraGame(...) 构造函数（7-90）
   ├─ PCHUsage（9）
   ├─ PublicIncludePaths（11-15）        // "LyraGame"
   ├─ PrivateIncludePaths（17-20）       // 空
   ├─ PublicDependencyModuleNames（22-47）   → 22 个
   ├─ PrivateDependencyModuleNames（49-77）  → 25 个（到 "DTLSHandlerComponent",）
   ├─ DynamicallyLoadedModuleNames（79-82）  → 空
   ├─ PublicDefinitions（85）            → SHIPPING_DRAW_DEBUG_ERROR=1
   └─ 两个 Setup（88-89）
      ├─ SetupGameplayDebuggerSupport
      └─ SetupIrisSupport
```

> ⚠️ 5.4 版**没有** `ExternalRpcRegistry` / `HTTPServer` / `WITH_RPC_REGISTRY` 那些分支——那是 5.6 才有的，别抄 5.6。

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraGame.Build.cs` | 否（只列模块名字符串） | ⚠️ 不能——引用的插件模块 S01~S05 才落地 | 编译验收放到 S05 末 |

关键依赖表：

| 依赖 | 作用 | 哪个阶段补上 |
|---|---|---|
| `ModularGameplay` / `ModularGameplayActors` | `AModularPawn` 等基类 | S01 |
| `GameplayMessageRuntime` / `AsyncMixin` | 消息路由、异步 mixin | S01 |
| `CommonLoadingScreen` | 加载界面接口 | S02 |
| `CommonGame` / `CommonUser` / `UIExtension` | UI 层栈、用户登录 | S03 |
| `GameSettings` / `GameSubtitles` | 设置、字幕 | S04 |
| `ReplicationGraph` / `SignificanceManager` | 网络复制优化、性能裁剪 | 引擎自带插件 |

---

## 四、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `04_LyraGame.Build.cs复制文件.md`**。
2. 你照抄到 `Source\LyraGame\LyraGame.Build.cs`，注释行整行不抄；**必须删掉旧 `Source\Tenkaichi\` 目录**。
3. 敲完说「下一步」→ 剔注释逐字比对（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 05。

---

**下一步**：第 5 个文件 `Source\LyraGame\LyraGameModule.cpp`（20 行，模块入口）。
