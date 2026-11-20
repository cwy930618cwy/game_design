// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiServerEOSTarget : TenkaichiServerTarget
{
	public TenkaichiServerEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "EOS";
	}
}