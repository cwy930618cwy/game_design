# 02 — `UnrealEditor.Target.cs` 完整代码（一比一还原）

> **对应真实文件**：`D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealEditor.Target.cs`（实读 **16 行**）。
> **本轮只展开这一个文件**；`UnrealGame.Target.cs` 等这份确认后再单独展开（不合并）。
> **交付口径**：代码一字不改、零改名；注释中文化；类与构造函数上方补中文注释；代码块零行内教学提示。

---

## 一、本文件要解决什么（先讲为什么）

UBT（编译施工队）开工前必须先看到一张单子，写明「这次要编的是**编辑器**」。这张单子就是答案：它不写任何游戏逻辑，只在构造函数里拨几个开关，让 UBT 知道「目标类型 = Editor、所有模块都编、用最新 include 顺序」。

**使用场景**：你在 VS 里编译 `UnrealEditor` 目标（或重新生成工程文件）时，UBT 就是按目标名找到 `UnrealEditor.Target.cs`、读它、再决定编哪些模块。没有这张单子，施工队不知道要盖什么。

**为什么单独一份 md**：一比一复刻要求**一个真实文件对应一份讲义**，不合并、不拆；上一课 `01_两个Target文件_总md.md` 是两张单子的合讲（全景），这一课才是第一张单子的完整内容。

> **记忆点（考点）**：它是 **C# 文件**，16 行，继承 `TargetRules`；`Source\` 顶层只有它和 `UnrealGame.Target.cs` 两个散文件。

---

## 二、对应 UE5 真实源码（只给路径，不整段贴原文）

| 项 | 值 |
|---|---|
| 真实路径 | `D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealEditor.Target.cs` |
| 真实行数 | 16 行（含 1 行版权注释、2 行 `using`、1 行空行、1 个类、1 个构造函数、5 行设置） |
| 名字 | **完全不改**，md 名也沿用真实文件名 `UnrealEditor.Target.cs` |
| 唯一权威 | 引擎真实源码（本路线不做前缀替换，没有「Lyra 原文」这一环节） |

**交付版与原文的差别只有 3 处，全部是注释**（代码行一字不动，连 Tab 缩进、`( TargetInfo Target )` 里的空格都照抄）：

1. 第 1 行版权注释 `// Copyright Epic Games, Inc. All Rights Reserved.` → 译成中文；
2. 类声明上方**补** 1 行中文注释（原文此处没有注释）；
3. 构造函数上方**补** 1 行中文注释（原文此处没有注释）。

> ⚠️ 补注释是「只不改名，注释仍中文」口径允许的例外，**不是加代码逻辑**；如果你要保留版权声明英文原文，说一声即可换回。

---

## 三、完整代码（已中文化，可直接照抄）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。

using UnrealBuildTool;
using System.Collections.Generic;

// 编辑器构建目标：告诉 UBT 这次编的是编辑器，并把所有模块都编进来
public class UnrealEditorTarget : TargetRules
{
	// 构造函数：把当前编译目标（平台 / 配置等信息）交给基类 TargetRules
	public UnrealEditorTarget( TargetInfo Target ) : base(Target)
	{
		Type = TargetType.Editor;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		BuildEnvironment = TargetBuildEnvironment.Shared;
		bBuildAllModules = true;
		ExtraModuleNames.Add("UnrealGame");
	}
}
```

> ✅ **交付前逐行扫描（铁律 45，硬动作已执行）**
>
> | 检查项 | 结果 |
> |---|---|
> | 代码块里还有英文注释吗 | ✅ 无，版权头已中文化 |
> | 类声明上方有中文注释吗 | ✅ 有 |
> | 构造函数（唯一方法）上方有中文注释吗 | ✅ 有 |
> | 有没有空 `/**  */` 占位 / 只写表格不带注释 | ✅ 无 |
> | 有没有 `// ← 改成XXX` 行内教学提示 | ✅ 无 |
> | 有没有残留前缀 / 擅自改名 | ✅ 无，全是 UE5 原名 |
> | 16 行代码逐字一致吗 | ✅ 仅多 2 行中文注释，代码行零改动 |
> | 照抄能否直接用 | ✅ 是干净的最终版 |

---

## 四、逐段讲解（回扣「为什么这么写」）

| 原文行 | 代码 | 为什么这么写（大白话） |
|---|---|---|
| 1 | `// Copyright …` | 版权声明，引擎每个源文件都有；按口径译成中文 |
| 3 | `using UnrealBuildTool;` | `TargetRules`、`TargetType` 这些类型都来自 UBT 程序集，不引入就找不到基类 |
| 4 | `using System.Collections.Generic;` | 引入 C# 泛型容器。**本文件其实没直接写 `List<>`，但引擎原样保留了它** —— 一比一复刻不许因为「看起来没用」就删（铁律 23/25） |
| 6 | `public class UnrealEditorTarget : TargetRules` | 类名 = 目标名，UBT 按这个名字找单子；继承 `TargetRules`（`TargetRules.cs:528`）才能拿到下面那些字段 |
| 8 | `public UnrealEditorTarget( TargetInfo Target ) : base(Target)` | `TargetInfo` 装着「当前编哪个平台 / 哪种配置」；`: base(Target)` 必须先把它交给基类，后面才能改基类的字段 |
| 10 | `Type = TargetType.Editor;` | **最核心一行**：声明目标是编辑器（`TargetType` 枚举在 `TargetRules.cs:20`） |
| 11 | `IncludeOrderVersion = EngineIncludeOrderVersion.Latest;` | 用 UE5.6 最新的 include 顺序规则，避免老式「隐式包含」依赖（枚举在 `TargetRules.cs:168`） |
| 12 | `BuildEnvironment = TargetBuildEnvironment.Shared;` | 共享构建环境，引擎模块可在多个目标间复用（枚举在 `TargetRules.cs:77`） |
| 13 | `bBuildAllModules = true;` | 编辑器要把**所有模块**编进来（各种编辑器工具都要能用）；游戏目标不需要，所以游戏单里没有这行 |
| 14 | `ExtraModuleNames.Add("UnrealGame");` | 额外把 `UnrealGame` 模块编进来；字段定义在 `TargetRules.cs:2716`：`public List<string> ExtraModuleNames { get; } = new();` |

### 常见疑问，正面回答

- **「`using System.Collections.Generic;` 没用到，删了行不行？」** 不行。一比一复刻看的是引擎真实文件长什么样，不是「能不能少一行」；删了就和 UE5 对不上号了。
- **「编辑器单为什么还要 `Add("UnrealGame")`？」** 编辑器也要加载游戏侧的运行时模块，才能在 PIE 里跑游戏内容；这是引擎原样，两个目标都写。
- **「`bBuildAllModules = true` 会不会把第三方库也全编？」** 它控制的是 UBT 收集**引擎模块**的范围；具体编哪些还受平台、配置、模块规则约束。这里先大概讲，细节到 `Programs\UnrealBuildTool\` 展开时再深入（铁律 27：写全 → 大概讲 → 详细后置）。

---

## 五、教学节奏与下一步

1. **本轮只交付这一个文件的完整代码**，停下等你说「懂了 / 下一步」。
2. **⏳ 下一步**：展开 `UnrealGame.Target.cs` 完整 17 行，同样一比一贴全、注释中文化、逐行扫描；重点讲它比编辑器单**多** `[SupportedPlatforms(UnrealPlatformClass.All)]`、**少** `bBuildAllModules = true`。
3. 每教一步同步更新 `01_两个Target文件_总md.md`、`00_Source_总md.md`、`00_Engine_总md.md` 与根总纲的进度列。
4. 讲义必须落成 md，不能只发聊天；收到「下一步」先核对上一步产物真落盘了再推进。
5. ✅ 还原目录已由用户 2026-11-14 指定：`ue_5.6\`；每次「下一步」先按 `01_下一步前先检查还原文件.md` 验收还原文件，全对才推进、有错不推进。

---

## 六、速查表 · 口诀 · 一句话白话总结

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\UnrealEditor.Target.cs`（16 行，C#） |
| 建到哪 | `从零复刻ue5\Engine\Source\02_UnrealEditor.Target.cs_完整代码.md`（本文件） |
| 谁用它 | `Programs\UnrealBuildTool\`，基类 `TargetRules.cs:528` |

**使用场景速记**：编译 `UnrealEditor` → 读这张单；以后加自己的模块 → 往 `ExtraModuleNames` 里 `Add`；把 `Type` 改成 `Game` 就变成另一张单（但引擎是分成两个文件写的，我们不合并）。

**口诀**：**一张单、十六行；`Type=Editor` 定方向，全模块 `true` 是编辑器专属；`Latest` 顺序、`Shared` 环境、`Add("UnrealGame")` 两张单都一样；注释改中文，代码一个字不动。**

**一句话白话总结**：这份文件就是给编译施工队的一张 16 行纸条，上面写着「这次盖编辑器，全编」，抄的时候连空格缩进都照抄，只把注释说成中国话、并在类和方法头顶补两行中文说明。
