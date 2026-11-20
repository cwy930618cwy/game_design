// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonUser : ModuleRules
{
	public CommonUser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bool bUseOnlineSubsystemV1 = true;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... 在此处添加所需的公共 include 路径 ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... 在此处添加其他所需的私有 include 路径 ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreOnline",
				"GameplayTags",
				"OnlineSubsystemUtils",
				// ... 在此处添加其他静态链接的公共依赖 ...
			}
			);

		if (bUseOnlineSubsystemV1)
		{
			PublicDependencyModuleNames.Add("OnlineSubsystem");
		}
		else
		{
			PublicDependencyModuleNames.Add("OnlineServicesInterface");
		}

		PublicDefinitions.Add("COMMONUSER_OSSV1=" + (bUseOnlineSubsystemV1 ? "1" : "0"));

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreOnline",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"InputCore",
				// ... 在此处添加其他静态链接的私有依赖 ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... 在此处添加你的模块动态加载的任何模块 ...
			}
			);
	}
}
