// InstaBuiltEditor.Target.cs — Editor build target
// Includes editor-only modules (tests, debug tools)

using UnrealBuildTool;
using System.Collections.Generic;

public class InstaBuiltEditorTarget : TargetRules
{
	public InstaBuiltEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		
		ExtraModuleNames.AddRange(new string[] {
			"InstaBuiltCore",
			"InstaBuiltSimulation",
			"InstaBuiltGame",
			"InstaBuiltTests"
		});
		
		bUseUnityBuild = true;
		bUseSharedPCHs = true;
		
		// Editor: full debug + profiling + test support
		GlobalDefinitions.Add("INSTABUILT_EDITOR=1");
		GlobalDefinitions.Add("INSTABUILT_ENABLE_PROFILING=1");
		GlobalDefinitions.Add("INSTABUILT_ENABLE_TESTS=1");
	}
}
