using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;
public class CustomComputeShadersModule : ModuleRules
{
	public CustomComputeShadersModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Renderer", "RenderCore", "RHI" });


		// Old blog way
		// string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
		// PublicIncludePaths.Add(EnginePath + "Source/Runtime/Renderer/Private");

		string rendererDir = Path.Combine(GetModuleDirectory("Renderer"));

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(rendererDir, "Private"),
			Path.Combine(rendererDir, "Internal"),
        });

	}
}