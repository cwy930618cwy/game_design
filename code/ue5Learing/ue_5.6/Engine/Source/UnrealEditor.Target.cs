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