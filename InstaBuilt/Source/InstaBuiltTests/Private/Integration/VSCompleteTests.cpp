// VSCompleteTests.cpp — Full vertical slice tests + failure scenarios

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
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
#include "Components/CompanyLayer.h"
#include "Components/QualityComponents.h"
#include "Components/WorldData.h"

#if WITH_AUTOMATION_TESTS

// ============================================================
// M4 TESTS — Company Layer
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSCompanyDashboardTest,
	"InstaBuilt.VS.CompanyDashboard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSCompanyDashboardTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FCompanyManager>());
	Orch.InitializeAll();
	
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	auto CM = Orch.GetSystem<FCompanyManager>(TEXT("CompanyManager"));
	
	GM->Cmd_NewGame(TEXT("TestCo"));
	
	// Test employee hiring
	FEntityId E1 = CM->HireEmployee(TEXT("Alice"), TEXT("Architect"), 65000);
	FEntityId E2 = CM->HireEmployee(TEXT("Bob"), TEXT("Engineer"), 72000);
	
	TestEqual("2 employees hired", CM->GetMonthlyPayroll() > 8000.0, true);
	
	// Test project finance
	CM->CreateProjectFinance(FEntityId(), 200000.0);
	CM->RecordProjectCost(FEntityId(), TEXT("Material"), 85000);
	CM->RecordProjectCost(FEntityId(), TEXT("Labor"), 45000);
	CM->RecordProjectCost(FEntityId(), TEXT("Equipment"), 12000);
	
	CM->FinalizeProject(FEntityId(), TEXT("Test Project"), TEXT("Client A"), 90.0f, true);
	
	// Test reputation
	CM->UpdateReputation(90, 95, 70, 60, 85);
	FString Dash = CM->GetReputationDashboard();
	TestTrue("Rep dashboard generated", Dash.Contains(TEXT("Quality")));
	
	// Test full dashboard
	FString FullDash = CM->GetFullDashboard();
	TestTrue("Full dashboard", FullDash.Contains(TEXT("DASHBOARD")));
	TestTrue("Contains employees", FullDash.Contains(TEXT("Alice")));
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// M5 TESTS — World Showcase
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSWorldShowcaseTest,
	"InstaBuilt.VS.WorldShowcase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSWorldShowcaseTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FWorldManager>());
	Orch.InitializeAll();
	
	auto World = Orch.GetSystem<FWorldManager>(TEXT("WorldManager"));
	World->CreateShowcaseDistrict();
	
	FString Summary = World->GetDistrictSummary();
	TestTrue("District created", Summary.Contains(TEXT("Riverside")));
	
	auto Plots = World->GetAvailablePlots();
	TestTrue("Plots available", Plots.Num() >= 3);
	
	FString PlotInfo = World->GetShowcasePlotInfo();
	TestTrue("Plot info generated", PlotInfo.Contains(TEXT("AVAILABLE")));
	
	FString Details = World->GetPlotDetails(Plots[0]);
	TestTrue("Plot details have soil type", Details.Contains(TEXT("Soil")));
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// M7 TESTS — Full Vertical Slice
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFullDemoTest,
	"InstaBuilt.VS.FullDemo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFullDemoTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
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
	Orch.InitializeAll();
	
	auto VS = Orch.GetSystem<FVerticalSliceAssembler>(TEXT("VerticalSliceAssembler"));
	
	// Run the full demo
	FString Output = VS->Cmd_RunDemo();
	
	TestTrue("Demo completed", Output.Contains(TEXT("COMPLETE")));
	TestTrue("Company created", Output.Contains(TEXT("InstaBuilt Premium")));
	TestTrue("Contract received", Output.Contains(TEXT("Johnson")));
	TestTrue("Building designed", Output.Contains(TEXT("BLUEPRINT")));
	TestTrue("Construction done", Output.Contains(TEXT("Quality")));
	TestTrue("Client happy", Output.Contains(TEXT("exactly")));
	TestTrue("Payment received", Output.Contains(TEXT("$109,250")));
	TestTrue("Save created", Output.Contains(TEXT("saved")));
	
	// Verify journey status
	FString Status = VS->GetJourneyStatus();
	TestTrue("Journey at final step", Status.Contains(TEXT("COMPLETE")));
	
	// Verify walkthrough
	FString Walkthrough = VS->GetWalkthrough();
	TestTrue("13 steps in walkthrough", Walkthrough.Contains(TEXT("13.")));
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// FAILURE SCENARIO TESTS
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFailureOverBudgetTest,
	"InstaBuilt.VS.Failure.OverBudget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFailureOverBudgetTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FEntityId ProjectId = FInstaBuiltECS::Get().CreateEntity();
	auto* Finance = FInstaBuiltECS::Get().AddComponent<C_ProjectFinance>(ProjectId);
	Finance->TotalBudget = 100000.0;
	Finance->AddCost(TEXT("Material"), 70000);
	Finance->AddCost(TEXT("Labor"), 50000);  // Over budget
	Finance->Recalculate();
	
	TestTrue("Over budget detected", Finance->bIsOverBudget);
	TestTrue("Over budget amount > 0", Finance->OverBudgetAmount > 15000);
	TestTrue("Negative profit", Finance->Profit < 0);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFailureLateDeliveryTest,
	"InstaBuilt.VS.Failure.LateDelivery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFailureLateDeliveryTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	auto* Perf = FInstaBuiltECS::Get().AddComponent<C_ProjectPerformance>(
		FInstaBuiltECS::Get().CreateEntity());
	Perf->BudgetedDays = 90;
	Perf->ActualDays = 120;  // 30 days late
	Perf->BudgetedCost = 100000;
	Perf->ActualCost = 105000;
	Perf->Finalize();
	
	TestFalse("Behind schedule", Perf->bOnSchedule);
	TestFalse("Over budget", Perf->bOnBudget);
	TestTrue("Days variance negative", Perf->DaysVariance < 0);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFailurePoorQualityTest,
	"InstaBuilt.VS.Failure.PoorQuality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFailurePoorQualityTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FEntityId BuildingId = FInstaBuiltECS::Get().CreateEntity();
	auto* Quality = FInstaBuiltECS::Get().AddComponent<C_BuildingQuality>(BuildingId);
	
	Quality->ApplyDefect(TEXT("Foundation"), 30, TEXT("Cracked slab — major rework"));
	Quality->ApplyDefect(TEXT("Structure"), 25, TEXT("Wall misalignment"));
	Quality->ApplyDefect(TEXT("Interior"), 20, TEXT("Poor paint finish"));
	Quality->ApplyDefect(TEXT("Finalization"), 15, TEXT("Missing fixtures"));
	
	TestTrue("Quality severely degraded", Quality->OverallQuality < 40);
	TestEqual("4 defects recorded", Quality->Defects.Num(), 4);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFailureClientUnsatisfiedTest,
	"InstaBuilt.VS.Failure.ClientUnsatisfied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFailureClientUnsatisfiedTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FEntityId ClientId = FInstaBuiltECS::Get().CreateEntity();
	auto* Client = FInstaBuiltECS::Get().AddComponent<C_ClientData>(ClientId);
	Client->ClientName = TEXT("Unhappy Client");
	Client->ExpectedBudget = 200000;
	Client->ExpectedTimeline = 90;
	Client->ExpectedQuality = 85;
	
	// Worst case: poor quality, way over budget, very late
	Client->CalculateSatisfaction(40, 320000, 180, false);
	
	TestTrue("Very unhappy", Client->OverallSatisfaction < 30);
	TestFalse("Would never hire again", Client->bWouldHireAgain);
	TestTrue("Negative feedback", Client->Feedback.Contains(TEXT("unhappy")) || Client->Feedback.Contains(TEXT("Disappointed")));
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
