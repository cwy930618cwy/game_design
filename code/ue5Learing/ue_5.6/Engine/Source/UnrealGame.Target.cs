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