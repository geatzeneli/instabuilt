// BuildingSystem.cpp — M6+M7: Building creation + Construction simulation

#include "Systems/BuildingSystem.h"
#include "Systems/InstaBuiltEvents.h"
#include "Logging/InstaBuiltLog.h"

// ============================================================
// BUILDING SYSTEM
// ============================================================

FEntityId FBuildingSystem::CreateBuilding(const FString& Name, const FString& Type, float Area, int32 Floors)
{
	FEntityId BuildingId = ECS.CreateEntity(EEntityType::Building);
	
	auto* Data = ECS.AddComponent<C_BuildingData>(BuildingId);
	Data->BuildingName = Name;
	Data->BuildingType = Type;
	Data->TotalArea = Area;
	Data->FootprintArea = Area;
	Data->FloorCount = Floors;
	
	IB_LOG_INFO("Building created: %s (%s, %.0f m², %d floors)", *Name, *Type, Area, Floors);
	return BuildingId;
}

void FBuildingSystem::GetBuildingInfo(FEntityId BuildingId, FString& OutName, FString& OutType, float& OutArea) const
{
	auto* Data = ECS.GetComponent<C_BuildingData>(BuildingId);
	OutName = Data ? Data->BuildingName : TEXT("Unknown");
	OutType = Data ? Data->BuildingType : TEXT("Unknown");
	OutArea = Data ? Data->TotalArea : 0.0f;
}

// ============================================================
// CONSTRUCTION SYSTEM
// ============================================================

FConstructionSystem::FConstructionSystem()
	: FInstaBuiltSystem(TEXT("ConstructionSystem"))
{
	AddDependency(TEXT("BuildingSystem"));
	SetUpdateInterval(0.5f); // Update every 0.5s for visible progress
}

void FConstructionSystem::OnInitialize()
{
	IB_LOG_INFO("ConstructionSystem initialized. Phase durations: %.0fs each", PhaseDurations[0]);
}

void FConstructionSystem::OnUpdate(float DeltaTime)
{
	for (auto& SiteId : ActiveSites)
	{
		AdvanceConstruction(SiteId, DeltaTime);
	}
}

FEntityId FConstructionSystem::StartConstruction(FEntityId BuildingId, FEntityId ContractId)
{
	FEntityId SiteId = ECS.CreateEntity(EEntityType::ConstructionSite);
	
	auto* State = ECS.AddComponent<C_ConstructionState>(SiteId);
	State->CurrentPhase = EConstructionPhase::Foundation;
	State->PhaseProgress = 0.0f;
	State->OverallProgress = 0.0f;
	State->ContractId = ContractId;
	
	auto* Site = ECS.AddComponent<C_ConstructionSite>(SiteId);
	Site->BuildingId = BuildingId;
	Site->ContractId = ContractId;
	Site->BuildSpeed = 1.0f;
	
	ActiveSites.Add(SiteId);
	
	EventBus.Publish<FConstructionStartedEvent>(SiteId, BuildingId, ContractId);
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("Construction started! Phase: Foundation")), TEXT("Info"));
	
	IB_LOG_INFO("Construction started on site %s", *SiteId.ToString());
	return SiteId;
}

void FConstructionSystem::AdvanceConstruction(FEntityId SiteId, float DeltaTime)
{
	auto* State = ECS.GetComponent<C_ConstructionState>(SiteId);
	auto* Site = ECS.GetComponent<C_ConstructionSite>(SiteId);
	if (!State || !Site || State->bIsPaused) return;
	
	int32 PhaseIndex = (int32)State->CurrentPhase - 1; // Foundation=0
	if (PhaseIndex < 0 || PhaseIndex >= 4) return;
	
	// Progress = time × buildSpeed / phaseDuration
	float PhaseDuration = PhaseDurations[PhaseIndex];
	float Speed = Site->BuildSpeed;
	State->PhaseProgress += (DeltaTime * Speed) / PhaseDuration;
	
	// Update overall progress (4 phases, each 25%)
	State->OverallProgress = (PhaseIndex * 0.25f) + (State->PhaseProgress * 0.25f);
	
	if (State->PhaseProgress >= 1.0f)
	{
		CompletePhase(SiteId);
	}
}

void FConstructionSystem::CompletePhase(FEntityId SiteId)
{
	auto* State = ECS.GetComponent<C_ConstructionState>(SiteId);
	if (!State) return;
	
	FString OldPhaseName = PhaseToString(State->CurrentPhase);
	
	// Move to next phase
	int32 NextPhase = (int32)State->CurrentPhase + 1;
	
	if (NextPhase > (int32)EConstructionPhase::Finalization)
	{
		// Building complete!
		CompleteConstruction(SiteId);
		return;
	}
	
	State->CurrentPhase = (EConstructionPhase)NextPhase;
	State->PhaseProgress = 0.0f;
	
	FString NewPhaseName = PhaseToString(State->CurrentPhase);
	
	EventBus.Publish<FPhaseCompletedEvent>(SiteId, OldPhaseName, State->QualityScore);
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("%s complete! Starting: %s"), *OldPhaseName, *NewPhaseName),
		TEXT("Info"));
	
	IB_LOG_INFO("Phase complete: %s → %s (Site: %s)", *OldPhaseName, *NewPhaseName, *SiteId.ToString());
}

void FConstructionSystem::CompleteConstruction(FEntityId SiteId)
{
	auto* State = ECS.GetComponent<C_ConstructionState>(SiteId);
	auto* Site = ECS.GetComponent<C_ConstructionSite>(SiteId);
	if (!State || !Site) return;
	
	State->CurrentPhase = EConstructionPhase::Complete;
	State->PhaseProgress = 1.0f;
	State->OverallProgress = 1.0f;
	
	ActiveSites.Remove(SiteId);
	
	EventBus.Publish<FConstructionCompletedEvent>(SiteId, Site->BuildingId, State->QualityScore);
	EventBus.Publish<FNotificationEvent>(
		TEXT("🎉 CONSTRUCTION COMPLETE! Building finished!"), TEXT("Success"));
	
	IB_LOG_INFO("Construction complete! Building: %s, Quality: %.0f%%",
		*Site->BuildingId.ToString(), State->QualityScore);
}

void FConstructionSystem::GetProgressInfo(FEntityId SiteId, FString& OutPhase,
	float& OutPhaseProgress, float& OutOverall, float& OutQuality, bool& OutPaused) const
{
	auto* State = ECS.GetComponent<C_ConstructionState>(SiteId);
	if (State)
	{
		OutPhase = PhaseToString(State->CurrentPhase);
		OutPhaseProgress = State->PhaseProgress;
		OutOverall = State->OverallProgress;
		OutQuality = State->QualityScore;
		OutPaused = State->bIsPaused;
	}
}

TArray<FEntityId> FConstructionSystem::GetActiveSites() const { return ActiveSites; }

bool FConstructionSystem::IsSiteComplete(FEntityId SiteId) const
{
	auto* State = ECS.GetComponent<C_ConstructionState>(SiteId);
	return State && State->CurrentPhase == EConstructionPhase::Complete;
}

void FConstructionSystem::AssignWorkers(FEntityId SiteId, int32 Count)
{
	auto* Site = ECS.GetComponent<C_ConstructionSite>(SiteId);
	if (Site)
	{
		Site->AssignedWorkerCount += Count;
		Site->BuildSpeed = 1.0f + (Site->AssignedWorkerCount * 0.5f);
		IB_LOG_INFO("Workers assigned to site %s: %d (Speed: %.1fx)",
			*SiteId.ToString(), Site->AssignedWorkerCount, Site->BuildSpeed);
	}
}

FString FConstructionSystem::PhaseToString(EConstructionPhase Phase)
{
	switch (Phase)
	{
	case EConstructionPhase::Foundation:   return TEXT("Foundation");
	case EConstructionPhase::Structure:    return TEXT("Structure");
	case EConstructionPhase::Interior:     return TEXT("Interior");
	case EConstructionPhase::Finalization: return TEXT("Finalization");
	case EConstructionPhase::Complete:     return TEXT("Complete");
	default: return TEXT("Unknown");
	}
}
