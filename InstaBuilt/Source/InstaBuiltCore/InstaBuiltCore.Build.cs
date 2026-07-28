// InstaBuiltCore.Build.cs
// Foundation module: ECS, Event Bus, Command Processor, Logging, Debug
// Architecture: ARCHITECTURE.md Sections 3.1, 7.1, 7.2
// Data Model: DATA_MODEL.md Section 2

using UnrealBuildTool;

public class InstaBuiltCore : ModuleRules
{
	public InstaBuiltCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Public headers (consumed by other modules)
		PublicIncludePaths.AddRange(new string[] {
			"InstaBuiltCore/Public"
		});
		
		// Private implementation
		PrivateIncludePaths.AddRange(new string[] {
			"InstaBuiltCore/Private"
		});
		
		// Core module has NO game-specific dependencies
		// It depends only on UE5 engine modules
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore"
		});
		
		// Private deps for internal implementation
		PrivateDependencyModuleNames.AddRange(new string[] {
			"DeveloperSettings"
		});
		
		// Optimization settings
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		
		// Defines for the core module
		PublicDefinitions.Add("INSTABUILT_CORE_MODULE=1");
		
		// Enable exceptions for ECS error handling
		bEnableExceptions = true;
	}
}
