// InstaBuiltTests.Build.cs

using UnrealBuildTool;

public class InstaBuiltTests : ModuleRules
{
	public InstaBuiltTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"InstaBuiltCore",
			"InstaBuiltSimulation",
			"InstaBuiltGame"
		});
		
		// Editor-only (see InstaBuiltEditor.Target.cs)
	}
}
