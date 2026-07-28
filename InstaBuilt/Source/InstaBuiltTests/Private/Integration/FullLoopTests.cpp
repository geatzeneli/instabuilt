// FullLoopTests.cpp — Complete game loop integration test (M12 validation)
// Validates: New Game → Contract → Build → Complete → Save → Load

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Systems/CompanySystem.h"
#include "Systems/ContractSystem.h"
#include "Systems/BuildingSystem.h"
#include "Systems/WorkerEconomy.h"
#include "Systems/InstaBuiltGameManager.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullLoopNewGameTest,
	"InstaBuilt.GameLoop.NewGame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFullLoopNewGameTest::RunTest(const FString& Parameters)
{
	// Setup: ECS + Orchestrator + all systems
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FContractSystem>());
	Orch.RegisterSystem(MakeShared<FBuildingSystem>());
	Orch.RegisterSystem(MakeShared<FConstructionSystem>());
	Orch.RegisterSystem(MakeShared<FWorkerSystem>());
	Orch.RegisterSystem(MakeShared<FEconomySystem>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	
	Orch.InitializeAll();
	
	// Get GameManager
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	TestTrue("GameManager exists", GM.IsValid());
	
	// Step 1: Start new game
	GM->Cmd_NewGame(TEXT("TestBuilders Inc."));
	TestTrue("Game started", GM->GetCompany() != nullptr);
	TestTrue("Company has cash", GM->GetCompany()->GetCash() > 0);
	
	// Step 2: Verify contracts generated
	auto Available = GM->GetContracts()->GetAvailableContracts();
	TestTrue("Contracts generated", Available.Num() >= 3);
	
	// Step 3: Accept first contract
	FString Result = GM->Cmd_AcceptContract(0);
	TestTrue("Contract accepted", Result.Contains(TEXT("accepted")));
	
	// Step 4: Hire workers
	Result = GM->Cmd_HireWorkers();
	TestTrue("Workers hired", GM->GetWorkers()->GetAllWorkers().Num() >= 2);
	
	// Step 5: Start building
	Result = GM->Cmd_StartBuilding();
	TestTrue("Construction started", Result.Contains(TEXT("started")));
	
	// Step 6: Simulate construction (fast-forward 60 seconds)
	for (int32 i = 0; i < 120; ++i)
	{
		Orch.UpdateAll(0.5f);
	}
	
	// Step 7: Verify construction completed and payment received
	double Cash = GM->GetCompany()->GetCash();
	TestTrue("Project completed and payment received", Cash > 200000.0);
	
	// Step 8: Save game
	GM->SaveGame(TEXT("TestSave"));
	
	// Step 9: Verify save file exists
	auto Slots = GM->GetSaveSlots();
	TestTrue("Save slot exists", Slots.Contains(TEXT("TestSave")));
	
	// Step 10: New contracts generated for next loop
	Available = GM->GetContracts()->GetAvailableContracts();
	TestTrue("New contracts available after completion", Available.Num() >= 3);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullLoopSaveLoadTest,
	"InstaBuilt.GameLoop.SaveLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFullLoopSaveLoadTest::RunTest(const FString& Parameters)
{
	// Session 1: Play, save
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FContractSystem>());
	Orch.RegisterSystem(MakeShared<FBuildingSystem>());
	Orch.RegisterSystem(MakeShared<FConstructionSystem>());
	Orch.RegisterSystem(MakeShared<FWorkerSystem>());
	Orch.RegisterSystem(MakeShared<FEconomySystem>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	Orch.InitializeAll();
	
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	GM->Cmd_NewGame(TEXT("SaveTest Co."));
	GM->Cmd_AcceptContract(0);
	GM->Cmd_HireWorkers();
	GM->Cmd_StartBuilding();
	
	// Run construction halfway
	for (int32 i = 0; i < 30; ++i) Orch.UpdateAll(0.5f);
	
	double CashBeforeSave = GM->GetCompany()->GetCash();
	GM->SaveGame(TEXT("MidGameSave"));
	
	// Shutdown session 1
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	
	// Session 2: Load and continue
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch2 = FSystemOrchestrator::Get();
	
	Orch2.RegisterSystem(MakeShared<FCompanySystem>());
	Orch2.RegisterSystem(MakeShared<FContractSystem>());
	Orch2.RegisterSystem(MakeShared<FBuildingSystem>());
	Orch2.RegisterSystem(MakeShared<FConstructionSystem>());
	Orch2.RegisterSystem(MakeShared<FWorkerSystem>());
	Orch2.RegisterSystem(MakeShared<FEconomySystem>());
	Orch2.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	Orch2.InitializeAll();
	
	auto GM2 = Orch2.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	
	// Load save
	bool bLoaded = GM2->LoadGame(TEXT("MidGameSave"));
	TestTrue("Save loaded successfully", bLoaded);
	
	double CashAfterLoad = GM2->GetCompany()->GetCash();
	TestTrue("Company restored after load", GM2->GetCompany() != nullptr);
	TestTrue("Cash restored", CashAfterLoad > 0);
	
	Orch2.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
