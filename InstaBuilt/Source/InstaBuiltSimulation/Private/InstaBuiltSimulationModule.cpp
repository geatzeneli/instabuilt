// InstaBuiltSimulationModule.cpp — Full system registration (M1-M7)
#include "InstaBuiltSimulationModule.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Systems/CompanySystem.h"
#include "Systems/ContractSystem.h"
#include "Systems/BuildingSystem.h"
#include "Systems/WorkerEconomy.h"
#include "Systems/BuildingDesigner.h"
#include "Systems/CompanyManager.h"
#include "Systems/WorldPresentation.h"
#include "Systems/InstaBuiltGameManager.h"
#include "Logging/InstaBuiltLog.h"

void FInstaBuiltSimulationModule::StartupModule()
{
	IB_LOG_INFO("InstaBuiltSimulation starting — registering all systems...");
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FContractSystem>());
	Orch.RegisterSystem(MakeShared<FBuildingSystem>());
	Orch.RegisterSystem(MakeShared<FConstructionSystem>());
	Orch.RegisterSystem(MakeShared<FWorkerSystem>());
	Orch.RegisterSystem(MakeShared<FEconomySystem>());
	Orch.RegisterSystem(MakeShared<FBuildingDesigner>());
	Orch.RegisterSystem(MakeShared<FCompanyManager>());
	Orch.RegisterSystem(MakeShared<FWorldManager>());
	Orch.RegisterSystem(MakeShared<FVerticalSliceAssembler>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	
	IB_LOG_INFO("All %d simulation systems registered.", Orch.GetSystemCount());
	bInitialized = true;
}

void FInstaBuiltSimulationModule::ShutdownModule() { bInitialized = false; }
FInstaBuiltSimulationModule& FInstaBuiltSimulationModule::Get()
	{ return FModuleManager::LoadModuleChecked<FInstaBuiltSimulationModule>("InstaBuiltSimulation"); }
IMPLEMENT_MODULE(FInstaBuiltSimulationModule, InstaBuiltSimulation)
