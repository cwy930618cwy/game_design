# 02 — `UnrealEditor.Target.cs` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealEditor.Target.cs`（**16 行**）
> 你新建文件：`ue_5.6\Engine\Source\UnrealEditor.Target.cs`
> **✅ 本节可独立编译**——C# 构建脚本，由 UBT 自己编译，不参与 C++ 编译。
> **注意**：这份是**讲解版**（每行都有详细注释），交付版在 `02_UnrealEditor.Target.cs_完整代码.md`。

---

## 一、这个文件解决什么问题（为什么）

UBT（编译施工队）开工前必须先看到一张单子，写明「这次要编的是**编辑器**」。这张单子就是答案：它不写任何游戏逻辑，只在构造函数里拨几个开关。

**说个场景**：你在 VS 里编译 `UnrealEditor` 目标时，UBT 按目标名找到 `UnrealEditor.Target.cs`、读它、再决定编哪些模块。没有这张单子，施工队不知道要盖什么。

> 🔑 **一句话**：16 行 C# 纸条，写着「这次盖编辑器，全编」。

---

## 二、完整代码（每行都有详细注释）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 引入 UBT（UnrealBuildTool）程序集。
// 本文件用到的 TargetRules（基类）、TargetInfo（目标信息）、
// TargetType（目标类型枚举）、EngineIncludeOrderVersion（include 顺序枚举）、
// TargetBuildEnvironment（构建环境枚举）全部来自这个程序集。
using UnrealBuildTool;

// 引入 C# 泛型容器命名空间。
// 本文件其实没直接写 List<>，但引擎原样保留了它 —— 一比一复刻不许因为「看起来没用」就删。
using System.Collections.Generic;

// 声明编辑器构建目标类，继承 TargetRules。
// 类名必须等于目标名（UnrealEditorTarget），UBT 按类名找到这份单子。
// TargetRules 是所有 *.Target.cs 的基类，定义在 TargetRules.cs:528
// （public abstract partial class TargetRules）。
public class UnrealEditorTarget : TargetRules
{
	// 构造函数：参数 Target 是 TargetInfo（目标信息），装着「当前编哪个平台 / 哪种配置」。
	// 注意：参数名和类型之间、括号内侧都有空格（( TargetInfo Target )），引擎原文如此，照抄。
	// : base(Target) 把 Target 交给基类 TargetRules 保存，后面才能改基类的字段。
	// TargetInfo 类定义在 TargetInfo.cs:12。
	public UnrealEditorTarget( TargetInfo Target ) : base(Target)
	{
		// 声明「目标是编辑器」。
		// Type 属性定义在 TargetRules.cs:681，默认值是 TargetType.Game。
		// TargetType 枚举定义在 TargetRules.cs:20，5 个值：
		// Game / Editor / Client / Server / Program。
		// 这一行是整个文件最核心的一行 —— 它决定了这次编的是 UnrealEditor.exe。
		Type = TargetType.Editor;

		// 设置 include 顺序版本为「最新」。
		// IncludeOrderVersion 属性定义在 TargetRules.cs:703。
		// EngineIncludeOrderVersion 枚举定义在 TargetRules.cs:168，
		// 值有 Unreal5_1 ~ Unreal5_6 / Latest / Oldest。
		// Latest = Unreal5_6（TargetRules.cs:209）。
		// 为什么用 Latest：避免老式「隐式包含」依赖，强制每个文件自己 include 需要的头。
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// 设置构建环境为「共享」。
		// BuildEnvironment 属性定义在 TargetRules.cs:2740。
		// TargetBuildEnvironment 枚举定义在 TargetRules.cs:77，3 个值：
		// Shared / Unique / UniqueIfNeeded。
		// Shared = 引擎二进制和中间产物输出到引擎目录，多个目标共用（省磁盘、省编译时间）。
		// Unique = 每个目标单独一套（改引擎设置时用）。
		BuildEnvironment = TargetBuildEnvironment.Shared;

		// 声明「编译所有模块」。
		// bBuildAllModules 属性定义在 TargetRules.cs:788，bool，默认 false。
		// 为什么编辑器要 true：编辑器要用到各种工具模块（材质编辑器、蓝图编辑器、
		// 关卡编辑器…），必须全编进来；游戏目标不需要，所以游戏单里没有这行。
		bBuildAllModules = true;

		// 往「额外模块清单」里加 UnrealGame 模块。
		// ExtraModuleNames 属性定义在 TargetRules.cs:2716，
		// 类型 List<string>，初始值 new()（空列表）。
		// 为什么编辑器也要加 UnrealGame：编辑器要加载游戏侧的运行时模块，
		// 才能在 PIE（Play In Editor）里跑游戏内容。
		ExtraModuleNames.Add("UnrealGame");
	}
}
```

---

## 三、逐行拆解（本段新元素 + 这段在干什么）

### 3.1 开头与类声明（第 1~8 行）

**本段新元素**

- `using UnrealBuildTool` —— 引入 UBT 程序集，`TargetRules` / `TargetInfo` / `TargetType` 都在里面
- `using System.Collections.Generic` —— 引入泛型容器（本文件没直接用，引擎原样保留）
- `TargetRules` —— **目标规则基类**，所有 `*.Target.cs` 都继承它（`TargetRules.cs:528`）
- `TargetInfo` —— **目标信息**，装着平台 / 配置等（`TargetInfo.cs:12`）
- `base(Target)` —— 调基类构造函数，把 `Target` 存进 `TargetRules`

**这段在干什么**：声明 `UnrealEditorTarget` 类继承 `TargetRules`，构造函数接收 `TargetInfo` 并交给基类。

> 类比：`Target.cs` 是「今天盖什么」，`Build.cs` 是「这间房怎么盖」——同一个包工头（UBT），两张不同的单子。

### 3.2 五个设置（第 10~14 行）

**本段新元素**

- `TargetType` —— 目标类型枚举，5 个值 Game/Editor/Client/Server/Program（`TargetRules.cs:20`）
- `TargetType.Editor` —— 编辑器枚举值（`TargetRules.cs:30`）
- `Type` —— 目标类型属性，默认 `TargetType.Game`（`TargetRules.cs:681`）
- `EngineIncludeOrderVersion` —— include 顺序版本枚举（`TargetRules.cs:168`）
- `EngineIncludeOrderVersion.Latest` —— 最新版本，= `Unreal5_6`（`TargetRules.cs:209`）
- `IncludeOrderVersion` —— include 顺序属性（`TargetRules.cs:703`）
- `TargetBuildEnvironment` —— 构建环境枚举，3 个值 Shared/Unique/UniqueIfNeeded（`TargetRules.cs:77`）
- `TargetBuildEnvironment.Shared` —— 共享构建环境（`TargetRules.cs:82`）
- `BuildEnvironment` —— 构建环境属性（`TargetRules.cs:2740`）
- `bBuildAllModules` —— 是否编所有模块，默认 false（`TargetRules.cs:788`）
- `ExtraModuleNames` —— 额外模块清单，`List<string>`（`TargetRules.cs:2716`）

**这段在干什么**

| 行 | 代码 | 干什么 |
|---|---|---|
| 10 | `Type = TargetType.Editor;` | **最核心**：声明目标是编辑器 |
| 11 | `IncludeOrderVersion = EngineIncludeOrderVersion.Latest;` | 用 UE5.6 最新 include 顺序 |
| 12 | `BuildEnvironment = TargetBuildEnvironment.Shared;` | 共享构建环境，多目标复用 |
| 13 | `bBuildAllModules = true;` | 编辑器要编所有模块 |
| 14 | `ExtraModuleNames.Add("UnrealGame");` | 额外编 UnrealGame 模块（PIE 要用） |

> 类比：这五行是「施工单上的五个勾选框」——盖什么（Editor）、按哪版规范（Latest）、材料放哪（Shared）、全做还是部分做（true）、额外加什么（UnrealGame）。

---

## 四、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `UnrealEditor.Target.cs` | `UnrealEditor.Target.cs` | **完全一致** |
| 路径 | `Engine\Source\` | `ue_5.6\Engine\Source\` | 只换根 |
| 类名 | `UnrealEditorTarget` | `UnrealEditorTarget` | 完全一致 |
| 代码行 | 16 行 | 16 行 | **行数一致**（注释原地中文化 + 补注释） |
| 英文注释 | 1 处（版权头） | 0 处 | 译成中文 |
| 补注释 | 无 | 类上方 + 构造函数上方 | 允许的例外 |
| 特殊空格 | `( TargetInfo Target )` | 原样保留 | 括号内侧有空格，照抄 |

> 除注释外，**代码一字不差**。

---

## 五、收尾

到这里 `UnrealEditor.Target.cs` 就完了（16 行）。

**结构**：一个 `TargetRules` 子类，构造函数里 5 个设置——`Type` / `IncludeOrderVersion` / `BuildEnvironment` / `bBuildAllModules` / `ExtraModuleNames`。

**关键概念**：
- `Type = TargetType.Editor` 决定编的是编辑器
- `bBuildAllModules = true` 是编辑器专属（游戏单没有）
- `ExtraModuleNames.Add("UnrealGame")` 两张单都有（PIE 要用）

---

## 六、速查表 · 口诀

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\UnrealEditor.Target.cs`（16 行，C#） |
| 讲义 | `从零复刻ue5\Engine\Source\02_UnrealEditor.Target.cs_完整代码.md` |
| 还原到哪 | `ue_5.6\Engine\Source\UnrealEditor.Target.cs` |

**口诀**：**一张单、十六行；`Type=Editor` 定方向，全模块 `true` 是编辑器专属；`Latest` 顺序、`Shared` 环境、`Add("UnrealGame")` 两张单都一样；注释改中文，代码一个字不动。**

---

## 待确认

1. 16 行看懂了吗？重点记 `Type = TargetType.Editor` 和 `bBuildAllModules = true`。
2. 看完说「下一步」，我验收还原文件，通过后教 `UnrealGame.Target.cs`（17 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealEditor.Target.cs`（16 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs`（`TargetType` 20、`TargetBuildEnvironment` 77、`EngineIncludeOrderVersion` 168、`Latest` 209、`TargetRules` 类 528、`Type` 681、`IncludeOrderVersion` 703、`bBuildAllModules` 788、`ExtraModuleNames` 2716、`BuildEnvironment` 2740）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetInfo.cs`（`TargetInfo` 类 12）