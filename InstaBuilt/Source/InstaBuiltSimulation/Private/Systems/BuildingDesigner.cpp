// BuildingDesigner.cpp — VS M2+M3: Full architecture design + construction visuals

#include "Systems/BuildingDesigner.h"
#include "Logging/InstaBuiltLog.h"

FBuildingDesigner::FBuildingDesigner()
	: FInstaBuiltSystem(TEXT("BuildingDesigner"))
{
	AddDependency(TEXT("BuildingSystem"));
}

void FBuildingDesigner::OnInitialize()
{
	IB_LOG_INFO("BuildingDesigner initialized. Ready for architecture design.");
}

// ============================================================
// DESIGN CREATION
// ============================================================

FEntityId FBuildingDesigner::CreateDesign(const FString& Name, const FString& BuildingType)
{
	ActiveDesignId = ECS.CreateEntity(EEntityType::Building);
	
	auto* Design = ECS.AddComponent<C_BuildingDesignData>(ActiveDesignId);
	Design->DesignName = Name;
	Design->BuildingType = BuildingType;
	Design->MaterialTier = EMaterialTier::Standard;
	
	auto* Quality = ECS.AddComponent<C_BuildingQuality>(ActiveDesignId);
	
	IB_LOG_INFO("Design created: %s (%s)", *Name, *BuildingType);
	return ActiveDesignId;
}

// ============================================================
// ROOM OPERATIONS
// ============================================================

FEntityId FBuildingDesigner::AddRoom(const FString& Name, const FString& Type,
	float X, float Y, float Width, float Depth, int32 Floor)
{
	FEntityId RoomId = ECS.CreateEntity(EEntityType::Room);
	
	auto* Room = ECS.AddComponent<C_Room>(RoomId);
	Room->RoomName = Name;
	Room->RoomType = Type;
	Room->MinX = X;
	Room->MinY = Y;
	Room->MaxX = X + Width;
	Room->MaxY = Y + Depth;
	Room->Area = Width * Depth;
	Room->FloorIndex = Floor;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design)
	{
		Design->AddRoom(RoomId);
		Design->TotalArea += Room->Area;
	}
	
	// Auto-create walls around the room
	AddWall(X, Y, X + Width, Y, true, false, Floor);         // Bottom
	AddWall(X + Width, Y, X + Width, Y + Depth, true, false, Floor); // Right
	AddWall(X + Width, Y + Depth, X, Y + Depth, true, false, Floor); // Top
	AddWall(X, Y + Depth, X, Y, true, false, Floor);         // Left
	
	IB_LOG_INFO("Room added: %s (%s, %.1f m²)", *Name, *Type, Room->Area);
	return RoomId;
}

void FBuildingDesigner::SetRoomType(FEntityId RoomId, const FString& NewType)
{
	auto* Room = ECS.GetComponent<C_Room>(RoomId);
	if (Room) Room->RoomType = NewType;
}

TArray<FEntityId> FBuildingDesigner::GetRooms() const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	return Design ? Design->RoomIds : TArray<FEntityId>();
}

// ============================================================
// WALL OPERATIONS
// ============================================================

FEntityId FBuildingDesigner::AddWall(float StartX, float StartY, float EndX, float EndY,
	bool bExterior, bool bLoadBearing, int32 Floor)
{
	FEntityId WallId = ECS.CreateEntity(EEntityType::Wall);
	
	auto* Wall = ECS.AddComponent<C_Wall>(WallId);
	Wall->StartX = StartX;
	Wall->StartY = StartY;
	Wall->EndX = EndX;
	Wall->EndY = EndY;
	Wall->bIsExterior = bExterior;
	Wall->bIsLoadBearing = bLoadBearing;
	Wall->FloorIndex = Floor;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design) Design->AddWall(WallId);
	
	return WallId;
}

void FBuildingDesigner::RemoveWall(FEntityId WallId)
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design) Design->WallIds.Remove(WallId);
	ECS.DestroyEntity(WallId);
}

// ============================================================
// DOOR/WINDOW
// ============================================================

FEntityId FBuildingDesigner::AddDoor(FEntityId WallId, float PosX, float PosY,
	FEntityId RoomA, FEntityId RoomB)
{
	FEntityId DoorId = ECS.CreateEntity(EEntityType::Door);
	
	auto* Door = ECS.AddComponent<C_Door>(DoorId);
	Door->PositionX = PosX;
	Door->PositionY = PosY;
	Door->WallId = WallId;
	Door->RoomA = RoomA;
	Door->RoomB = RoomB;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design) Design->AddDoor(DoorId);
	
	return DoorId;
}

FEntityId FBuildingDesigner::AddWindow(FEntityId WallId, float PosX, float PosY, FEntityId RoomId)
{
	FEntityId WindowId = ECS.CreateEntity(EEntityType::Window);
	
	auto* Window = ECS.AddComponent<C_Window>(WindowId);
	Window->PositionX = PosX;
	Window->PositionY = PosY;
	Window->WallId = WallId;
	Window->RoomId = RoomId;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design) Design->AddWindow(WindowId);
	
	return WindowId;
}

// ============================================================
// MATERIALS
// ============================================================

void FBuildingDesigner::SetMaterialTier(EMaterialTier Tier)
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design)
	{
		Design->MaterialTier = Tier;
		Design->EstimatedCost = EstimateCost();
		IB_LOG_INFO("Material tier set to: %s (Est. cost: $%.0f)", *TierToString(Tier), Design->EstimatedCost);
	}
}

FString FBuildingDesigner::TierToString(EMaterialTier Tier)
{
	switch (Tier)
	{
	case EMaterialTier::Budget:   return TEXT("Budget");
	case EMaterialTier::Standard: return TEXT("Standard");
	case EMaterialTier::Premium:  return TEXT("Premium");
	default: return TEXT("Unknown");
	}
}

void FBuildingDesigner::SetRoomFloor(FEntityId RoomId, const FString& Material)
{
	auto* Room = ECS.GetComponent<C_Room>(RoomId);
	if (Room) Room->FloorMaterial = Material;
}

float FBuildingDesigner::GetCostPerSquareMeter() const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return 800.0f;
	
	switch (Design->MaterialTier)
	{
	case EMaterialTier::Budget:   return 800.0f;
	case EMaterialTier::Standard: return 1200.0f;
	case EMaterialTier::Premium:  return 2000.0f;
	default: return 1200.0f;
	}
}

// ============================================================
// VALIDATION
// ============================================================

bool FBuildingDesigner::ValidateDesign()
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return false;
	
	Design->ValidationErrors.Empty();
	
	ValidateRoomAccess();
	ValidateExteriorWalls();
	ValidateWindows();
	ValidateClientRequirements();
	
	Design->bHasValidationErrors = Design->ValidationErrors.Num() > 0;
	Design->EstimatedCost = EstimateCost();
	
	if (!Design->bHasValidationErrors)
	{
		IB_LOG_INFO("Design validation PASSED. Estimated cost: $%.0f", Design->EstimatedCost);
	}
	else
	{
		IB_LOG_WARN("Design validation FAILED with %d errors.", Design->ValidationErrors.Num());
		for (const auto& Err : Design->ValidationErrors)
		{
			IB_LOG_WARN("  - %s", *Err);
		}
	}
	
	return !Design->bHasValidationErrors;
}

bool FBuildingDesigner::ValidateRoomAccess()
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return false;
	
	bool bValid = true;
	for (auto& RoomId : Design->RoomIds)
	{
		auto* Room = ECS.GetComponent<C_Room>(RoomId);
		if (Room && Room->DoorIds.Num() == 0)
		{
			Design->ValidationErrors.Add(
				FString::Printf(TEXT("Room '%s' has no door — inaccessible."), *Room->RoomName));
			bValid = false;
		}
	}
	return bValid;
}

bool FBuildingDesigner::ValidateExteriorWalls()
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return false;
	
	int32 ExteriorCount = 0;
	for (auto& WallId : Design->WallIds)
	{
		auto* Wall = ECS.GetComponent<C_Wall>(WallId);
		if (Wall && Wall->bIsExterior) ExteriorCount++;
	}
	
	if (ExteriorCount < 4)
	{
		Design->ValidationErrors.Add(TEXT("Building must have at least 4 exterior walls."));
		return false;
	}
	return true;
}

bool FBuildingDesigner::ValidateWindows()
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return false;
	
	bool bValid = true;
	for (auto& RoomId : Design->RoomIds)
	{
		auto* Room = ECS.GetComponent<C_Room>(RoomId);
		if (Room && Room->RoomType == TEXT("Bedroom") && Room->WindowIds.Num() == 0)
		{
			Design->ValidationErrors.Add(
				FString::Printf(TEXT("Bedroom '%s' requires at least one window."), *Room->RoomName));
			bValid = false;
		}
	}
	return bValid;
}

bool FBuildingDesigner::ValidateClientRequirements()
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return false;
	
	bool bValid = true;
	
	// Minimum 3 rooms
	if (Design->RoomIds.Num() < 3)
	{
		Design->ValidationErrors.Add(TEXT("Client requires at least 3 rooms."));
		bValid = false;
	}
	
	// Must have a bedroom
	bool bHasBedroom = false;
	bool bHasBathroom = false;
	for (auto& RoomId : Design->RoomIds)
	{
		auto* Room = ECS.GetComponent<C_Room>(RoomId);
		if (Room)
		{
			if (Room->RoomType == TEXT("Bedroom")) bHasBedroom = true;
			if (Room->RoomType == TEXT("Bathroom")) bHasBathroom = true;
		}
	}
	
	if (!bHasBedroom)
	{
		Design->ValidationErrors.Add(TEXT("Design must include at least one bedroom."));
		bValid = false;
	}
	if (!bHasBathroom)
	{
		Design->ValidationErrors.Add(TEXT("Design must include at least one bathroom."));
		bValid = false;
	}
	
	return bValid;
}

FString FBuildingDesigner::GetValidationReport() const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return TEXT("No active design.");
	
	if (Design->bHasValidationErrors)
	{
		FString Report = FString::Printf(TEXT("VALIDATION FAILED — %d issues:\n"), Design->ValidationErrors.Num());
		for (const auto& Err : Design->ValidationErrors)
		{
			Report += FString::Printf(TEXT("  ❌ %s\n"), *Err);
		}
		return Report;
	}
	else
	{
		return FString::Printf(TEXT("✅ VALIDATION PASSED\n  Estimated cost: $%.0f\n  Area: %.1f m²\n  Rooms: %d"),
			Design->EstimatedCost, Design->TotalArea, Design->RoomIds.Num());
	}
}

float FBuildingDesigner::EstimateCost() const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return 0.0f;
	
	float BaseCost = Design->TotalArea * GetCostPerSquareMeter();
	
	// Add per-door/window costs
	BaseCost += Design->DoorIds.Num() * 500.0f;
	BaseCost += Design->WindowIds.Num() * 800.0f;
	
	return BaseCost;
}

FString FBuildingDesigner::GetBlueprintSummary() const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (!Design) return TEXT("No design");
	
	return FString::Printf(
		TEXT("BLUEPRINT: %s\n")
		TEXT("  Type: %s | Tier: %s\n")
		TEXT("  Area: %.1f m² | Rooms: %d | Floors: %d\n")
		TEXT("  Walls: %d | Doors: %d | Windows: %d\n")
		TEXT("  Est. Cost: $%.0f"),
		*Design->DesignName, *Design->BuildingType, *TierToString(Design->MaterialTier),
		Design->TotalArea, Design->RoomIds.Num(), Design->FloorCount,
		Design->WallIds.Num(), Design->DoorIds.Num(), Design->WindowIds.Num(),
		Design->EstimatedCost);
}

bool FBuildingDesigner::ApproveDesign()
{
	if (!ValidateDesign()) return false;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(ActiveDesignId);
	if (Design) Design->bIsApproved = true;
	
	IB_LOG_INFO("Design approved: %s", *GetBlueprintSummary());
	return true;
}

// ============================================================
// CONSTRUCTION VISUALS (M3)
// ============================================================

FString FBuildingDesigner::GetPhaseVisualDescription(int32 PhaseIndex) const
{
	if (PhaseIndex >= 0 && PhaseIndex < 4)
		return PhaseAtmosphere[PhaseIndex];
	return TEXT("Unknown phase");
}

FString FBuildingDesigner::GetWorkerActivity(int32 PhaseIndex) const
{
	if (PhaseIndex >= 0 && PhaseIndex < 4)
		return WorkerActivities[PhaseIndex];
	return TEXT("Idle");
}

FString FBuildingDesigner::GetSiteAtmosphere() const
{
	return TEXT("Morning sunlight. Birdsong mixed with distant construction noise. "
		"The smell of fresh lumber and concrete. Workers greeting each other. "
		"Equipment rumbling to life. Another day of building the future.");
}
