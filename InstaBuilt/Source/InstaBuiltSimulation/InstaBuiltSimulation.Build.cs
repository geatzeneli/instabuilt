// InstaBuiltSimulation.Build.cs
// Simulation module: all domain systems (Building, Construction, Contract, Worker, Economy, etc.)
// Architecture: ARCHITECTURE.md Section 3.2-3.10

using UnrealBuildTool;

public class InstaBuiltSimulation : ModuleRules
{
	public InstaBuiltSimulation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.Add("InstaBuiltSimulation/Public");
		PrivateIncludePaths.Add("InstaBuiltSimulation/Private");
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InstaBuiltCore"      // Depends on ECS, EventBus, Commands
		});
		
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		bEnableExceptions = true;
	}
}
