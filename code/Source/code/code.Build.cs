// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class code : ModuleRules
{
	public code(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 【重点】添加 GAS 相关的三个模块依赖
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayAbilities",   // ← GAS 核心
			"GameplayTags",        // ← Tag 系统（GAS 依赖）
			"GameplayTasks"        // ← 异步任务（GAS 依赖）
		});
	}
}
