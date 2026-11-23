# 06 — `Source/LyraEditor/LyraEditor.Build.cs` —— 总 md

> 依据：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs`（**现在 891 B / 28 行，占位版**）
> 对照源：`E:\ue5\LyraStarterGame\Source\LyraEditor\LyraEditor.Build.cs`（**1385 B，正式版，S22 才教**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs`
> **⚠️ 本节不能独立编译** —— 它依赖 `LyraGame` 模块（第 04 轮的 `Build.cs`），而 `LyraGame` 又要等 S01~S05 插件就位。

---

## 一、这个文件解决什么问题（为什么）

### 场景：为什么现在就要有这个文件

第 03 轮你敲了 `Source\LyraEditor.Target.cs`，它第 12 行写着：

```csharp
ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });
```

UBT 读到这一行，就认定**工程里存在一个叫 `LyraEditor` 的模块**，然后去 `Source\LyraEditor\` 找 `LyraEditor.Build.cs`。

找不到会怎样：

| 后果 | 具体现象 |
|---|---|
| ① UBT 直接失败 | `Could not find definition for module 'LyraEditor'` |
| ② IDE 全篇标红 | UBT 生成不了 makefile → Rider / VS 拿不到引擎头文件路径 → 连 `#include "CoreMinimal.h"` 都报红 |

🔑 **一句话比喻**：`Target.cs` 是「花名册」，上面写了 `LyraEditor` 这个名字；`Build.cs` 是这个人的「档案」。花名册上有名字、档案找不到 → 整个流程卡住。

### 那为什么 S00 只放「占位版」不放正式版

Lyra 正式版的 `LyraEditor.Build.cs` 列了 **13 个公共依赖 + 12 个私有依赖**，其中一半以上依赖的类（`FLyraEditorEngine`、`GameEditorStyle`、`EditorValidator`…）要到 **S22** 才复刻。

现在照着正式版写 → UBT 找不到那些类 → 报一批新错误。

所以 S00 的策略是：**放一个「最小可解析」的 Build.cs，只列必需的 5 个依赖，让 UBT 能过；S22 再整份替换成 Lyra 正式版。**

### 核心设计三点

1. **类名 = 文件名前缀**：`public class LyraEditor` 对应 `LyraEditor.Build.cs`。UBT 靠这个匹配，写错直接报「找不到类」。
2. **只列 5 个依赖**：`Core` / `CoreUObject` / `Engine` / `UnrealEd` / `LyraGame` —— 够让 UBT 解析，不多不少。
3. **`PCHUsage` 必须写**：`UseExplicitOrSharedPCHs` 是 UE5 推荐值，不写会走 UE4 的旧 PCH 行为。

---

## 二、内部结构全景

```
LyraEditor.Build.cs（28 行 / 891 B）
├─ 版权 + 占位说明注释（1-8）          ← 中文说明为什么是占位
├─ using UnrealBuildTool;（10）
├─ public class LyraEditor : ModuleRules（12-28）
│  └─ LyraEditor(ReadOnlyTargetRules)（14-27）  构造函数
│     ├─ PCHUsage（16）
│     └─ PublicDependencyModuleNames.AddRange（18-26）
│        ├─ "Core"          （20）
│        ├─ "CoreUObject"   （21）
│        ├─ "Engine"        （22）
│        ├─ "UnrealEd"      （23）
│        └─ "LyraGame"      （24）★ 关键：编辑器模块要引用游戏模块的类
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|---|---|---|---|
| `LyraEditor.Build.cs` | ✅ 依赖 `LyraGame` 模块 | ⚠️ 不能 —— `LyraGame` 要等 S01~S05 插件 | 编译验收放到 S05 末 |

**关键依赖表**

| 依赖 | 作用 | 谁提供 |
|---|---|---|
| `Core` | `FString` / `TArray` / 容器 / 数学库 | 引擎 |
| `CoreUObject` | `UCLASS` / `UPROPERTY` / 反射 / GC | 引擎 |
| `Engine` | `AActor` / `UWorld` / 渲染 | 引擎 |
| `UnrealEd` | 编辑器基础设施（`FEdMode` / 编辑器窗口基类） | 引擎 |
| `LyraGame` | 主游戏模块（编辑器要操作 Lyra 的类） | **第 04 轮的 `Build.cs`** |

> ⚠️ **`LyraEditor` 是编辑器模块**，只在编 `TargetType.Editor` 时参与编译；打包游戏时不会带上它。

---

## 四、与 Lyra 正式版的差距（S22 换）

| 项 | 我们的占位版 | Lyra 正式版 | 差异 |
|---|---|---|---|
| `PublicIncludePaths` | ❌ 无 | ✅ `"LyraEditor"` | 头文件搜索路径 |
| `PrivateIncludePaths` | ❌ 无 | ✅ 空数组 | — |
| `PublicDependencyModuleNames` | **5 个** | **13 个** | 缺 `EditorFramework` / `PhysicsCore` / `GameplayTagsEditor` / `GameplayTasksEditor` / `GameplayAbilities` / `GameplayAbilitiesEditor` / `StudioTelemetry` |
| `PrivateDependencyModuleNames` | ❌ 无 | **12 个** | 缺 `InputCore` / `Slate` / `SlateCore` / `ToolMenus` / `EditorStyle` / `DataValidation` / `MessageLog` / `Projects` / `DeveloperToolSettings` / `CollectionManager` / `SourceControl` / `Chaos` |
| `DynamicallyLoadedModuleNames` | ❌ 无 | ✅ 空数组 | — |
| `PublicDefinitions` | ❌ 无 | ✅ `SHIPPING_DRAW_DEBUG_ERROR=1` | 发行版禁 DrawDebug |

> 📌 **S22 会给你完整 1385 B 正式版的复制文件 md**，到那时直接用那份覆盖本文件即可。

---

## 五、怎么教（推进节奏）

1. 本轮只教这一个文件：**两件套 = 本总 md + `06_LyraEditor.Build.cs复制文件.md`**。
2. 你新建文件 `Source\LyraEditor\LyraEditor.Build.cs`，注释行不抄（复制文件里有逐行中文注释，敲的时候整行 `//` 不要）。
3. 敲完说「下一步」→ 剔注释逐字比对（`codeOurs == codeEngine` 且 `diff == 0`）→ 通过才发 07。
4. **本轮敲完仍然不能编译** —— `LyraGame` 模块的插件依赖要等 S01~S05。这是设计好的，不是你的错。

---

## 六、预期现象（不是错误）

| 现象 | 原因 | 什么时候好 |
|---|---|---|
| UBT 报 `Could not find definition for module 'ModularGameplayActors'` | `LyraGame.Build.cs` 依赖的插件模块 S01 才建 | S05 末 |
| IDE 里 `CoreMinimal.h` 等引擎头文件全红 | UBT 生成不了 makefile → IntelliSense 拿不到路径 | S05 末 |
| `LyraEditor` 模块在 Rider 里显示为空模块 | 正常 —— 占位 `.cpp` 只有一行 `IMPLEMENT_MODULE` | S22 正式版 |

---

**下一步**：第 07 轮 `Source\LyraEditor\LyraEditor.cpp`（9 行 / 393 B，占位模块实现）。