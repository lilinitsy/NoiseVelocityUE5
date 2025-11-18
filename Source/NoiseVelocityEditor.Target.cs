// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class NoiseVelocityEditorTarget : TargetRules
{
	public NoiseVelocityEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("NoiseVelocity");
		ExtraModuleNames.Add("CustomComputeShadersModule");

		// This might build the unreal lightmass tool too?

		PreBuildTargets.Add(
			new TargetInfo(
				"UnrealLightmass", 
				Target.Platform, 
				Target.Configuration, 
				Target.Architectures,
				null, 
				Target.Arguments));
	}
}
