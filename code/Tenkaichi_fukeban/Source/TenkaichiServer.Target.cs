// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class TenkaichiServerTarget : TargetRules
{
	public TenkaichiServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		ExtraModuleNames.AddRange(new string[] { "TenkaichiGame" });

		TenkaichiGameTarget.ApplySharedTenkaichiTargetSettings(this);

		bUseChecksInShipping = true;
	}
}