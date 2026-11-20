// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// SteamEOS refers to a game published and launched on Steam while still taking advantage of EOS for online and crossplay (integrated platform).

public class TenkaichiGameSteamEOSTarget : TenkaichiGameTarget
{
	public TenkaichiGameSteamEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "SteamEOS";
	}
}