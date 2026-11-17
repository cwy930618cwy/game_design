# 03 — `UnrealGame.Target.cs` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealGame.Target.cs`（**17 行**）
> 你新建文件：`ue_5.6\Engine\Source\UnrealGame.Target.cs`
> **✅ 本节可独立编译**——C# 构建脚本，由 UBT 自己编译，不参与 C++ 编译。
> **注意**：这份是**讲解版**（每行都有详细注释），交付版在 `03_UnrealGame.Target.cs_完整代码.md`。

---

## 一、这个文件解决什么问题（为什么）

上一课 `UnrealEditor.Target.cs` 是「盖编辑器」的单子。这一份是「**盖游戏**」的单子——`Type = TargetType.Game`，并且**声明支持所有平台类别**。

**说个场景**：你打包一个独立游戏（`UnrealGame.exe`）时，UBT 按目标名找到 `UnrealGame.Target.cs`、读它、再决定编哪些模块。它和编辑器单**只差 3 处**：`Type` 值、多一个 `[SupportedPlatforms]` 特性、少一行 `bBuildAllModules`。

> 🔑 **一句话**：17 行 C# 纸条，写着「这次盖游戏，所有平台都能盖」。

---

## 二、完整代码（每行都有详细注释）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。
// 版权声明，引擎每个源文件第一行都是它，交付时译成中文。

// 引入 UBT（UnrealBuildTool）程序集。
// 本文件用到的 TargetRules（基类）、TargetInfo（目标信息）、
// TargetType（目标类型枚举）、EngineIncludeOrderVersion（include 顺序枚举）、
// TargetBuildEnvironment（构建环境枚举）、SupportedPlatforms（平台特性）、
// UnrealPlatformClass（平台类别枚举）全部来自这个程序集。
using UnrealBuildTool;

// 引入 C# 泛型容器命名空间。
// 本文件其实没直接写 List<>，但引擎原样保留了它 —— 一比一复刻不许因为「看起来没用」就删。
using System.Collections.Generic;

// 特性（Attribute）：声明这个目标支持「所有平台类别」。
// SupportedPlatformsAttribute 定义在 SupportedPlatformsAttribute.cs:12，
// 构造重载 (UnrealPlatformClass category) 在 :42。
// UnrealPlatformClass 枚举定义在 UEBuildTarget.cs:1039，4 个值：
// All / Desktop / Editor / Server。
// All = 所有已注册平台（Utils.cs:513 → UnrealTargetPlatform.GetValidPlatforms）。
// 为什么游戏单要这个：游戏要能打包到任意平台（Win/Mac/Linux/Android/iOS…），
// 编辑器单不需要（编辑器只在桌面平台跑）。
[SupportedPlatforms(UnrealPlatformClass.All)]

// 声明游戏构建目标类，继承 TargetRules。
// 类名必须等于目标名（UnrealGameTarget），UBT 按类名找到这份单子。
// TargetRules 是所有 *.Target.cs 的基类，定义在 TargetRules.cs:528
// （public abstract partial class TargetRules）。
public class UnrealGameTarget : TargetRules
{
	// 构造函数：参数 Target 是 TargetInfo（目标信息），装着「当前编哪个平台 / 哪种配置」。
	// 注意：参数名和类型之间、括号内侧都有空格（( TargetInfo Target )），引擎原文如此，照抄。
	// : base(Target) 把 Target 交给基类 TargetRules 保存，后面才能改基类的字段。
	// TargetInfo 类定义在 TargetInfo.cs:12。
	public UnrealGameTarget( TargetInfo Target ) : base(Target)
	{
		// 声明「目标是游戏」。
		// Type 属性定义在 TargetRules.cs:681，默认值就是 TargetType.Game。
		// TargetType 枚举定义在 TargetRules.cs:20，5 个值：
		// Game / Editor / Client / Server / Program。
		// 和编辑器单的唯一区别：这里是 Game，编辑器单是 Editor。
		Type = TargetType.Game;

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

		// 注意：这里有一个空行（引擎原文如此，照抄）。
		// 编辑器单在这个位置是 bBuildAllModules = true;，游戏单没有这一行 ——
		// 因为游戏只需要游戏相关模块，不需要编辑器工具模块。

		// 往「额外模块清单」里加 UnrealGame 模块。
		// ExtraModuleNames 属性定义在 TargetRules.cs:2716，
		// 类型 List<string>，初始值 new()（空列表）。
		// 为什么游戏单也要加 UnrealGame：这是游戏本体的运行时模块，
		// 打包出来的 UnrealGame.exe 就是靠它跑起来的。
		ExtraModuleNames.Add("UnrealGame");
	}
}
```

---

## 三、逐段拆解（本段新元素 + 这段在干什么）

### 3.1 开头与特性（第 1~7 行）

**本段新元素**

- `using UnrealBuildTool` —— 引入 UBT 程序集
- `using System.Collections.Generic` —— 引入泛型容器（本文件没直接用，引擎原样保留）
- `SupportedPlatforms` —— **特性（Attribute）**，声明目标支持哪些平台（`SupportedPlatformsAttribute.cs:12`）
- `UnrealPlatformClass` —— 平台类别枚举，4 个值 All/Desktop/Editor/Server（`UEBuildTarget.cs:1039`）
- `UnrealPlatformClass.All` —— 所有已注册平台（`Utils.cs:513`）
- `TargetRules` —— 目标规则基类（`TargetRules.cs:528`）
- `TargetInfo` —— 目标信息（`TargetInfo.cs:12`）

**这段在干什么**：声明 `UnrealGameTarget` 类继承 `TargetRules`，并用 `[SupportedPlatforms(UnrealPlatformClass.All)]` 特性声明「支持所有平台」。

> 类比：`[SupportedPlatforms(...)]` 是贴在单子上的「适用范围标签」——写着「所有平台都能用这张单」。

### 3.2 四个设置（第 11~15 行）

**本段新元素**

- `TargetType.Game` —— 游戏枚举值（`TargetRules.cs:25`）
- `Type` —— 目标类型属性，默认 `TargetType.Game`（`TargetRules.cs:681`）
- `EngineIncludeOrderVersion.Latest` —— 最新 include 顺序（`TargetRules.cs:209`）
- `IncludeOrderVersion` —— include 顺序属性（`TargetRules.cs:703`）
- `TargetBuildEnvironment.Shared` —— 共享构建环境（`TargetRules.cs:82`）
- `BuildEnvironment` —— 构建环境属性（`TargetRules.cs:2740`）
- `ExtraModuleNames` —— 额外模块清单（`TargetRules.cs:2716`）

**这段在干什么**

| 行 | 代码 | 干什么 |
|---|---|---|
| 11 | `Type = TargetType.Game;` | **最核心**：声明目标是游戏 |
| 12 | `IncludeOrderVersion = EngineIncludeOrderVersion.Latest;` | 用 UE5.6 最新 include 顺序 |
| 13 | `BuildEnvironment = TargetBuildEnvironment.Shared;` | 共享构建环境 |
| 15 | `ExtraModuleNames.Add("UnrealGame");` | 加 UnrealGame 模块（游戏本体） |

> 注意第 14 行是**空行**——编辑器单在这个位置是 `bBuildAllModules = true;`，游戏单没有。

---

## 四、和 `UnrealEditor.Target.cs` 的差异（关键）

| 项 | `UnrealEditor.Target.cs` | `UnrealGame.Target.cs` |
|---|---|---|
| 行数 | 16 行 | **17 行** |
| 类名 | `UnrealEditorTarget` | `UnrealGameTarget` |
| `[SupportedPlatforms]` | ❌ 没有 | ✅ `[SupportedPlatforms(UnrealPlatformClass.All)]` |
| `Type` | `TargetType.Editor` | `TargetType.Game` |
| `IncludeOrderVersion` | `Latest` | `Latest`（相同） |
| `BuildEnvironment` | `Shared` | `Shared`（相同） |
| `bBuildAllModules` | ✅ `= true` | ❌ **没有** |
| `ExtraModuleNames` | `Add("UnrealGame")` | `Add("UnrealGame")`（相同） |

**三处差异**：
1. **多** `[SupportedPlatforms(UnrealPlatformClass.All)]`（游戏要能打包到任意平台）
2. `Type` 从 `Editor` 变 `Game`
3. **少** `bBuildAllModules = true`（游戏不需要编辑器工具模块）

---

## 五、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `UnrealGame.Target.cs` | `UnrealGame.Target.cs` | **完全一致** |
| 路径 | `Engine\Source\` | `ue_5.6\Engine\Source\` | 只换根 |
| 类名 | `UnrealGameTarget` | `UnrealGameTarget` | 完全一致 |
| 代码行 | 17 行 | 17 行 | **行数一致** |
| 英文注释 | 1 处（版权头） | 0 处 | 译成中文 |
| 补注释 | 无 | 类上方 + 构造函数上方 | 允许的例外 |
| 特殊空格 | `( TargetInfo Target )` | 原样保留 | 括号内侧有空格，照抄 |
| 第 14 行空行 | 有 | 原样保留 | 照抄 |

> 除注释外，**代码一字不差**。

---

## 六、收尾

到这里 `UnrealGame.Target.cs` 就完了（17 行）。

**结构**：一个 `TargetRules` 子类 + 一个 `[SupportedPlatforms]` 特性，构造函数里 4 个设置。

**关键概念**：
- `[SupportedPlatforms(UnrealPlatformClass.All)]` 声明支持所有平台
- `Type = TargetType.Game` 决定编的是游戏
- **没有** `bBuildAllModules`（游戏不需要编辑器工具模块）

---

## 七、速查表 · 口诀

| 三要素 | 值 |
|---|---|
| 读什么 | `Engine\Source\UnrealGame.Target.cs`（17 行，C#） |
| 讲义 | `从零复刻ue5\Engine\Source\03_UnrealGame.Target.cs_完整代码.md` |
| 还原到哪 | `ue_5.6\Engine\Source\UnrealGame.Target.cs` |

**口诀**：**游戏单比编辑器单多一行特性（`SupportedPlatforms(All)`）、少一行开关（`bBuildAllModules`）、`Type` 从 `Editor` 变 `Game`；其余三行一模一样。**

---

## 待确认

1. 17 行看懂了吗？重点记和编辑器单的**三处差异**。
2. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\UnrealGame.Target.cs`（17 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs`（`TargetType` 20、`TargetBuildEnvironment` 77、`EngineIncludeOrderVersion` 168、`Latest` 209、`TargetRules` 类 528、`Type` 681、`IncludeOrderVersion` 703、`ExtraModuleNames` 2716、`BuildEnvironment` 2740）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildTarget.cs`（`UnrealPlatformClass` 1039）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\SupportedPlatformsAttribute.cs`（类 12、`UnrealPlatformClass` 构造 42）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\Utils.cs`（`GetPlatformsInClass` 506、`All` 映射 513）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetInfo.cs`（`TargetInfo` 类 12）