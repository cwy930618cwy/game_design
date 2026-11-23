# 07 — `Source/LyraEditor/LyraEditor.cpp` —— 总 md

> 依据：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.cpp`（**现在 9 行 / 393 B，占位版**）
> 对照源：`E:\ue5\LyraStarterGame\Source\LyraEditor\LyraEditor.cpp`（**329 行 / 10470 B，正式版，S22 才教**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.cpp`
> **⚠️ 本节不能独立编译** —— 依赖 `LyraEditor.Build.cs`（第 06 轮），而后者又依赖 `LyraGame`（S01~S05 后能编）。

---

## 一、这个文件解决什么问题（为什么）

### 场景：`Build.cs` 有了，为什么还要 `.cpp`

上一轮你敲了 `LyraEditor.Build.cs`，那只是「档案」——告诉 UBT 这个模块叫什么、依赖什么。

但 UBT 还要求：**每个模块必须有一个实现了 `IMPLEMENT_MODULE` 的 `.cpp`**。没有它，UBT 会报：

```
Module 'LyraEditor' does not have a valid module implementation
```

🔑 **一句话比喻**：`Build.cs` 是「档案」，`.cpp` 里的 `IMPLEMENT_MODULE` 是「本人到场签字」。光有档案没人签字，模块不算存在。

### 为什么是 `IMPLEMENT_MODULE` 而不是 `IMPLEMENT_PRIMARY_GAME_MODULE`

这两种宏的用途完全不同：

| 宏 | 用在哪 | 一个工程能有几个 |
|---|---|---|
| `IMPLEMENT_PRIMARY_GAME_MODULE` | **主游戏模块**（`LyraGameModule.cpp`，第 05 轮） | **只能 1 个** |
| `IMPLEMENT_MODULE` | **普通模块**（`LyraEditor.cpp` 这种） | 每个模块 1 个，不限总数 |

> ⚠️ 如果 `LyraEditor.cpp` 里错用了 `IMPLEMENT_PRIMARY_GAME_MODULE` → 与 `LyraGameModule.cpp` 重复定义主模块 → 链接报错。

### 核心设计三点

1. **`FDefaultModuleImpl` = 引擎给的「什么都不做」的默认实现** —— 占位阶段正好够用。
2. **`IMPLEMENT_MODULE(实现类, 模块名)` 两个参数** —— 第一个是类，第二个是模块名（必须与 `Build.cs` 的类名、目录名一致）。
3. **S22 会换成 `FLyraEditorModule`** —— 那时才真正干活（注册编辑器菜单、资产校验钩子等），文件从 9 行涨到 329 行。

---

## 二、内部结构全景

```
LyraEditor.cpp（9 行 / 393 B）
├─ 版权 + 占位说明注释（1-5）              ← 中文说明，敲的时候不抄
├─ #include "Modules/ModuleManager.h"（7）  ← IMPLEMENT_MODULE 与 FDefaultModuleImpl 都在这
└─ IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);（9）
```

对比 Lyra 正式版（S22）的结构：

```
LyraEditor.cpp（329 行 / 10470 B）
├─ 30+ 行 #include
├─ class FLyraEditorModule : public IModuleInterface
│  ├─ StartupModule()          ← 注册编辑器菜单 / GameplayCue 扩展 / 资产校验
│  ├─ ShutdownModule()         ← 注销
│  ├─ RegisterGameEditorStyle()
│  ├─ RegisterMenus()
│  ├─ RegisterSettings()
│  └─ ...
└─ IMPLEMENT_MODULE(FLyraEditorModule, LyraEditor)
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraEditor.cpp` | ❌ 只用到引擎的 `FDefaultModuleImpl` | ⚠️ 需 `LyraEditor.Build.cs` 存在 | 编译验收放到 S05 末 |

**关键依赖表**

| 依赖 | 作用 | 源码位置 |
|---|---|---|
| `Modules/ModuleManager.h` | 提供 `IMPLEMENT_MODULE` 宏与 `FDefaultModuleImpl` | `Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h` |
| `FDefaultModuleImpl` | 引擎提供的空模块实现 | `ModuleManager.h:760-761` |
| `IMPLEMENT_MODULE` | 注册普通模块的宏 | `ModuleManager.h`（按分支多处定义） |

> 📌 **为什么 include 能写成 `"Modules/ModuleManager.h"`**：
> `LyraEditor.Build.cs` 的公共依赖里有 `Core` → UBT 把 `Core\Public\` 加进 include 搜索路径 → `Modules\` 是它下面的子目录。
> 详见答疑 `答疑文件\Modules是哪来的.md`。

---

## 四、与 Lyra 正式版的差距（S22 换）

| 项 | 占位版 | Lyra 正式版 |
|---|---|---|
| 模块实现类 | `FDefaultModuleImpl`（引擎空实现） | `FLyraEditorModule`（自定义） |
| 行数 / 字节 | 9 行 / 393 B | **329 行 / 10470 B** |
| `StartupModule` | ❌ 没有（用基类空实现） | ✅ 有，注册一大堆编辑器功能 |
| `ShutdownModule` | ❌ 没有 | ✅ 有 |
| 依赖的头文件 | 1 个 | 30+ 个 |

**Lyra 正式版那 329 行干了什么**（S22 会逐个讲）：

```
① RegisterGameEditorStyle()        → 注册编辑器自定义样式（按钮图标等）
② RegisterMenus()                  → 在编辑器菜单栏加 Lyra 相关菜单
③ RegisterSettings()               → 把 ULyraEditorSettings 挂到项目设置面板
④ RegisterGameplayCueNotifyClasses() → GameplayCue 编辑器支持
⑤ RegisterGameFeatureEditor()      → GameFeature 编辑器扩展
⑥ RegisterAssetValidators()        → 资产校验钩子（EditorValidator_*）
⑦ 注册 FLyraEditorEngine 替换默认编辑器引擎
```

> 📌 **S22 会给你完整 10470 B 正式版的复制文件 md**，到那时用那份覆盖本文件即可。

### Lyra 正式版还有 3 个我们没建的文件

```
E:\ue5\LyraStarterGame\Source\LyraEditor\（5 个文件）
├─ LyraEditor.Build.cs      1385 B   ← 我们占了位（06 轮）
├─ LyraEditor.cpp          10470 B   ← 我们占了位（本轮 07）
├─ LyraEditor.h              157 B   ← ❌ 没有（声明 LogLyraEditor 日志类别）
├─ LyraEditorEngine.cpp     2597 B   ← ❌ 没有（FLyraEditorEngine 编辑器引擎子类）
└─ LyraEditorEngine.h        851 B   ← ❌ 没有
```

> 这 3 个文件在 **S22** 一起建。

---

## 五、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `07_LyraEditor.cpp复制文件.md`**。
2. 你新建文件 `Source\LyraEditor\LyraEditor.cpp`，注释行不抄。
3. 敲完说「下一步」→ 剔注释逐字比对（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 08。
4. **本轮敲完仍然不能编译** —— 原因同第 06 轮（`LyraGame` 的插件依赖要到 S05）。

---

## 六、预期现象（不是错误）

| 现象 | 原因 | 什么时候好 |
|---|---|---|
| UBT 报 `Could not find definition for module 'ModularGameplayActors'` | `LyraGame.Build.cs` 的插件依赖 S01 才建 | S05 末 |
| IDE 全篇标红 | UBT 生成不了 makefile | S05 末 |
| Rider 里 `LyraEditor` 模块里只有 1 个符号 | 占位版就是只有 `IMPLEMENT_MODULE` 一行 | S22 正式版 |

---

**下一步**：第 08 轮 `Source\LyraGame\LyraLogChannels.h`（14 行，4 个日志频道声明）。