// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiServerSteamEOSTarget : TenkaichiServerTarget
{
	public TenkaichiServerSteamEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "SteamEOS";
	}
}