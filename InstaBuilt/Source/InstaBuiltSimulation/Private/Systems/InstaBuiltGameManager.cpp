// InstaBuiltGameManager.cpp — Complete game loop (M10-M12)
// Wires all systems. Drives: New Game → Contract → Build → Complete → Reward

#include "Systems/InstaBuiltGameManager.h"
#include "Systems/InstaBuiltEvents.h"
#include "Logging/InstaBuiltLog.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFileManager.h"

FInstaBuiltGameManager::FInstaBuiltGameManager()
	: FInstaBuiltSystem(TEXT("GameManager"))
{
	// Depends on all gameplay systems
	AddDependency(TEXT("CompanySystem"));
	AddDependency(TEXT("ContractSystem"));
	AddDependency(TEXT("BuildingSystem"));
	AddDependency(TEXT("ConstructionSystem"));
	AddDependency(TEXT("WorkerSystem"));
	AddDependency(TEXT("EconomySystem"));
}

void FInstaBuiltGameManager::OnInitialize()
{
	ResolveSystemReferences();
	SubscribeToEvents();
	IB_LOG_INFO("GameManager initialized. All systems linked.");
}

void FInstaBuiltGameManager::ResolveSystemReferences()
{
	auto& Orch = FSystemOrchestrator::Get();
	CompanySys = Orch.GetSystem<FCompanySystem>(TEXT("CompanySystem")).Get();
	ContractSys = Orch.GetSystem<FContractSystem>(TEXT("ContractSystem")).Get();
	BuildingSys = Orch.GetSystem<FBuildingSystem>(TEXT("BuildingSystem")).Get();
	ConstructionSys = Orch.GetSystem<FConstructionSystem>(TEXT("ConstructionSystem")).Get();
	WorkerSys = Orch.GetSystem<FWorkerSystem>(TEXT("WorkerSystem")).Get();
	EconomySys = Orch.GetSystem<FEconomySystem>(TEXT("EconomySystem")).Get();
	
	if (EconomySys) EconomySys->SetCompanySystem(CompanySys);
	
	IB_LOG_INFO("System references resolved. Company:%s Contract:%s Building:%s Construction:%s Worker:%s Economy:%s",
		CompanySys ? TEXT("✓") : TEXT("✗"),
		ContractSys ? TEXT("✓") : TEXT("✗"),
		BuildingSys ? TEXT("✓") : TEXT("✗"),
		ConstructionSys ? TEXT("✓") : TEXT("✗"),
		WorkerSys ? TEXT("✓") : TEXT("✗"),
		EconomySys ? TEXT("✓") : TEXT("✗"));
}

void FInstaBuiltGameManager::SubscribeToEvents()
{
	EventBus.Subscribe<FPhaseCompletedEvent>(
		FEventCallback::CreateRaw(this, &FInstaBuiltGameManager::OnConstructionPhaseCompleted));
	EventBus.Subscribe<FConstructionCompletedEvent>(
		FEventCallback::CreateRaw(this, &FInstaBuiltGameManager::OnConstructionCompleted));
	EventBus.Subscribe<FContractCompletedEvent>(
		FEventCallback::CreateRaw(this, &FInstaBuiltGameManager::OnContractCompleted));
}

void FInstaBuiltGameManager::OnUpdate(float DeltaTime) {}

// ============================================================
// GAME FLOW
// ============================================================

void FInstaBuiltGameManager::Cmd_NewGame(const FString& CompanyName)
{
	if (bGameStarted)
	{
		IB_LOG_WARN("Game already started. Ignoring Cmd_NewGame.");
		return;
	}
	
	IB_LOG_INFO("=== STARTING NEW GAME: %s ===", *CompanyName);
	
	// 1. Create company
	CompanySys->CreateCompany(CompanyName, 250000.0);
	
	// 2. Generate starter contracts
	ContractSys->GenerateStarterContracts();
	
	// 3. Ready
	bGameStarted = true;
	bDirty = true;
	
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("Welcome to %s! Check the dashboard for contracts."), *CompanyName),
		TEXT("Success"));
	
	CompanySys->PrintStatus();
}

FString FInstaBuiltGameManager::Cmd_AcceptContract(int32 ContractIndex)
{
	if (!bGameStarted) return TEXT("Game not started. Use 'newgame' first.");
	
	auto Available = ContractSys->GetAvailableContracts();
	if (ContractIndex < 0 || ContractIndex >= Available.Num())
	{
		return FString::Printf(TEXT("Invalid contract index. Available: 0-%d"), Available.Num() - 1);
	}
	
	FEntityId ContractId = Available[ContractIndex];
	auto* Contract = FInstaBuiltECS::Get().GetComponent<C_ContractData>(ContractId);
	if (!Contract) return TEXT("Contract data missing.");
	
	// Accept at budget mid-point
	double Bid = (Contract->BudgetMin + Contract->BudgetMax) / 2.0;
	ContractSys->AcceptContract(ContractId, Bid);
	ActiveContractId = ContractId;
	bDirty = true;
	
	// 2. Create building
	ActiveBuildingId = BuildingSys->CreateBuilding(
		Contract->ContractName + TEXT(" Building"),
		Contract->BuildingType,
		Contract->RequiredArea,
		1);
	
	return FString::Printf(TEXT("Contract accepted: %s ($%.0f). Use 'startbuild' to begin construction."),
		*Contract->ContractName, Bid);
}

FString FInstaBuiltGameManager::Cmd_StartBuilding()
{
	if (!ActiveContractId.IsValid()) return TEXT("No active contract. Accept one first.");
	if (ActiveSiteId.IsValid()) return TEXT("Construction already in progress.");
	
	// 1. Activate contract
	ContractSys->ActivateContract(ActiveContractId);
	
	// 2. Start construction
	ActiveSiteId = ConstructionSys->StartConstruction(ActiveBuildingId, ActiveContractId);
	bDirty = true;
	
	return TEXT("Construction started! Watch the progress...");
}

FString FInstaBuiltGameManager::Cmd_HireWorkers()
{
	if (!WorkerSys) return TEXT("Worker system not available.");
	
	auto* Finances = FInstaBuiltECS::Get().GetComponent<C_Financials>(CompanySys->GetCompanyId());
	if (Finances && Finances->CashOnHand < 10000)
		return TEXT("Not enough money to hire workers.");
	
	FEntityId W1 = WorkerSys->HireWorker(TEXT("Marku"), EWorkerRole::Laborer, 22.0f);
	FEntityId W2 = WorkerSys->HireWorker(TEXT("Arta"), EWorkerRole::Carpenter, 28.0f);
	
	// Assign to active site if exists
	if (ActiveSiteId.IsValid())
	{
		WorkerSys->AssignToSite(W1, ActiveSiteId);
		WorkerSys->AssignToSite(W2, ActiveSiteId);
		ConstructionSys->AssignWorkers(ActiveSiteId, 2);
	}
	
	bDirty = true;
	return FString::Printf(TEXT("Hired 2 workers: Marku ($22/hr) and Arta ($28/hr)"));
}

// ============================================================
// EVENT HANDLERS
// ============================================================

void FInstaBuiltGameManager::OnConstructionPhaseCompleted(const FGameEvent& Event)
{
	const auto& E = static_cast<const FPhaseCompletedEvent&>(Event);
	IB_LOG_INFO("Phase completed: %s (Site: %s)", *E.PhaseName, *E.SiteId.ToString());
}

void FInstaBuiltGameManager::OnConstructionCompleted(const FGameEvent& Event)
{
	const auto& E = static_cast<const FConstructionCompletedEvent&>(Event);
	IB_LOG_INFO("Construction finished! Building ready for inspection.");
	
	CompleteActiveProject();
}

void FInstaBuiltGameManager::OnContractCompleted(const FGameEvent& Event)
{
	const auto& E = static_cast<const FContractCompletedEvent&>(Event);
	IB_LOG_INFO("Contract paid: $%.0f", E.PaymentAmount);
	
	bDirty = true;
}

bool FInstaBuiltGameManager::CompleteActiveProject()
{
	if (!ActiveContractId.IsValid() || !ActiveSiteId.IsValid()) return false;
	
	auto* State = FInstaBuiltECS::Get().GetComponent<C_ConstructionState>(ActiveSiteId);
	float Quality = State ? State->QualityScore : 85.0f;
	bool bOnTime = true; // Simplified for prototype
	
	// 1. Complete contract (triggers payment)
	ContractSys->CompleteContract(ActiveContractId, (int32)Quality, bOnTime);
	
	// 2. Process payment through economy
	auto* Contract = FInstaBuiltECS::Get().GetComponent<C_ContractData>(ActiveContractId);
	double Payment = Contract ? Contract->Reward : 100000.0;
	EconomySys->ProcessContractPayment(ActiveContractId, Payment);
	
	// 3. Update reputation
	CompanySys->AddProjectReputation(Quality, bOnTime);
	
	// 4. Generate new contracts
	ContractSys->GenerateStarterContracts();
	
	// 5. Reset active state
	ActiveContractId = FEntityId::Invalid();
	ActiveBuildingId = FEntityId::Invalid();
	ActiveSiteId = FEntityId::Invalid();
	
	CompanySys->PrintStatus();
	EventBus.Publish<FNotificationEvent>(TEXT("Project complete! New contracts available."), TEXT("Success"));
	
	return true;
}

// ============================================================
// STATUS / UI DATA
// ============================================================

FString FInstaBuiltGameManager::GetGameStatus() const
{
	FString Status;
	
	if (!bGameStarted)
	{
		Status = TEXT("No active game. Use 'newgame' to start.");
	}
	else if (!ActiveContractId.IsValid())
	{
		Status = TEXT("No active contract. Browse available contracts and accept one.");
	}
	else if (!ActiveSiteId.IsValid())
	{
		auto* Contract = FInstaBuiltECS::Get().GetComponent<C_ContractData>(ActiveContractId);
		Status = Contract
			? FString::Printf(TEXT("Contract accepted: %s. Ready to build. Use 'startbuild'."), *Contract->ContractName)
			: TEXT("Contract accepted. Ready to build.");
	}
	else
	{
		FString Phase; float PhaseProg, Overall, Quality; bool Paused;
		ConstructionSys->GetProgressInfo(ActiveSiteId, Phase, PhaseProg, Overall, Quality, Paused);
		
		FString ProgressBar;
		int32 Bars = FMath::RoundToInt(Overall * 20.0f);
		for (int32 i = 0; i < 20; ++i) ProgressBar += (i < Bars) ? TEXT("█") : TEXT("░");
		
		Status = FString::Printf(TEXT("Building: %s | Phase: %s (%.0f%%) | Overall: %s %.0f%% | Workers: %d"),
			Phase == TEXT("Complete") ? TEXT("COMPLETE!") : TEXT("In Progress"),
			*Phase, PhaseProg * 100.0f,
			*ProgressBar, Overall * 100.0f,
			FInstaBuiltECS::Get().GetComponent<C_ConstructionSite>(ActiveSiteId)
				? FInstaBuiltECS::Get().GetComponent<C_ConstructionSite>(ActiveSiteId)->AssignedWorkerCount : 0);
	}
	
	return Status;
}

FString FInstaBuiltGameManager::GetDashboardReport() const
{
	FString Report;
	
	// Company status
	FString Name, Tier, Cash, Rep, Projects;
	CompanySys->GetDashboardData(Name, Tier, Cash, Rep, Projects);
	
	Report += TEXT("═══════════════════════════════════\n");
	Report += FString::Printf(TEXT("  %s — %s\n"), *Name, *Tier);
	Report += FString::Printf(TEXT("  Cash: %s | Reputation: %s\n"), *Cash, *Rep);
	Report += FString::Printf(TEXT("  Projects Completed: %s\n"), *Projects);
	Report += TEXT("═══════════════════════════════════\n");
	
	// Available contracts
	auto Available = ContractSys->GetAvailableContracts();
	Report += FString::Printf(TEXT("\nAVAILABLE CONTRACTS (%d):\n"), Available.Num());
	for (int32 i = 0; i < Available.Num(); ++i)
	{
		Report += FString::Printf(TEXT("  [%d] %s\n"), i,
			*ContractSys->GetContractDetails(Available[i]));
	}
	
	// Workers
	auto Workers = WorkerSys->GetAllWorkers();
	Report += FString::Printf(TEXT("\nWORKERS (%d):\n"), Workers.Num());
	for (auto& W : Workers)
	{
		Report += FString::Printf(TEXT("  %s\n"), *WorkerSys->GetWorkerInfo(W));
	}
	
	// Construction status
	Report += TEXT("\nCONSTRUCTION STATUS:\n");
	Report += FString::Printf(TEXT("  %s\n"), *GetGameStatus());
	
	return Report;
}

void FInstaBuiltGameManager::PrintFullStatus() const
{
	IB_LOG_INFO("%s", *GetDashboardReport());
}

// ============================================================
// SAVE/LOAD (M11)
// ============================================================

FString FInstaBuiltGameManager::GetSaveDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Saves");
}

FString FInstaBuiltGameManager::GetSaveFilePath(const FString& SlotName) const
{
	return GetSaveDirectory() / (SlotName + TEXT(".ibsave"));
}

bool FInstaBuiltGameManager::SaveGame(const FString& SlotName)
{
	FString Dir = GetSaveDirectory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Dir))
	{
		PlatformFile.CreateDirectory(*Dir);
	}
	
	// Build JSON save data
	TSharedPtr<FJsonObject> SaveData = MakeShared<FJsonObject>();
	SaveData->SetNumberField("SaveVersion", 1);
	SaveData->SetStringField("CompanyName", TEXT("InstaBuilt"));
	
	// Save company financials
	auto* Finances = FInstaBuiltECS::Get().GetComponent<C_Financials>(CompanySys->GetCompanyId());
	if (Finances)
	{
		SaveData->SetNumberField("CashOnHand", Finances->CashOnHand);
		SaveData->SetNumberField("TotalProfit", Finances->TotalProfit);
		SaveData->SetNumberField("ProjectsCompleted", Finances->ProjectsCompleted);
	}
	
	// Save reputation
	auto* Rep = CompanySys->GetReputation();
	if (Rep)
	{
		SaveData->SetNumberField("Reputation", Rep->OverallRating);
	}
	
	// Serialize to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(SaveData.ToSharedRef(), Writer);
	
	// Write to file
	FString FilePath = GetSaveFilePath(SlotName);
	if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		bDirty = false;
		IB_LOG_INFO("Game saved to: %s", *FilePath);
		EventBus.Publish<FNotificationEvent>(TEXT("Game saved."), TEXT("Info"));
		return true;
	}
	
	IB_LOG_ERROR("Failed to save game to: %s", *FilePath);
	return false;
}

bool FInstaBuiltGameManager::LoadGame(const FString& SlotName)
{
	FString FilePath = GetSaveFilePath(SlotName);
	FString JsonString;
	
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		IB_LOG_ERROR("Failed to load save file: %s", *FilePath);
		return false;
	}
	
	TSharedPtr<FJsonObject> SaveData;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, SaveData) || !SaveData.IsValid())
	{
		IB_LOG_ERROR("Failed to parse save file.");
		return false;
	}
	
	int32 SaveVersion = SaveData->GetIntegerField("SaveVersion");
	IB_LOG_INFO("Loading save v%d from: %s", SaveVersion, *FilePath);
	
	// Restore state
	double Cash = SaveData->GetNumberField("CashOnHand");
	FString Name = SaveData->GetStringField("CompanyName");
	
	// Create new game with restored data
	Cmd_NewGame(Name);
	
	auto* Finances = FInstaBuiltECS::Get().GetComponent<C_Financials>(CompanySys->GetCompanyId());
	if (Finances)
	{
		Finances->CashOnHand = Cash;
		Finances->TotalProfit = SaveData->GetNumberField("TotalProfit");
		Finances->ProjectsCompleted = SaveData->GetIntegerField("ProjectsCompleted");
	}
	
	bDirty = false;
	EventBus.Publish<FNotificationEvent>(TEXT("Game loaded."), TEXT("Success"));
	IB_LOG_INFO("Game loaded successfully.");
	return true;
}

TArray<FString> FInstaBuiltGameManager::GetSaveSlots() const
{
	TArray<FString> Slots;
	FString Dir = GetSaveDirectory();
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	
	if (PF.DirectoryExists(*Dir))
	{
		PF.FindFiles(Slots, *Dir, TEXT(".ibsave"));
		for (auto& S : Slots) S = FPaths::GetBaseFilename(S);
	}
	
	return Slots;
}
