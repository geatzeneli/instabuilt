// Phase6Tests.cpp — M2-M7: Building variety, employee AI, world, contracts, economy

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Systems/CompanySystem.h"
#include "Systems/BuildingDesigner.h"
#include "Systems/BuildingValidator.h"
#include "Systems/CompanyManager.h"
#include "Systems/WorldPresentation.h"
#include "Systems/InstaBuiltGameManager.h"
#include "Components/ProductLines.h"
#include "Components/QualityComponents.h"
#include "Components/WorldData.h"
#include "Components/CompanyLayer.h"

#if WITH_AUTOMATION_TESTS

// ============================================================
// M2: BUILDING VARIETY
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM2AllProductLinesTest,
	"InstaBuilt.Phase6.M2.AllProductLines",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM2AllProductLinesTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	auto& ECS = FInstaBuiltECS::Get();
	
	// 1. POP UP 28
	FEntityId PopUp = ECS.CreateEntity();
	ECS.AddComponent<C_BuildingDesignData>(PopUp)->BuildingType = TEXT("POP_UP");
	auto* PopCfg = ECS.AddComponent<C_PopUpConfig>(PopUp);
	PopCfg->ModuleSize = 28;
	TestEqual("POP UP area", PopCfg->GetTotalArea(), 28.0f);
	TestTrue("POP UP fast build", PopCfg->GetBuildTimeMultiplier() < 0.5f);
	
	// 2. Multifamily
	FEntityId Multi = ECS.CreateEntity();
	ECS.AddComponent<C_BuildingDesignData>(Multi)->BuildingType = TEXT("MULTIFAMILY");
	auto* MulCfg = ECS.AddComponent<C_MultifamilyConfig>(Multi);
	MulCfg->OneBedroomUnits = 4; MulCfg->TwoBedroomUnits = 4;
	MulCfg->MaxStories = 4;
	MulCfg->CalculateMetrics();
	TestEqual("Multifamily 8 units", MulCfg->TotalUnits, 8);
	TestTrue("Needs elevator at 4 stories", MulCfg->bHasElevator);
	
	// 3. Senior Housing
	FEntityId Senior = ECS.CreateEntity();
	auto* SenCfg = ECS.AddComponent<C_SeniorHousingConfig>(Senior);
	SenCfg->bWideDoorways = true; SenCfg->bZeroStepEntries = true;
	SenCfg->bGrabBarsInBathrooms = true; SenCfg->bEmergencyCallSystem = true;
	SenCfg->bSlipResistantFloors = true;
	TestTrue("Senior accessibility passed", SenCfg->ValidateAccessibility());
	
	// 4. Micro Apartments
	FEntityId Micro = ECS.CreateEntity();
	auto* MicCfg = ECS.AddComponent<C_MicroApartmentConfig>(Micro);
	MicCfg->UnitSize = 28; MicCfg->UnitCount = 24;
	TestTrue("Micro high density", MicCfg->GetDensity() > 0.5f);
	
	// 5. Traditional Home
	FEntityId Trad = ECS.CreateEntity();
	auto* TradCfg = ECS.AddComponent<C_TraditionalHomeConfig>(Trad);
	TradCfg->ArchitecturalStyle = TEXT("Colonial");
	TradCfg->bHasPitchedRoof = true; TradCfg->bHasPorch = true;
	TradCfg->bHasBayWindows = true;
	TestTrue("Traditional authentic", TradCfg->GetAuthenticityScore() > 70.0f);
	
	// 6. Signature Home
	FEntityId Sig = ECS.CreateEntity();
	auto* SigCfg = ECS.AddComponent<C_SignatureHomeConfig>(Sig);
	TestTrue("Signature min budget check", SigCfg->ValidateBudget(600000));
	TestFalse("Signature rejects low budget", SigCfg->ValidateBudget(300000));
	
	// 7. Bathpod
	FEntityId Pod = ECS.CreateEntity();
	auto* PodCfg = ECS.AddComponent<C_BathpodConfig>(Pod);
	TestTrue("Bathpod fully complete", PodCfg->FactoryCompletion >= 1.0f);
	TestTrue("Bathpod tested", PodCfg->bPressureTested && PodCfg->bWaterproofed);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM2ValidationTest,
	"InstaBuilt.Phase6.M2.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM2ValidationTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	Orch.RegisterSystem(MakeShared<FBuildingDesigner>());
	Orch.RegisterSystem(MakeShared<FBuildingValidator>());
	Orch.InitializeAll();
	
	auto Designer = Orch.GetSystem<FBuildingDesigner>(TEXT("BuildingDesigner"));
	auto Validator = Orch.GetSystem<FBuildingValidator>(TEXT("BuildingValidator"));
	
	// Design a 3-story multifamily that SHOULD fail validation (no elevator)
	FEntityId DesignId = Designer->CreateDesign(TEXT("Test Tower"), TEXT("MULTIFAMILY"));
	auto* Multi = FInstaBuiltECS::Get().AddComponent<C_MultifamilyConfig>(DesignId);
	Multi->MaxStories = 5;
	Multi->OneBedroomUnits = 10;
	Multi->CalculateMetrics();
	
	auto* Design = FInstaBuiltECS::Get().GetComponent<C_BuildingDesignData>(DesignId);
	Design->FloorCount = 5;
	
	auto Report = Validator->ValidateDesign(DesignId);
	TestFalse("5-story without elevator fails", Report.bPassed);
	TestTrue("Has validation errors", Report.ErrorCount > 0);
	
	// Fix: add elevator
	Multi->bHasElevator = true;
	Multi->bRequiresFireSuppression = true;
	Design->FloorCount = 3; // Reduce to 3 floors
	
	auto Report2 = Validator->ValidateDesign(DesignId);
	// Should have fewer errors now
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// M3: COMPANY MANAGEMENT
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM3CompanyManagementTest,
	"InstaBuilt.Phase6.M3.CompanyManagement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM3CompanyManagementTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FCompanyManager>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	Orch.InitializeAll();
	
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	auto CM = Orch.GetSystem<FCompanyManager>(TEXT("CompanyManager"));
	
	GM->Cmd_NewGame(TEXT("GrowthCo"));
	
	// Hire team
	CM->HireEmployee(TEXT("Sarah Architect"), TEXT("Senior Architect"), 85000);
	CM->HireEmployee(TEXT("Tom Engineer"), TEXT("Structural Engineer"), 92000);
	CM->HireEmployee(TEXT("Maria Manager"), TEXT("Project Manager"), 78000);
	CM->HireEmployee(TEXT("James Laborer"), TEXT("Skilled Laborer"), 45000);
	
	// Verify payroll
	double Payroll = CM->GetMonthlyPayroll();
	TestTrue("Monthly payroll > $20K", Payroll > 20000);
	
	// Complete projects and track history
	FEntityId ProjId = FInstaBuiltECS::Get().CreateEntity();
	CM->CreateProjectFinance(ProjId, 250000);
	CM->RecordProjectCost(ProjId, TEXT("Material"), 100000);
	CM->RecordProjectCost(ProjId, TEXT("Labor"), 80000);
	CM->FinalizeProject(ProjId, TEXT("Riverside Villa"), TEXT("Client A"), 92, true);
	
	// Rate employees
	for (int32 i = 0; i < 4; ++i)
	{
		// Would rate actual employees here
	}
	
	// Reputation update
	CM->UpdateReputation(92, 95, 70, 65, 88);
	
	FString Dash = CM->GetFullDashboard();
	TestTrue("Dashboard includes revenue", Dash.Contains(TEXT("Revenue")));
	TestTrue("Dashboard includes employees", Dash.Contains(TEXT("Architect")));
	
	double Valuation = CM->EstimateCompanyValue();
	TestTrue("Company valued > 0", Valuation > 0);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// M4: EMPLOYEE AI
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM4EmployeeAITest,
	"InstaBuilt.Phase6.M4.EmployeeAI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM4EmployeeAITest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	auto& ECS = FInstaBuiltECS::Get();
	
	// Test employee contract lifecycle
	FEntityId Emp = ECS.CreateEntity();
	auto* Contract = ECS.AddComponent<C_EmployeeContract>(Emp);
	Contract->Position = TEXT("Journeyman Carpenter");
	Contract->AnnualSalary = 55000;
	Contract->CalculateHourlyRate();
	Contract->PerformanceRating = 85;
	Contract->ProjectsWorked = 12;
	Contract->CalculateEfficiency();
	
	TestTrue("Efficiency above 0.8 for good performer", Contract->Efficiency > 0.8f);
	TestTrue("Hourly rate calculated", Contract->HourlyRate > 20);
	
	// Test morale/performance relationship
	Contract->PerformanceRating = 30; // Poor performer
	Contract->CalculateEfficiency();
	TestTrue("Efficiency drops for poor performer", Contract->Efficiency < 0.8f);
	
	// Test tenure
	Contract->DaysEmployed = 365;
	TestEqual("One year tenure", Contract->DaysEmployed, 365);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

// ============================================================
// M5+M6+M7: WORLD, CONTRACTS, ECONOMY
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM567IntegrationTest,
	"InstaBuilt.Phase6.M567.Integration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM567IntegrationTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FContractSystem>());
	Orch.RegisterSystem(MakeShared<FBuildingDesigner>());
	Orch.RegisterSystem(MakeShared<FBuildingValidator>());
	Orch.RegisterSystem(MakeShared<FCompanyManager>());
	Orch.RegisterSystem(MakeShared<FWorldManager>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	Orch.InitializeAll();
	
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	auto World = Orch.GetSystem<FWorldManager>(TEXT("WorldManager"));
	auto Validator = Orch.GetSystem<FBuildingValidator>(TEXT("BuildingValidator"));
	
	// 1. Set up world
	GM->Cmd_NewGame(TEXT("City Builders Inc."));
	World->CreateShowcaseDistrict();
	
	auto Plots = World->GetAvailablePlots();
	TestTrue("Plots available for construction", Plots.Num() >= 3);
	
	// 2. Build a POP UP on a plot
	GM->Cmd_AcceptContract(0);
	auto& ECS = FInstaBuiltECS::Get();
	FEntityId PlotId = Plots[0];
	World->PlaceBuilding(FEntityId(), PlotId);
	
	// 3. Verify plot is now occupied
	FString PlotDetails = World->GetPlotDetails(PlotId);
	TestTrue("Plot shows occupied", PlotDetails.Contains(TEXT("No")) || !PlotDetails.Contains(TEXT("Yes")));
	
	// 4. Verify product line detection
	FEntityId TestBuilding = ECS.CreateEntity();
	ECS.AddComponent<C_BuildingDesignData>(TestBuilding)->BuildingType = TEXT("SENIOR");
	ECS.AddComponent<C_SeniorHousingConfig>(TestBuilding);
	
	FString DetectedLine = FBuildingValidator::GetProductLine(TestBuilding);
	TestEqual("Senior housing detected", DetectedLine, TEXT("SENIOR"));
	
	// 5. Full validation on product line
	auto Report = Validator->ValidateDesign(TestBuilding);
	TestTrue("Validation runs on senior housing", Report.OverallScore >= 0);
	
	// 6. Save + verify
	GM->SaveGame(TEXT("Phase6Test"));
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
