# 05 — `Source/LyraGame/LyraGameModule.cpp` —— 总 md

> 依据：`E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp`（**20 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGameModule.cpp`
> **⚠️ 本节不能独立编译**——本文件本身只依赖引擎 `Core`，但项目整体要等 S05 插件就位（04 的完整 `Build.cs` 列了 S01~S05 的模块）。

---

## 一、这个文件解决什么问题（为什么）

🔑 **一句话比喻**：这个文件是游戏的**身份证登记处**——引擎启动时问"谁是主游戏模块？"，最后那行 `IMPLEMENT_PRIMARY_GAME_MODULE` 举着手回答"我，`LyraGame`"。

没有它，UBT 编出来的 dll 里没有游戏入口，引擎加载模块时找不到主模块，游戏根本起不来。

核心设计三点：
1. **空实现就是全部**：`StartupModule` / `ShutdownModule` 目前都是空的——Lyra 故意什么都不做，模块入口只负责"存在"。
2. **继承 `FDefaultGameModuleImpl`**：引擎给的默认游戏模块基类，已经把生命周期虚函数准备好了，我们只挑需要的重写。
3. **宏把类和模块名绑在一起**：`IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame")` 三个参数分别是实现类、模块名、兼容旧名。

---

## 二、内部结构全景

```
LyraGameModule.cpp（20 行）
├─ 版权注释（1）
├─ #include "Modules/ModuleManager.h"（3）
├─ 块注释 FLyraGameModule（6-8）
├─ class FLyraGameModule : public FDefaultGameModuleImpl（9-18）
│  ├─ StartupModule() override（11-13，空实现）
│  └─ ShutdownModule() override（15-17，空实现）
└─ IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame")（20）
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraGameModule.cpp` | 否 | ⚠️ 文件本身可以；项目整体等 S05 插件 | 否 |

关键依赖表：

| 依赖 | 作用 | 源码位置 |
|---|---|---|
| `Modules/ModuleManager.h` | 提供 `FDefaultGameModuleImpl` 和主模块宏 | 引擎 `Core\Public\Modules\ModuleManager.h` |
| `FDefaultGameModuleImpl` | 空实现的游戏模块基类 | `ModuleManager.h:767-768` |
| `IModuleInterface::StartupModule` | 模块加载时回调 | `ModuleInterface.h:31-33` |
| `IModuleInterface::ShutdownModule` | 模块卸载时回调 | `ModuleInterface.h:55-57` |
| `IMPLEMENT_PRIMARY_GAME_MODULE` | 注册主游戏模块 | `ModuleManager.h:973` / `1007` |

> 源码依据补充：`E:\ue5\game_design\code\LyraStarterGame` 是实查到的 Lyra 5.4 源根（文档写的 `E:\ue5\LyraStarterGame` 不存在）。

---

## 四、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `05_LyraGameModule.cpp复制文件.md`**。
2. 你照抄到 `Source\LyraGame\LyraGameModule.cpp`，注释行不抄；块注释按讲义要求压成一行中文注释。
3. 本段新元素（`FLyraGameModule`、`FDefaultGameModuleImpl`、`StartupModule`、`ShutdownModule`、`IMPLEMENT_PRIMARY_GAME_MODULE`）已就地列在复制文件 md 里，并登记进台账。
4. 敲完说「下一步」→ 剔注释逐字比对（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 06。

---

**下一步**：第 6 个文件 `Source\LyraGame\LyraLogChannels.h`（14 行，4 个日志频道声明）。
