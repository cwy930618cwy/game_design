# 09 — `Source/LyraGame/LyraLogChannels.h` —— 总 md

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame\LyraLogChannels.h`（**14 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraLogChannels.h`
> **⚠️ 本节不能独立编译** —— 本文件本身只依赖引擎 `Core`，但整个 `LyraGame` 模块要等 S01~S05 插件就位才能编过。

---

## 一、这个文件解决什么问题（为什么）

### 场景：日志为什么需要「频道」

你写 `UE_LOG(LogTemp, Warning, TEXT("xxx"))` 时，`LogTemp` 就是**日志频道**。编辑器 `Output Log` 面板能按频道过滤 —— 只看 `LogLyraAbilitySystem`、只看 `LogLyraTeams`。

引擎自带 `LogTemp` / `LogActor` / `LogNet` 等几十个频道，但**不可能为 Lyra 的业务预置**「能力系统日志」「队伍日志」。所以每个游戏要自己声明自己的频道。

🔑 **一句话比喻**：日志频道是「广播电台的频率」。`LogTemp` 是公共频道（谁都能用，吵）；`LogLyraTeams` 是 Lyra 的队伍专用频道（只播队伍相关，干净）。

### 核心设计三点

1. **`.h` 里 `DECLARE_LOG_CATEGORY_EXTERN`，`.cpp` 里 `DEFINE_LOG_CATEGORY`** —— 声明/定义分离，和变量一样。多文件 include `.h` 不会重复定义。
2. **`LYRAGAME_API` 前缀** —— 让其他模块（`LyraEditor`、插件）也能用这些频道。没有它 → 只有 `LyraGame` 模块自己能看见。
3. **4 个频道 + 1 个工具函数** —— 不多不少，正好是 Lyra 全工程用到的。

### 4 个频道分别管什么

| 频道 | 谁在用 | 典型日志内容 |
|---|---|---|
| `LogLyra` | 通用 | 游戏流程、初始化、杂项 |
| `LogLyraExperience` | Experience 系统（S08） | 加载 Experience / GameFeature 加载失败 |
| `LogLyraAbilitySystem` | GAS（S10） | 能力授予、GE 应用、属性变化 |
| `LogLyraTeams` | 队伍系统（S09） | 队伍分配、队伍切换 |

### `GetClientServerContextString` 是干嘛的

联机调试时，日志前面加个 `[Server]` / `[Client]`，一眼看出是服务端还是客户端打的：

```
LogLyraTeams: Warning: [Server] Team 1 assigned
LogLyraTeams: Warning: [Client]  Team 1 assigned
```

函数体在 `.cpp`（第 10 轮），`.h` 里只有声明 + 默认参数 `nullptr`。

---

## 二、内部结构全景

```
LyraLogChannels.h（14 行）
├─ 版权注释（1）                                    // 中文注释
├─ #pragma once（3）                                // 防重复 include
├─ #include "Logging/LogMacros.h"（5）              // 两个宏的来源
├─ class UObject;（7）                              // 前置声明（给第 14 行的指针参数用）
├─ 4 个频道声明（9-12）
│  ├─ LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
│  ├─ LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
│  ├─ LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
│  └─ LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);
└─ 工具函数声明（14）
   └─ LYRAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraLogChannels.h` | ❌ 只依赖引擎 `Core` | ⚠️ 文件本身可以；模块整体等 S05 | 编译验收放到 S05 末 |

**关键依赖表**

| 依赖 | 作用 | 源码位置 |
|---|---|---|
| `Logging/LogMacros.h` | 提供 `DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY` 宏 | `Engine\Source\Runtime\Core\Public\Logging\LogMacros.h` |
| `LYRAGAME_API` | 模块导出宏；让其他模块能用这些符号 | 由 UBT 自动生成（`LyraGame` 模块名 + `_API`） |
| `class UObject;` | 前置声明，避免 include `UObject.h` 整个重头文件 | — |
| `FString` | 返回值类型 | `Core`（`Containers/UnrealString.h`） |

> ⚠️ **为什么是 `class UObject;` 而不是 `#include "UObject/Object.h"`**：
> 第 14 行只需要「`UObject*` 这个指针类型存在」就能声明函数，不需要知道 `UObject` 的大小/成员。
> 用前置声明 → 编译快；`#include` 整个 `UObject.h` → 编译慢。

---

## 四、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `09_LyraLogChannels.h复制文件.md`**。
2. 你新建文件 `Source\LyraGame\LyraLogChannels.h`，注释行不抄。
3. 敲完说「下一步」→ 剔注释逐字比对（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 10。
4. **本轮敲完仍然不能编译** —— 缺 `LyraGame.Build.cs` 的插件依赖（S01~S05）。

---

## 五、预期现象（不是错误）

| 现象 | 原因 | 什么时候好 |
|---|---|---|
| IDE 里 `LYRAGAME_API` 标红 | UBT 生成不了 makefile → 宏未展开 | S05 末 |
| IDE 里 `FString` 标红 | 同上 | S05 末 |
| 编译报 `Could not find definition for module 'ModularGameplayActors'` | `LyraGame.Build.cs` 的插件依赖 | S05 末 |

---

**下一步**：第 10 轮 `Source\LyraGame\LyraLogChannels.cpp`（40 行，4 个频道定义 + `GetClientServerContextString` 实现）。