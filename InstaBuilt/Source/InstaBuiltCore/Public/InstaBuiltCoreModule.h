// InstaBuiltCoreModule.h — Core module interface
// Milestone 1: Project Foundation
// Owns: ECS Core, Event Bus, Command Processor, Logging, Debug

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FInstaBuiltCoreModule
 * 
 * Foundation module for the entire InstaBuilt engine.
 * Initializes the ECS core, event bus, command processor,
 * logging framework, and debug tools.
 * 
 * All other modules depend on this one.
 * This module depends on nothing except UE5 engine modules.
 * 
 * Architecture: ARCHITECTURE.md Section 3.1, 6.1
 */
class INSTABUILTCORE_API FInstaBuiltCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** Singleton accessor */
	static FInstaBuiltCoreModule& Get();
	
	/** Is the core module fully initialized? */
	bool IsInitialized() const { return bInitialized; }
	
private:
	bool bInitialized = false;
	
	/** Initialize subsystems in dependency order */
	void InitializeECS();
	void InitializeEventBus();
	void InitializeCommandProcessor();
	void InitializeLogging();
	void InitializeDebugTools();
	
	/** Shutdown in reverse dependency order */
	void ShutdownDebugTools();
	void ShutdownLogging();
	void ShutdownCommandProcessor();
	void ShutdownEventBus();
	void ShutdownECS();
};
