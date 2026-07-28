// InstaBuiltGameManager.h — Central gameplay orchestrator (M10-M12)
// Wires: Company → Contract → Building → Construction → Worker → Economy → Save
// GSS Section 4: Complete Game Flow

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Systems/CompanySystem.h"
#include "Systems/ContractSystem.h"
#include "Systems/BuildingSystem.h"
#include "Systems/WorkerEconomy.h"

class FConstructionSystem;

/**
 * FInstaBuiltGameManager
 * 
 * The master system that orchestrates the complete gameplay loop.
 * All other systems are peers; this one coordinates them.
 * 
 * This is the "game director" — it knows the flow:
 *   Start → Generate Contracts → Accept → Design → Build → Complete → Reward → Repeat
 */
class INSTABUILTSIMULATION_API FInstaBuiltGameManager : public FInstaBuiltSystem
{
public:
	FInstaBuiltGameManager();
	
	virtual void OnInitialize() override;
	virtual void OnUpdate(float DeltaTime) override;
	
	// ============================================================
	// GAME FLOW COMMANDS (called by UI or console)
	// ============================================================
	
	/** Start a new game: create company, generate starter contracts */
	void Cmd_NewGame(const FString& CompanyName);
	
	/** Accept a contract by index (0-based from available list) */
	FString Cmd_AcceptContract(int32 ContractIndex);
	
	/** Start building the accepted contract */
	FString Cmd_StartBuilding();
	
	/** Hire default workers */
	FString Cmd_HireWorkers();
	
	/** Get current game status for UI display */
	FString GetGameStatus() const;
	
	/** Get the full dashboard report */
	FString GetDashboardReport() const;
	
	/** Print full status to log */
	void PrintFullStatus() const;
	
	// ============================================================
	// SYSTEM ACCESS
	// ============================================================
	
	FCompanySystem* GetCompany() const { return CompanySys; }
	FContractSystem* GetContracts() const { return ContractSys; }
	FBuildingSystem* GetBuildings() const { return BuildingSys; }
	FConstructionSystem* GetConstruction() const { return ConstructionSys; }
	FWorkerSystem* GetWorkers() const { return WorkerSys; }
	FEconomySystem* GetEconomy() const { return EconomySys; }
	
	// ============================================================
	// SAVE/LOAD (M11)
	// ============================================================
	
	/** Save current game state to file */
	bool SaveGame(const FString& SlotName);
	
	/** Load game state from file */
	bool LoadGame(const FString& SlotName);
	
	/** Get list of save slots */
	TArray<FString> GetSaveSlots() const;
	
	/** Is there unsaved progress? */
	bool HasUnsavedProgress() const { return bDirty; }
	
private:
	// System references (set during OnInitialize)
	FCompanySystem* CompanySys = nullptr;
	FContractSystem* ContractSys = nullptr;
	FBuildingSystem* BuildingSys = nullptr;
	FConstructionSystem* ConstructionSys = nullptr;
	FWorkerSystem* WorkerSys = nullptr;
	FEconomySystem* EconomySys = nullptr;
	
	// Current game state
	FEntityId ActiveContractId;
	FEntityId ActiveBuildingId;
	FEntityId ActiveSiteId;
	bool bGameStarted = false;
	bool bDirty = false;
	
	// Save directory
	FString GetSaveDirectory() const;
	FString GetSaveFilePath(const FString& SlotName) const;
	
	// Event handlers
	void OnConstructionPhaseCompleted(const FGameEvent& Event);
	void OnConstructionCompleted(const FGameEvent& Event);
	void OnContractCompleted(const FGameEvent& Event);
	
	// Internal helpers
	void ResolveSystemReferences();
	void SubscribeToEvents();
	bool CompleteActiveProject();
};
