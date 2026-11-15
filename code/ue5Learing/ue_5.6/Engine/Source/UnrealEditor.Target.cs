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