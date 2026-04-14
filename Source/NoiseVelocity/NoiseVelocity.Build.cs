// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NoiseVelocity : ModuleRules
{
	public NoiseVelocity(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "CustomComputeShadersModule" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Includes for Tobii eye tracking.
		// Comment out if no eye tracking available
		string tobii_path = Path.Combine(ModuleDirectory, "..", "ThirdParty", "Tobii");
		PublicIncludePaths.Add(Path.Combine(tobii_path, "include"));
		PublicAdditionalLibraries.Add(Path.Combine(tobii_path, "lib", "tobii_gameintegration_x64.lib"));
		RuntimeDependencies.Add("$(BinaryOutputDir)/tobii_gameintegration_x64.dll", Path.Combine(tobii_path, "lib", "tobii_gameintegration_x64.dll"));


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
