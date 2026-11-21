# 04 — `Source/LyraGame/LyraGame.Build.cs` —— 总 md

> 依据：`E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（**106 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.Build.cs`
> **⚠️ 本节不能独立编译**——完整一比一版依赖 S01~S05 的 11 个插件模块，插件就位前 UBT 会报缺模块；编译里程碑顺延到 S05 末。
>
> ⚠️ **修订记录（2026-11-21）**：本讲义此前是「S00 精简版（6 公共 + 3 私有）」，**违反 00 号铁律"一比一、不删不跳、不简化"**，已作废并按源文件 106 行全文重写。
> 验收实测：`codeEngine=92 / codeOurs=41 / diff=55`（精简版），**未通过**，需按本讲义重敲。

---

## 一、这个文件解决什么问题（为什么）

🔑 **一句话比喻**：`Build.cs` 是模块的**采购清单**——UBT 拿着它去仓库领依赖（Core / UMG / GAS / 各种插件），清单少写一行，代码里对应的 `#include` 就找不到。

场景：你写 `#include "GameplayAbilities/..."`，编译器不会自己去仓库翻，它只看这张清单上有没有 `GameplayAbilities`。所以**清单必须和 Lyra 一字不差**——这也是为什么"精简版"是错的：它让 S00 能编过，但和 Lyra 对不上号，后面每个 `#include` 都会出问题。

核心设计三点：
1. **模块名 = 类名 = `LyraGame`**：零改名，与 Lyra 完全一致（工程叫 Tenkaichi 没关系，Lyra 官方也是工程名 ≠ 模块名）。
2. **公共依赖 22 个 + 私有依赖 26 个**：公共依赖会"传染"给依赖本模块的其他模块；私有依赖只在本模块 `.cpp` 里用。
3. **Shipping 安全开关**：外部 RPC 框架在发行版里被编掉（`WITH_RPC_REGISTRY=0`），减少攻击面。

---

## 二、内部结构全景

```
LyraGame.Build.cs（106 行）
├─ 版权注释（1）                         // 中文注释
├─ using UnrealBuildTool;（3）
├─ public class LyraGame : ModuleRules（5-6）
└─ public LyraGame(...) 构造函数（7-105）
   ├─ PCHUsage（9）
   ├─ PublicIncludePaths（11-15）        // "LyraGame"
   ├─ PrivateIncludePaths（17-20）       // 空
   ├─ PublicDependencyModuleNames（22-47）   → 22 个
   ├─ PrivateDependencyModuleNames（49-78）  → 26 个（注意末尾 Json 有逗号）
   ├─ DynamicallyLoadedModuleNames（80-83）  → 空
   ├─ PublicDefinitions（86）            → SHIPPING_DRAW_DEBUG_ERROR=1
   ├─ 外部 RPC 框架（90-101）
   │  ├─ ExternalRpcRegistry / HTTPServer（90-91）
   │  └─ if Shipping → 关；else → 开（92-101）
   └─ 两个 Setup（103-104）
      ├─ SetupGameplayDebuggerSupport
      └─ SetupIrisSupport
```

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
| `PropertyPath` / `ExternalRpcRegistry` 等 | 属性路径、外部 RPC | 引擎自带插件 |

---

## 四、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `04_LyraGame.Build.cs复制文件.md`**（完整代码 + 每行上方中文注释）。
2. 你照抄到 `Source\LyraGame\LyraGame.Build.cs`，注释行整行不抄；**必须删掉旧 `Source\Tenkaichi\` 目录**（`.uproject` 只声明了 `LyraGame`）。
3. 敲完说「下一步」→ 我按剔注释逐字比对验收（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 05。

---

**下一步**：第 5 个文件 `Source\LyraGame\LyraGameModule.cpp`（20 行，模块入口）——**等 04 重敲验收通过后**才推进。
