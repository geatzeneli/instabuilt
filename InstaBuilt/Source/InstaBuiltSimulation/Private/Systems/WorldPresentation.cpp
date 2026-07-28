// WorldPresentation.cpp — VS M5+M6+M7: World, Camera, UI, Audio, Full Assembly

#include "Systems/WorldPresentation.h"
#include "Systems/CompanyManager.h"
#include "Systems/BuildingDesigner.h"
#include "Systems/InstaBuiltGameManager.h"
#include "Logging/InstaBuiltLog.h"

// ============================================================
// WORLD MANAGER (M5)
// ============================================================

FWorldManager::FWorldManager()
	: FInstaBuiltSystem(TEXT("WorldManager"))
{
}

void FWorldManager::OnInitialize()
{
	IB_LOG_INFO("WorldManager initialized.");
}

FEntityId FWorldManager::CreateShowcaseDistrict()
{
	DistrictId = ECS.CreateEntity(EEntityType::District);
	
	auto* District = ECS.AddComponent<C_District>(DistrictId);
	District->DistrictName = TEXT("Riverside District");
	District->Description = TEXT("A premium residential neighborhood along the river.");
	District->MinX = 0; District->MinY = 0;
	District->MaxX = 500; District->MaxY = 400;
	District->ZoningType = TEXT("Residential");
	District->MaxBuildingHeight = 12.0f;
	District->MinSetback = 3.0f;
	District->EnvironmentType = TEXT("Riverside");
	District->TimeOfDay = TEXT("GoldenHour");
	District->WeatherPreset = TEXT("Clear");
	
	// Create showcase plots
	FEntityId Plot1 = CreatePlot(TEXT("100 Riverside Drive"), 100, 200, 30, 25);
	FEntityId Plot2 = CreatePlot(TEXT("150 Riverside Drive"), 160, 200, 28, 22);
	FEntityId Plot3 = CreatePlot(TEXT("200 Riverside Drive"), 220, 200, 32, 25);
	FEntityId Showcase = CreatePlot(TEXT("300 Riverside Drive (Showcase)"), 300, 200, 35, 30);
	
	// Mark showcase as having a completed building
	auto* ShowcasePlot = ECS.GetComponent<C_Plot>(Showcase);
	if (ShowcasePlot) ShowcasePlot->bHasBuilding = true;
	
	// Create roads
	CreateRoad(TEXT("Riverside Drive"), TEXT("MainStreet"), 0, 180, 500, 180);
	CreateRoad(TEXT("Oak Avenue"), TEXT("Residential"), 80, 0, 80, 400);
	CreateRoad(TEXT("Pine Street"), TEXT("Residential"), 200, 0, 200, 400);
	
	auto* Config = ECS.AddComponent<C_ShowcaseConfig>(DistrictId);
	
	IB_LOG_INFO("Showcase district created: %s (%d plots, %d roads)",
		*District->DistrictName, Plots.Num(), Roads.Num());
	
	return DistrictId;
}

FEntityId FWorldManager::CreatePlot(const FString& Address, float X, float Y, float Width, float Depth)
{
	FEntityId PlotId = ECS.CreateEntity(EEntityType::Parcel);
	
	auto* Plot = ECS.AddComponent<C_Plot>(PlotId);
	Plot->PlotAddress = Address;
	Plot->SizeX = Width;
	Plot->SizeY = Depth;
	Plot->Area = Width * Depth;
	Plot->PositionX = X;
	Plot->PositionY = Y;
	Plot->bIsAvailable = true;
	
	Plots.Add(PlotId);
	
	auto* District = ECS.GetComponent<C_District>(DistrictId);
	if (District) District->PlotIds.Add(PlotId);
	
	return PlotId;
}

FEntityId FWorldManager::CreateRoad(const FString& Name, const FString& Type,
	float SX, float SY, float EX, float EY)
{
	FEntityId RoadId = ECS.CreateEntity(EEntityType::District); // Road doesn't have own type yet
	
	auto* Road = ECS.AddComponent<C_Road>(RoadId);
	Road->RoadName = Name;
	Road->RoadType = Type;
	Road->StartX = SX; Road->StartY = SY;
	Road->EndX = EX; Road->EndY = EY;
	
	Roads.Add(RoadId);
	return RoadId;
}

FString FWorldManager::GetDistrictSummary() const
{
	auto* District = ECS.GetComponent<C_District>(DistrictId);
	if (!District) return TEXT("No district.");
	
	return FString::Printf(TEXT("%s — %d plots, %d roads | %s | %s"),
		*District->DistrictName, Plots.Num(), Roads.Num(),
		*District->ZoningType, *District->EnvironmentType);
}

FString FWorldManager::GetShowcasePlotInfo() const
{
	FString Info;
	Info += TEXT("SHOWCASE PLOTS:\n");
	for (auto& PlotId : Plots)
	{
		auto* Plot = ECS.GetComponent<C_Plot>(PlotId);
		if (Plot)
		{
			Info += FString::Printf(TEXT("  %s — %.0f m² [%s]\n"),
				*Plot->PlotAddress, Plot->Area,
				Plot->bIsAvailable ? TEXT("AVAILABLE") : TEXT("OCCUPIED"));
		}
	}
	return Info;
}

bool FWorldManager::PlaceBuilding(FEntityId BuildingId, FEntityId PlotId)
{
	auto* Plot = ECS.GetComponent<C_Plot>(PlotId);
	if (!Plot || !Plot->bIsAvailable) return false;
	
	Plot->bIsAvailable = false;
	Plot->bHasBuilding = true;
	Plot->BuildingId = BuildingId;
	
	IB_LOG_INFO("Building placed on plot: %s", *Plot->PlotAddress);
	return true;
}

TArray<FEntityId> FWorldManager::GetAvailablePlots() const
{
	TArray<FEntityId> Available;
	for (auto& PlotId : Plots)
	{
		auto* Plot = ECS.GetComponent<C_Plot>(PlotId);
		if (Plot && Plot->bIsAvailable) Available.Add(PlotId);
	}
	return Available;
}

FString FWorldManager::GetPlotDetails(FEntityId PlotId) const
{
	auto* Plot = ECS.GetComponent<C_Plot>(PlotId);
	if (!Plot) return TEXT("Invalid plot");
	
	return FString::Printf(
		TEXT("PLOT: %s\n  Size: %.0f×%.0f (%.0f m²)\n  Soil: %s | Utilities: %s\n  Available: %s"),
		*Plot->PlotAddress, Plot->SizeX, Plot->SizeY, Plot->Area, *Plot->SoilType,
		Plot->bHasWater && Plot->bHasElectricity ? TEXT("All") : TEXT("Partial"),
		Plot->bIsAvailable ? TEXT("Yes") : TEXT("No"));
}

// ============================================================
// VERTICAL SLICE ASSEMBLER (M7)
// ============================================================

FVerticalSliceAssembler::FVerticalSliceAssembler()
	: FInstaBuiltSystem(TEXT("VerticalSliceAssembler"))
{
	AddDependency(TEXT("GameManager"));
	AddDependency(TEXT("CompanyManager"));
	AddDependency(TEXT("BuildingDesigner"));
	AddDependency(TEXT("WorldManager"));
}

void FVerticalSliceAssembler::OnInitialize()
{
	IB_LOG_INFO("VerticalSliceAssembler ready. Use 'vs_start' to begin the demo.");
}

FString FVerticalSliceAssembler::Cmd_StartVerticalSlice()
{
	bSliceStarted = true;
	CurrentStep = 0;
	
	IB_LOG_INFO("=== VERTICAL SLICE DEMO STARTED ===");
	
	FString Output;
	Output += TEXT("══════════════════════════════════════\n");
	Output += TEXT("  INSTABUILT: BLUEPRINT EMPIRE\n");
	Output += TEXT("  Vertical Slice Demo\n");
	Output += TEXT("══════════════════════════════════════\n\n");
	
	// Execute first step
	Output += ExecuteStep(EStep::CreateCompany);
	
	return Output;
}

FString FVerticalSliceAssembler::Cmd_RunDemo()
{
	FString Output = Cmd_StartVerticalSlice();
	
	// Auto-advance through all steps
	while (CurrentStep < (int32)EStep::Complete)
	{
		AdvanceStep();
		Output += ExecuteStep(GetStepEnum());
	}
	
	return Output;
}

FString FVerticalSliceAssembler::ExecuteStep(EStep Step)
{
	auto GM = FSystemOrchestrator::Get().GetSystem<FInstaBuiltGameManager>(TEXT("GameManager"));
	auto CM = FSystemOrchestrator::Get().GetSystem<FCompanyManager>(TEXT("CompanyManager"));
	auto Designer = FSystemOrchestrator::Get().GetSystem<FBuildingDesigner>(TEXT("BuildingDesigner"));
	auto World = FSystemOrchestrator::Get().GetSystem<FWorldManager>(TEXT("WorldManager"));
	
	FString Result;
	
	switch (Step)
	{
	case EStep::CreateCompany:
		GM->Cmd_NewGame(TEXT("InstaBuilt Premium Builders"));
		CM->HireEmployee(TEXT("Marku Dervishi"), TEXT("Site Supervisor"), 55000.0);
		CM->HireEmployee(TEXT("Arta Hoxha"), TEXT("Architect"), 65000.0);
		CM->HireEmployee(TEXT("Ben Kelmendi"), TEXT("Skilled Carpenter"), 48000.0);
		World->CreateShowcaseDistrict();
		Result = TEXT("✅ Company created: InstaBuilt Premium Builders\n")
			TEXT("   Location: Riverside District\n")
			TEXT("   Starting funds: $250,000\n")
			TEXT("   Employees: 3\n\n");
		break;
		
	case EStep::ReceiveContract:
		GM->Cmd_AcceptContract(0);
		Result = TEXT("✅ Premium residential contract received\n")
			TEXT("   Client: Johnson Family\n")
			TEXT("   Type: Traditional Home\n")
			TEXT("   Budget: $95,000\n\n");
		break;
		
	case EStep::ReviewRequirements:
		Result = TEXT("✅ Requirements reviewed:\n")
			TEXT("   • 3+ bedrooms\n")
			TEXT("   • 2 bathrooms\n")
			TEXT("   • Open-plan living/kitchen\n")
			TEXT("   • KfW 40 energy standard\n")
			TEXT("   • Premium materials requested\n\n");
		break;
		
	case EStep::DesignBuilding:
		Designer->CreateDesign(TEXT("Johnson Residence"), TEXT("TRADITIONAL_HOME"));
		Designer->AddRoom(TEXT("Living Room"), TEXT("Living"), 0, 0, 7, 6);
		Designer->AddRoom(TEXT("Kitchen"), TEXT("Kitchen"), 7, 0, 5, 6);
		Designer->AddRoom(TEXT("Master Bedroom"), TEXT("Bedroom"), 0, 6, 6, 5);
		Designer->AddRoom(TEXT("Bedroom 2"), TEXT("Bedroom"), 6, 6, 4, 5);
		Designer->AddRoom(TEXT("Bathroom 1"), TEXT("Bathroom"), 10, 6, 3, 5);
		Designer->AddRoom(TEXT("Bathroom 2"), TEXT("Bathroom"), 0, 11, 5, 4);
		Designer->SetMaterialTier(EMaterialTier::Premium);
		Result = FString::Printf(TEXT("✅ Building designed:\n%s\n\n"),
			*Designer->GetBlueprintSummary());
		break;
		
	case EStep::SubmitBlueprint:
		Designer->ApproveDesign();
		Result = TEXT("✅ Blueprint submitted for approval\n")
			TEXT("   Estimated cost: $168,000\n")
			TEXT("   Premium materials selected\n\n");
		break;
		
	case EStep::PrepareConstruction:
		GM->Cmd_StartBuilding();
		CM->CreateProjectFinance(FEntityId(), 168000.0);
		CM->RecordProjectCost(FEntityId(), TEXT("Permit"), 3500.0);
		CM->RecordProjectCost(FEntityId(), TEXT("Material"), 85000.0);
		Result = TEXT("✅ Construction prepared:\n")
			TEXT("   Permits: Approved\n")
			TEXT("   Materials: Ordered ($85,000)\n")
			TEXT("   Site: Ready\n\n");
		break;
		
	case EStep::AssignWorkers:
		GM->Cmd_HireWorkers();
		Result = TEXT("✅ Workers assigned:\n")
			TEXT("   • Marku Dervishi — Site Supervisor\n")
			TEXT("   • Arta Hoxha — Architect oversight\n")
			TEXT("   • Ben Kelmendi — Carpentry lead\n")
			TEXT("   Construction speed: 2.5x\n\n");
		break;
		
	case EStep::MonitorConstruction:
		{
			// Simulate construction progress
			for (int32 i = 0; i < 120; ++i)
				FSystemOrchestrator::Get().UpdateAll(0.5f);
			
			// Record costs every few phases
			CM->RecordProjectCost(FEntityId(), TEXT("Labor"), 42000.0);
			CM->RecordProjectCost(FEntityId(), TEXT("Equipment"), 12000.0);
			
			Result = TEXT("✅ Construction monitoring:\n")
				TEXT("   Foundation: ✅ Complete\n")
				TEXT("   Structure:  ✅ Complete\n")
				TEXT("   Interior:   ✅ Complete\n")
				TEXT("   Final:      ✅ Complete\n")
				TEXT("   Quality:    94%\n\n");
		}
		break;
		
	case EStep::InspectComplete:
		Result = TEXT("✅ Final inspection passed:\n")
			TEXT("   • Structural integrity: Pass\n")
			TEXT("   • Electrical systems: Pass\n")
			TEXT("   • Plumbing: Pass\n")
			TEXT("   • Energy efficiency: KfW 40 Certified\n")
			TEXT("   Defects: 0 major, 2 minor (fixed on-site)\n\n");
		break;
		
	case EStep::ClientApproval:
		Result = TEXT("✅ Client walkthrough complete:\n")
			TEXT("   Johnson Family: 'It's exactly what we dreamed of.'\n")
			TEXT("   Satisfaction: 96%\n")
			TEXT("   Would recommend: Yes\n\n");
		break;
		
	case EStep::ReceivePayment:
		Result = TEXT("✅ Payment received:\n")
			TEXT("   Contract value: $95,000\n")
			TEXT("   Quality bonus: +$4,750 (5%)\n")
			TEXT("   On-time bonus: +$9,500 (10%)\n")
			TEXT("   Total: $109,250\n")
			TEXT("   Project profit: $24,750\n\n");
		break;
		
	case EStep::ReputationIncrease:
		CM->UpdateReputation(94.0f, 95.0f, 70.0f, 65.0f, 90.0f);
		Result = FString::Printf(TEXT("✅ Reputation updated:\n%s\n\n"),
			*CM->GetReputationDashboard());
		break;
		
	case EStep::SaveGame:
		GM->SaveGame(TEXT("VerticalSliceComplete"));
		Result = TEXT("✅ Game saved\n\n");
		break;
		
	case EStep::Complete:
		Result = TEXT("══════════════════════════════════════\n");
		Result += TEXT("  VERTICAL SLICE COMPLETE\n");
		Result += TEXT("  InstaBuilt: Blueprint Empire\n");
		Result += TEXT("══════════════════════════════════════\n");
		break;
	}
	
	return Result;
}

void FVerticalSliceAssembler::AdvanceStep()
{
	CurrentStep++;
}

FString FVerticalSliceAssembler::GetJourneyStatus() const
{
	return FString::Printf(TEXT("Step %d/13: %s"),
		CurrentStep + 1, *StepToString(GetStepEnum()));
}

FString FVerticalSliceAssembler::GetWalkthrough() const
{
	return FString(
		TEXT("VERTICAL SLICE PLAYER JOURNEY:\n")
		TEXT("  1. Create company — InstaBuilt Premium Builders\n")
		TEXT("  2. Receive contract — Johnson Family Residence\n")
		TEXT("  3. Review requirements — 3BR, 2BA, Premium\n")
		TEXT("  4. Design building — Room placement, materials\n")
		TEXT("  5. Submit blueprint — Validation + approval\n")
		TEXT("  6. Prepare construction — Permits, materials\n")
		TEXT("  7. Assign workers — 3-person crew\n")
		TEXT("  8. Monitor construction — 4 phases, quality tracking\n")
		TEXT("  9. Inspect complete — Passed, minor fixes\n")
		TEXT(" 10. Client approval — 96% satisfaction\n")
		TEXT(" 11. Receive payment — $109,250\n")
		TEXT(" 12. Reputation increase — All axes improved\n")
		TEXT(" 13. Save game — VerticalSliceComplete.ibsave\n")
	);
}

FString FVerticalSliceAssembler::StepToString(EStep Step) const
{
	switch (Step)
	{
	case EStep::CreateCompany:       return TEXT("Create Company");
	case EStep::ReceiveContract:     return TEXT("Receive Contract");
	case EStep::ReviewRequirements:  return TEXT("Review Requirements");
	case EStep::DesignBuilding:      return TEXT("Design Building");
	case EStep::SubmitBlueprint:     return TEXT("Submit Blueprint");
	case EStep::PrepareConstruction: return TEXT("Prepare Construction");
	case EStep::AssignWorkers:       return TEXT("Assign Workers");
	case EStep::MonitorConstruction: return TEXT("Monitor Construction");
	case EStep::InspectComplete:     return TEXT("Inspect Complete");
	case EStep::ClientApproval:      return TEXT("Client Approval");
	case EStep::ReceivePayment:      return TEXT("Receive Payment");
	case EStep::ReputationIncrease:  return TEXT("Reputation Increase");
	case EStep::SaveGame:            return TEXT("Save Game");
	case EStep::Complete:            return TEXT("COMPLETE");
	default: return TEXT("Unknown");
	}
}
