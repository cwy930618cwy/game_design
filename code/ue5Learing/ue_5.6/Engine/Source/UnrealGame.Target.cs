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