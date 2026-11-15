# 03 — `UnrealGame.Target.cs` 完整代码（一比一还原）

> **对应真实文件**：`D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealGame.Target.cs`（实读 **17 行**）。
> **本轮只展开这一个文件**；两张施工单到此讲完，下一课回 `Source\` 分区清单，等你定。
> **交付口径**：代码一字不改、零改名；注释中文化；类与构造函数上方补中文注释；代码块零行内教学提示。

---

## 一、本文件要解决什么（先讲为什么）

上一课的编辑器单告诉 UBT「编编辑器、全模块」。这一张说的则是「**编游戏**」：目标类型换成 `Game`，并且**声明这个目标支持所有平台类别**（`[SupportedPlatforms(UnrealPlatformClass.All)]`），但**不需要**把全部模块编进来。

**使用场景**：打包游戏、编译 `UnrealGame` 目标时，UBT 按目标名找到 `UnrealGame.Target.cs`，读它决定「目标类型 = Game、共享构建环境、额外带上 `UnrealGame` 模块」。它同样不写任何游戏逻辑，只拨开关。

> **记忆点（考点）**：两张单子**只有 4 处不同**（含一个空行），其余全一样；它是 C#、17 行、继承 `TargetRules`。

---

## 二、对应 UE5 真实源码（只给路径，不整段贴原文）

| 项 | 值 |
|---|---|
| 真实路径 | `D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealGame.Target.cs` |
| 真实行数 | 17 行（含 1 行版权注释、2 行 `using`、1 个平台特性、1 个类、1 个构造函数、4 行设置、**1 个空行**） |
| 名字 | **完全不改**，md 名沿用真实文件名 `UnrealGame.Target.cs` |
| 唯一权威 | 引擎真实源码（本路线不做前缀替换，没有「Lyra 原文」这一环节） |

**交付版与原文的差别只有 3 处，全部是注释**（代码行一字不动，连 Tab 缩进、`( TargetInfo Target )` 里的空格、以及 `BuildEnvironment` 后面那个**空行**都照抄）：

1. 第 1 行版权注释译成中文；
2. 类声明块（平台特性 + 类）上方**补** 1 行中文注释；
3. 构造函数上方**补** 1 行中文注释。

> ⚠️ **最容易抄错的一处**：原文第 14 行是**空行**（`BuildEnvironment = ...;` 与 `ExtraModuleNames.Add(...)` 之间），编辑器单里没有这个空行。一比一复刻不许因为「看起来多余」就删掉它。

---

## 三、完整代码（已中文化，可直接照抄）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。

using UnrealBuildTool;
using System.Collections.Generic;

// 游戏构建目标：告诉 UBT 这次编的是游戏，并声明支持所有平台类别
[SupportedPlatforms(UnrealPlatformClass.All)]
public class UnrealGameTarget : TargetRules
{
	// 构造函数：把当前编译目标（平台 / 配置等信息）交给基类 TargetRules
	public UnrealGameTarget( TargetInfo Target ) : base(Target)
	{
		Type = TargetType.Game;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		BuildEnvironment = TargetBuildEnvironment.Shared;

		ExtraModuleNames.Add("UnrealGame");
	}
}
```

> ✅ **交付前逐行扫描（铁律 45，硬动作已执行）**
>
> | 检查项 | 结果 |
> |---|---|
> | 代码块里还有英文注释吗 | ✅ 无，版权头已中文化 |
> | 类声明块（特性 + 类）上方有中文注释吗 | ✅ 有 |
> | 构造函数（唯一方法）上方有中文注释吗 | ✅ 有 |
> | 有没有空 `/**  */` 占位 / 只写表格不带注释 | ✅ 无 |
> | 有没有 `// ← 改成XXX` 行内教学提示 | ✅ 无 |
> | 有没有残留前缀 / 擅自改名 | ✅ 无，全是 UE5 原名 |
> | 17 行代码逐字一致吗 | ✅ 仅多 2 行中文注释，**第 14 行空行也照抄** |
> | 照抄能否直接用 | ✅ 是干净的最终版 |

---

## 四、逐段讲解（回扣「为什么这么写」）

### 两张单子对照图（箭头：`──差异──>` 指出不同的地方）

```
                  UnrealEditor.Target.cs          UnrealGame.Target.cs
版权头             // Copyright …        ──同名──> // Copyright …
using              2 行                  ──同名──> 2 行
平台特性           无                    ──差异──> [SupportedPlatforms(UnrealPlatformClass.All)]
类名               UnrealEditorTarget    ──差异──> UnrealGameTarget
构造函数           UnrealEditorTarget(…) ──差异──> UnrealGameTarget(…)
Type               TargetType.Editor     ──差异──> TargetType.Game
IncludeOrderVersion Latest               ──同名──> Latest
BuildEnvironment   Shared                ──同名──> Shared
全模块             bBuildAllModules=true ──差异──> 无
空行               无                    ──差异──> 有（BuildEnvironment 之后）
ExtraModuleNames   Add("UnrealGame")     ──同名──> Add("UnrealGame")
```

| 原文行 | 代码 | 为什么这么写（大白话） |
|---|---|---|
| 1 | `// Copyright …` | 版权声明，按口径译成中文 |
| 3 | `using UnrealBuildTool;` | `TargetRules`、`TargetType`、`SupportedPlatforms` 都来自 UBT 程序集 |
| 4 | `using System.Collections.Generic;` | 引擎原样保留，**没用到也不许删**（一比一复刻不许自己优化） |
| 6 | `[SupportedPlatforms(UnrealPlatformClass.All)]` | 声明该目标支持所有平台类别；特性类在 `SupportedPlatformsAttribute.cs:12`，枚举在 `UEBuildTarget.cs:1039` |
| 7 | `public class UnrealGameTarget : TargetRules` | 类名 = 目标名，UBT 按它找单子；继承 `TargetRules.cs:528` |
| 9 | `public UnrealGameTarget( TargetInfo Target ) : base(Target)` | 把「当前编哪个平台 / 哪种配置」交给基类，后面才能改基类字段 |
| 11 | `Type = TargetType.Game;` | **最核心一行**：这次编的是游戏（`TargetType` 在 `TargetRules.cs:20`） |
| 12 | `IncludeOrderVersion = EngineIncludeOrderVersion.Latest;` | 用 UE5.6 最新 include 顺序规则（枚举在 `TargetRules.cs:168`） |
| 13 | `BuildEnvironment = TargetBuildEnvironment.Shared;` | 共享构建环境，引擎模块可复用（枚举在 `TargetRules.cs:77`） |
| 14 | **（空行）** | 引擎原样，一比一照抄；编辑器单没有这行 |
| 15 | `ExtraModuleNames.Add("UnrealGame");` | 额外带上 `UnrealGame` 模块；字段在 `TargetRules.cs:2716` |

### 常见疑问，正面回答

- **「游戏单为什么多一个 `[SupportedPlatforms]`？」** 编辑器只在开发机上跑，游戏包要面向各类平台；这个特性让 UBT 知道该目标可用于所有平台类别。它标在类上、由 UBT 读取，不是 C++ 宏。
- **「游戏单为什么没有 `bBuildAllModules = true`？」** 编辑器要把所有编辑器工具模块都编进来；游戏只编真正用到的模块，省时间省体积。
- **「那个空行能不能删？」** **不能**。它是引擎真实文件的一部分，删了就和 UE5 对不上号；一比一复刻看的是原文长什么样，不是「能不能更整齐」。
- **「两张单子都 `Add("UnrealGame")`，是不是写重了？」** 不是。编辑器里 PIE 也要跑游戏侧模块，两个目标都需要它；这是引擎原样。

---

## 五、教学节奏与下一步

1. **本轮只交付这一个文件的完整代码**，停下等你说「懂了 / 下一步」。
2. ✅ 两张施工单（`UnrealEditor.Target.cs` 16 行 + `UnrealGame.Target.cs` 17 行）到此讲完。
3. **⏳ 下一步**：回 `00_Source_总md.md` 的 5 分区清单，等你定先攻哪个（`Runtime\` 建议 / `Editor\` / `Developer\` / `Programs\` / `ThirdParty\`）；`Engine\` 下其余 13 个目录仍全在清单里、一个没删。
4. 每教一步同步更新 `01_两个Target文件_总md.md`、`00_Source_总md.md`、`00_Engine_总md.md` 与根总纲的进度列。
5. **下次「下一步」先验收还原文件**：本课的还原目标为 `ue_5.6\Engine\Source\UnrealGame.Target.cs`，全对才推进，有错不推进。
6. 讲义必须落成 md，不能只发聊天。

---

## 六、速查表 · 口诀 · 一句话白话总结

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\UnrealGame.Target.cs`（17 行，C#） |
| 建到哪 | `从零复刻ue5\Engine\Source\03_UnrealGame.Target.cs_完整代码.md`（本文件） |
| 还原到哪 | `ue_5.6\Engine\Source\UnrealGame.Target.cs`（下一步验收对象） |

**使用场景速记**：打包游戏 / 编译 `UnrealGame` → 读这张单；换平台配置 → `[SupportedPlatforms]` 决定可不可编；以后加游戏模块 → 往 `ExtraModuleNames` 里 `Add`。

**口诀**：**两张单只差四处 —— 平台特性、类名、目标类型、空行；编辑器多「全编」，游戏多「全平台」；其余字段、两个 `using`、末尾 `Add` 全一样；空行也照抄。**

**一句话白话总结**：这张 17 行纸条就是告诉施工队「这次盖游戏、全平台能跑、别把编辑器工具全编进来」，抄的时候连中间那个空格行都别动，只把注释说成中国话。
