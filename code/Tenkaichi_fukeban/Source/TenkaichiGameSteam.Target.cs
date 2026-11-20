// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiGameSteamTarget : TenkaichiGameTarget
{
	public TenkaichiGameSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
	}
}