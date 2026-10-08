// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiEditorTarget : TargetRules
{
	public TenkaichiEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "TenkaichiGame", "TenkaichiEditor" });

		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		TenkaichiGameTarget.ApplySharedTenkaichiTargetSettings(this);

		// 这是配合「Unreal Remote 2」应用做触屏开发用的
		EnablePlugins.Add("RemoteSession");
	}
}