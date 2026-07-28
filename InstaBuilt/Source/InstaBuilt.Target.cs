// InstaBuilt.Target.cs — Game build target
// Milestone 1: Project Foundation
// Architecture: ROADMAP.md Section 7, ARCHITECTURE.md Section 5

using UnrealBuildTool;
using System.Collections.Generic;

public class InstaBuiltTarget : TargetRules
{
	public InstaBuiltTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		
		ExtraModuleNames.AddRange(new string[] {
			"InstaBuiltCore",
			"InstaBuiltSimulation", 
			"InstaBuiltGame"
		});
		
		// Performance: Enable Unity Build for faster iteration
		bUseUnityBuild = true;
		
		// Memory: Use shared PCH for core modules
		bUseSharedPCHs = true;
		
		// Debug: Enable profiling in Development builds
		if (Configuration == UnrealTargetConfiguration.Development)
		{
			GlobalDefinitions.Add("INSTABUILT_ENABLE_PROFILING=1");
		}
		
		// Shipping: Strip debug symbols for release
		if (Configuration == UnrealTargetConfiguration.Shipping)
		{
			GlobalDefinitions.Add("INSTABUILT_SHIPPING=1");
		}
	}
}
