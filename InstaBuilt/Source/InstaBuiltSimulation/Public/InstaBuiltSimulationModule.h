// InstaBuiltSimulationModule.h — Simulation module interface
// Milestone 1: Stub. Systems will be registered in Milestones 3-9.

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class INSTABUILTSIMULATION_API FInstaBuiltSimulationModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	static FInstaBuiltSimulationModule& Get();
	bool IsInitialized() const { return bInitialized; }
	
private:
	bool bInitialized = false;
};
