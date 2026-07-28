// InstaBuiltGame.Build.cs

using UnrealBuildTool;

public class InstaBuiltGame : ModuleRules
{
	public InstaBuiltGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore",
			"InstaBuiltCore",
			"InstaBuiltSimulation"
		});
		
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
