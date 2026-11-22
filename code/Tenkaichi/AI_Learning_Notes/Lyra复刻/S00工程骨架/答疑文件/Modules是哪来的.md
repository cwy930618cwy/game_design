# 答疑：`#include "Modules/ModuleManager.h"` 里的 `Modules` 是哪来的？

> 触发：用户在 `05_LyraGameModule.cpp复制文件.md` 第 14 行看到这行 include，问"这个 Modules 哪来的？"
> 现象：IDE 里 `LyraGameModule.cpp` 标红、提示"属性找不到"。

---

## 一、先给结论（一句话）

**`Modules` 不是你工程里的目录，是引擎 `Core` 模块的 Public 头文件目录 `Engine\Source\Runtime\Core\Public\` 下的一个子目录。**

UBT 编译时会自动把每个依赖模块的 `Public\` 路径加进 include 搜索列表；我们 `Build.cs` 第 24 行写了 `"Core"`，所以引擎的 `...\Core\Public\` 被加进来 → `#include "Modules/ModuleManager.h"` 就能解析到：

```
D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h
                                                       ↑
                                            这里的 Modules 是引擎的目录
```

---

## 二、类比（为什么 include 路径能"跨工程"）

🔑 **一句话比喻**：`#include` 像**寄快递写地址**，不需要写"我自己的房间号"，只要写"**收件人知道的地址**"。

```
你的工程 Tenkaichi\Source\LyraGame\LyraGameModule.cpp
   └─ #include "Modules/ModuleManager.h"
        │
        ├─ 编译器按「搜索路径列表」依次找：
        │   ① 当前文件所在目录（Tenkaichi\Source\LyraGame\）→ 没有 Modules\
        │   ② 各依赖模块的 Public\ 路径（UBT 自动加）：
        │      └─ Core 模块 → D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\
        │           └─ Modules\ModuleManager.h  ✅ 找到了！
```

> **关键认知**：include 写的不是"从工程根开始的路径"，而是"从**某个 include 搜索根**开始的相对路径"。`Modules/` 从 `Core\Public\` 这个搜索根开始算。

---

## 三、引擎源码证据（已实查）

| 符号 / 路径 | 位置 |
|---|---|
| `Modules\ModuleManager.h` 本体 | `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h` ✅ 存在 |
| `FDefaultGameModuleImpl`（我们继承的基类） | 同文件 **第 767-768 行**：`class FDefaultGameModuleImpl : public FDefaultModuleImpl` |
| `FDefaultModuleImpl` | 同文件 **第 760-761 行**：`: public IModuleInterface { };` |
| `StartupModule()` 虚函数 | `Engine\Source\Runtime\Core\Public\Modules\ModuleInterface.h` **第 31 行** |
| `ShutdownModule()` 虚函数 | 同文件 **第 55 行** |
| `IMPLEMENT_PRIMARY_GAME_MODULE` 宏 | `ModuleManager.h` **第 973 / 989 / 1007 行**（按分支三处定义） |

**Lyra 官方源码也这么写**（不是我自己发明的）：
```
E:\ue5\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp:3  → #include "Modules/ModuleManager.h"
E:\ue5\LyraStarterGame\Source\LyraGame\LyraDevelopmentStatics.cpp:12 → 同样
E:\ue5\LyraStarterGame\Source\LyraEditor\Private\EditorValidator_Blueprints.cpp:9 → 同样
```

---

## 四、为什么 IDE 标红说"属性找不到"（这才是你遇到的真问题）

**注意区分两件事**：

| 层面 | 状态 |
|---|---|
| 代码本身 | ✅ 正确，逐字一比一（`codeOurs=11 / codeEngine=11 / diff=0`） |
| 工程编译环境 | ❌ 还没就位 → IDE IntelliSense 找不到符号，标红 |

**IDE 标红的真正原因**（按影响从大到小）：

1. **UBT 还编不过整个工程**：当前 S00 阶段 `LyraGame.Build.cs` 依赖了 S01~S05 才建的插件模块（如 `ModularGameplayActors`），这些模块现在不存在 → UBT 报 `Could not find definition for module 'ModularGameplayActors'` → **makefile 都没生成成功** → IDE 拿不到完整的 include 路径和预编译头 → `FDefaultGameModuleImpl` 之类的符号全标红。
   > 实测：跑 `Build.bat LyraGame Win64 Development -Project=Tenkaichi.uproject` 输出：
   > `Could not find definition for module 'ModularGameplayActors', (referenced via Target -> LyraGame.Build.cs)`
2. **Target 名变了**：工程模块从 `Tenkaichi` 改成 `LyraGame`，IDE 里如果还在编译 `TenkaichiEditor` target，会报 `Couldn't find target rules file for target 'TenkaichiEditor'`。
3. **`.uproject` 改了但没重新生成工程文件**：模块名/插件列表全变了，VS/Rider 的 IntelliSense 缓存还是旧的。

> ⚠️ **总纲早就写明**：S00 的里程碑是"空模块可编译"要等 S05 插件就位。现在 S00 第 5 步，插件一个没建 → **编译不过、IDE 标红是预期现象**，不是你这行代码写错了。

---

## 五、怎么办（不用改代码）

1. **这行 include 完全不用动** —— 它就是 Lyra 原写法，权威源第 3 行。
2. 想消红，得按路线继续把 S01~S05 的插件建出来，让 UBT 能生成完整 makefile；到那时 IDE 自动能解析到 `Core\Public\Modules\ModuleManager.h`。
3. 中途想临时验证：可在 IDE 里手动刷新 IntelliSense / 重新生成工程文件，但只要模块依赖还缺，编译仍会失败。
4. **不要为了消红去改 `#include`**：写成 `#include "D:/ue5/.../ModuleManager.h"` 绝对路径、`#include "../Engine/..."` 相对工程路径、或删掉这行 —— 都违反一比一，且换机器必挂。

---

## 六、一句话结论

**`Modules` 是引擎 `Core` 模块 Public 目录下的子目录，不是你工程里的文件夹；因为 `Build.cs` 依赖了 `Core`，UBT 把 `Core\Public\` 加进 include 搜索路径，`Modules/ModuleManager.h` 才能被找到。IDE 标红是因为 S00 阶段插件依赖还没建、UBT 编不过、IntelliSense 拿不到路径——代码本身没问题，继续按路线走即可。**
