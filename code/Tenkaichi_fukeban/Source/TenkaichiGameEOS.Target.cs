// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TenkaichiGameEOSTarget : TenkaichiGameTarget
{
	public TenkaichiGameEOSTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "EOS";
	}
}