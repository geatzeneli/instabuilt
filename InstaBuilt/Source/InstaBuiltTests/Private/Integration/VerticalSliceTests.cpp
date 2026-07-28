// VerticalSliceTests.cpp — Full VS experience: Design → Quality → Build → Client Satisfaction

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Systems/CompanySystem.h"
#include "Systems/ContractSystem.h"
#include "Systems/BuildingSystem.h"
#include "Systems/WorkerEconomy.h"
#include "Systems/BuildingDesigner.h"
#include "Systems/InstaBuiltGameManager.h"
#include "Components/BuildingDesign.h"
#include "Components/QualityComponents.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSBuildingDesignTest,
	"InstaBuilt.VerticalSlice.DesignTool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSBuildingDesignTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FBuildingDesigner>());
	Orch.InitializeAll();
	
	auto Designer = Orch.GetSystem<FBuildingDesigner>(TEXT("BuildingDesigner"));
	TestTrue("Designer exists", Designer.IsValid());
	
	// Step 1: Create design
	FEntityId DesignId = Designer->CreateDesign(TEXT("Riverside Villa"), TEXT("TRADITIONAL_HOME"));
	TestTrue("Design created", DesignId.IsValid());
	
	// Step 2: Add rooms
	FEntityId LivingRoom = Designer->AddRoom(TEXT("Living Room"), TEXT("Living"), 0, 0, 6, 5);
	FEntityId Kitchen = Designer->AddRoom(TEXT("Kitchen"), TEXT("Kitchen"), 6, 0, 4, 5);
	FEntityId Bedroom = Designer->AddRoom(TEXT("Master Bedroom"), TEXT("Bedroom"), 0, 5, 5, 4);
	FEntityId Bathroom = Designer->AddRoom(TEXT("Bathroom"), TEXT("Bathroom"), 5, 5, 3, 4);
	
	TestEqual("4 rooms created", Designer->GetRooms().Num(), 4);
	
	// Step 3: Validate (should fail — no doors, no windows)
	bool bValid = Designer->ValidateDesign();
	TestFalse("Design fails validation (no doors/windows)", bValid);
	
	// Step 4: Set materials to Premium
	Designer->SetMaterialTier(EMaterialTier::Premium);
	
	// Step 5: Get blueprint
	FString Blueprint = Designer->GetBlueprintSummary();
	TestTrue("Blueprint generated", Blueprint.Contains(TEXT("BLUEPRINT")));
	TestTrue("Premium tier in blueprint", Blueprint.Contains(TEXT("Premium")));
	
	// Step 6: Approve should fail
	bool bApproved = Designer->ApproveDesign();
	TestFalse("Design not approved (validation fails)", bApproved);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSQualitySatisfactionTest,
	"InstaBuilt.VerticalSlice.QualityAndClient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSQualitySatisfactionTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	// Test building quality component
	FEntityId BuildingId = FInstaBuiltECS::Get().CreateEntity(EEntityType::Building);
	auto* Quality = FInstaBuiltECS::Get().AddComponent<C_BuildingQuality>(BuildingId);
	
	Quality->FoundationQuality = 95.0f;
	Quality->StructureQuality = 90.0f;
	Quality->InteriorQuality = 88.0f;
	Quality->FinalizationQuality = 92.0f;
	Quality->Recalculate();
	
	TestTrue("Quality between 85-95", Quality->OverallQuality > 85.0f && Quality->OverallQuality < 96.0f);
	
	// Apply a defect
	Quality->ApplyDefect(TEXT("Structure"), 15.0f, TEXT("Wall misalignment in north corner"));
	TestTrue("Quality dropped after defect", Quality->OverallQuality < 90.0f);
	TestEqual("One defect recorded", Quality->Defects.Num(), 1);
	
	// Test client satisfaction
	FEntityId ClientId = FInstaBuiltECS::Get().CreateEntity(EEntityType::Client);
	auto* Client = FInstaBuiltECS::Get().AddComponent<C_ClientData>(ClientId);
	Client->ClientName = TEXT("Johnson Family");
	Client->ExpectedBudget = 200000.0;
	Client->ExpectedTimeline = 120;
	Client->ExpectedQuality = 85.0f;
	
	// Excellent delivery
	Client->CalculateSatisfaction(95.0f, 190000.0, 110, true);
	TestTrue("Happy client (>90)", Client->OverallSatisfaction > 90.0f);
	TestTrue("Would hire again", Client->bWouldHireAgain);
	TestTrue("Positive feedback", Client->Feedback.Contains(TEXT("Exceptional")));
	
	// Poor delivery
	Client->CalculateSatisfaction(55.0f, 250000.0, 160, false);
	TestTrue("Unhappy client (<60)", Client->OverallSatisfaction < 60.0f);
	TestFalse("Would not hire again", Client->bWouldHireAgain);
	
	// Test project performance
	FEntityId ProjectId = FInstaBuiltECS::Get().CreateEntity(EEntityType::Building);
	auto* Perf = FInstaBuiltECS::Get().AddComponent<C_ProjectPerformance>(ProjectId);
	Perf->BudgetedCost = 200000.0;
	Perf->ActualCost = 185000.0;
	Perf->BudgetedDays = 120;
	Perf->ActualDays = 110;
	Perf->Finalize();
	
	TestTrue("Under budget", Perf->bOnBudget);
	TestTrue("Ahead of schedule", Perf->bOnSchedule);
	TestTrue("Cost variance positive", Perf->CostVariance > 0);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVSFullVerticalSliceTest,
	"InstaBuilt.VerticalSlice.FullExperience",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVSFullVerticalSliceTest::RunTest(const FString& Parameters)
{
	// Full vertical slice: Design → Validate → Build → Quality → Client
	FInstaBuiltECS::Get().Initialize();
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	Orch.RegisterSystem(MakeShared<FCompanySystem>());
	Orch.RegisterSystem(MakeShared<FContractSystem>());
	Orch.RegisterSystem(MakeShared<FBuildingSystem>());
	Orch.RegisterSystem(MakeShared<FConstructionSystem>());
	Orch.RegisterSystem(MakeShared<FWorkerSystem>());
	Orch.RegisterSystem(MakeShared<FEconomySystem>());
	Orch.RegisterSystem(MakeShared<FBuildingDesigner>());
	Orch.RegisterSystem(MakeShared<FInstaBuiltGameManager>());
	Orch.InitializeAll();
	
	auto GM = Orch.GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	auto Designer = Orch.GetSystem<FBuildingDesigner>(TEXT("BuildingDesigner"));
	
	// 1. Start game
	GM->Cmd_NewGame(TEXT("InstaBuilt Premium"));
	
	// 2. Design the building
	Designer->CreateDesign(TEXT("Lakeside Residence"), TEXT("TRADITIONAL_HOME"));
	Designer->AddRoom(TEXT("Living Room"), TEXT("Living"), 0, 0, 7, 6);
	Designer->AddRoom(TEXT("Kitchen"), TEXT("Kitchen"), 7, 0, 5, 6);
	Designer->AddRoom(TEXT("Master Bedroom"), TEXT("Bedroom"), 0, 6, 6, 5);
	Designer->AddRoom(TEXT("Guest Bedroom"), TEXT("Bedroom"), 6, 6, 5, 5);
	Designer->AddRoom(TEXT("Bathroom"), TEXT("Bathroom"), 11, 6, 3, 5);
	Designer->SetMaterialTier(EMaterialTier::Premium);
	
	TestEqual("5 rooms designed", Designer->GetRooms().Num(), 5);
	
	// 3. Accept contract and build
	FString Result = GM->Cmd_AcceptContract(0);
	TestTrue("Contract accepted", Result.Contains(TEXT("accepted")));
	
	GM->Cmd_HireWorkers();
	GM->Cmd_StartBuilding();
	
	// 4. Run construction with quality tracking
	FEntityId SiteId = GM->GetConstruction()->GetActiveSites().Num() > 0
		? GM->GetConstruction()->GetActiveSites()[0] : FEntityId::Invalid();
	
	if (SiteId.IsValid())
	{
		// Simulate 60 seconds of construction
		for (int32 i = 0; i < 120; ++i) Orch.UpdateAll(0.5f);
	}
	
	// 5. Verify company growth
	double FinalCash = GM->GetCompany()->GetCash();
	TestTrue("Company has grown", FinalCash > 200000.0);
	
	// 6. Save
	GM->SaveGame(TEXT("VerticalSlice"));
	auto Slots = GM->GetSaveSlots();
	TestTrue("Save created", Slots.Contains(TEXT("VerticalSlice")));
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
