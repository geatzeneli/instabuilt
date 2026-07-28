// BuildingSystem.h — Building creation + ConstructionSystem.h — Phase progression
// M6 + M7 combined: buildings, construction phases, progress tracking

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/CompanyComponents.h"

// ============================================================
// BUILDING SYSTEM (M6)
// ============================================================

class FBuildingSystem : public FInstaBuiltSystem
{
public:
	FBuildingSystem() : FInstaBuiltSystem(TEXT("BuildingSystem")) {}
	
	FEntityId CreateBuilding(const FString& Name, const FString& Type, float Area, int32 Floors);
	void GetBuildingInfo(FEntityId BuildingId, FString& OutName, FString& OutType, float& OutArea) const;
};

// ============================================================
// CONSTRUCTION SYSTEM (M7)
// ============================================================

class FConstructionSystem : public FInstaBuiltSystem
{
public:
	FConstructionSystem();
	
	virtual void OnInitialize() override;
	virtual void OnUpdate(float DeltaTime) override;
	
	/** Start construction on a building for a contract */
	FEntityId StartConstruction(FEntityId BuildingId, FEntityId ContractId);
	
	/** Get progress info for UI */
	void GetProgressInfo(FEntityId SiteId, FString& OutPhase, float& OutPhaseProgress,
		float& OutOverall, float& OutQuality, bool& OutPaused) const;
	
	/** Get all active construction sites */
	TArray<FEntityId> GetActiveSites() const;
	
	/** Is the site complete? */
	bool IsSiteComplete(FEntityId SiteId) const;
	
	/** Add workers to a construction site */
	void AssignWorkers(FEntityId SiteId, int32 Count);
	
	/** Get phase name string */
	static FString PhaseToString(EConstructionPhase Phase);
	
private:
	TArray<FEntityId> ActiveSites;
	
	void AdvanceConstruction(FEntityId SiteId, float DeltaTime);
	void CompletePhase(FEntityId SiteId);
	void CompleteConstruction(FEntityId SiteId);
	
	// Phase durations in seconds (scaled for prototype — real game uses in-game days)
	static constexpr float PhaseDurations[4] = { 10.0f, 15.0f, 12.0f, 8.0f };
	// Foundation=10s, Structure=15s, Interior=12s, Finalization=8s
	
	static constexpr const TCHAR* PhaseNames[4] = {
		TEXT("Foundation"), TEXT("Structure"), TEXT("Interior"), TEXT("Finalization")
	};
};
