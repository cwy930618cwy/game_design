// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiServerSteamTarget : TenkaichiServerTarget
{
	public TenkaichiServerSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
	}
}