// BuildingDesigner.h — VS M2+M3: Architecture design + construction visuals system
// GSS Section 6: Design workflow (survey → walls → rooms → doors/windows → validation)
// GSS Section 7: Construction phases with visual feedback

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/BuildingDesign.h"
#include "Components/QualityComponents.h"

/**
 * FBuildingDesigner — The architecture design tool
 * 
 * This is the primary creative tool. Players place rooms, draw walls,
 * add doors and windows, select materials, and validate their design
 * against client requirements and building codes.
 */
class INSTABUILTSIMULATION_API FBuildingDesigner : public FInstaBuiltSystem
{
public:
	FBuildingDesigner();
	
	virtual void OnInitialize() override;
	
	// ============================================================
	// DESIGN CREATION
	// ============================================================
	
	/** Start a new building design */
	FEntityId CreateDesign(const FString& Name, const FString& BuildingType);
	
	/** Get the current active design */
	FEntityId GetActiveDesign() const { return ActiveDesignId; }
	
	// ============================================================
	// ROOM OPERATIONS
	// ============================================================
	
	/** Add a rectangular room. Returns room entity ID. */
	FEntityId AddRoom(const FString& Name, const FString& Type, float X, float Y, float Width, float Depth, int32 Floor = 0);
	
	/** Change a room's type */
	void SetRoomType(FEntityId RoomId, const FString& NewType);
	
	/** Get all rooms in the active design */
	TArray<FEntityId> GetRooms() const;
	
	// ============================================================
	// WALL OPERATIONS
	// ============================================================
	
	/** Add a wall between two points */
	FEntityId AddWall(float StartX, float StartY, float EndX, float EndY, bool bExterior, bool bLoadBearing, int32 Floor = 0);
	
	/** Remove a wall */
	void RemoveWall(FEntityId WallId);
	
	// ============================================================
	// DOOR/WINDOW OPERATIONS
	// ============================================================
	
	/** Place a door on a wall connecting two rooms */
	FEntityId AddDoor(FEntityId WallId, float PositionX, float PositionY, FEntityId RoomA, FEntityId RoomB);
	
	/** Place a window on an exterior wall */
	FEntityId AddWindow(FEntityId WallId, float PositionX, float PositionY, FEntityId RoomId);
	
	// ============================================================
	// MATERIALS
	// ============================================================
	
	/** Set the material tier for the design */
	void SetMaterialTier(EMaterialTier Tier);
	
	/** Get material tier display name */
	static FString TierToString(EMaterialTier Tier);
	
	/** Set room floor material */
	void SetRoomFloor(FEntityId RoomId, const FString& Material);
	
	// ============================================================
	// VALIDATION
	// ============================================================
	
	/** Run full validation. Returns true if design passes. */
	bool ValidateDesign();
	
	/** Get validation errors as a formatted string */
	FString GetValidationReport() const;
	
	/** Estimate construction cost based on current design */
	float EstimateCost() const;
	
	// ============================================================
	// BLUEPRINT
	// ============================================================
	
	/** Generate a blueprint summary for the UI */
	FString GetBlueprintSummary() const;
	
	/** Approve the design and lock it */
	bool ApproveDesign();
	
	// ============================================================
	// CONSTRUCTION VISUALS (M3)
	// ============================================================
	
	/** Get construction phase visual description */
	FString GetPhaseVisualDescription(int32 PhaseIndex) const;
	
	/** Get worker activity description for current phase */
	FString GetWorkerActivity(int32 PhaseIndex) const;
	
	/** Get site atmosphere description (for audio/visual direction) */
	FString GetSiteAtmosphere() const;
	
private:
	FEntityId ActiveDesignId;
	
	// Material cost multipliers
	static constexpr float BudgetMultiplier = 0.75f;
	static constexpr float StandardMultiplier = 1.0f;
	static constexpr float PremiumMultiplier = 1.5f;
	
	// Cost per m² per material tier
	float GetCostPerSquareMeter() const;
	
	// Validation helpers
	bool ValidateRoomAccess();
	bool ValidateExteriorWalls();
	bool ValidateWindows();
	bool ValidateClientRequirements();
	
	// Construction phase visuals (M3)
	static constexpr const TCHAR* PhaseAtmosphere[4] = {
		TEXT("Excavators digging foundations. Dust and gravel. Workers in hard hats marking lines."),
		TEXT("Steel beams and timber frames rising. The skeleton takes shape. Hammers echoing."),
		TEXT("Drywall going up. Painters and tilers inside. The building becomes a home."),
		TEXT("Final touches. Landscaping. Windows cleaned. The ribbon goes on the door.")
	};
	
	static constexpr const TCHAR* WorkerActivities[4] = {
		TEXT("Surveying, excavating, pouring concrete, laying rebar"),
		TEXT("Erecting framing, installing joists, sheathing walls, raising trusses"),
		TEXT("Hanging drywall, painting walls, laying flooring, installing cabinets"),
		TEXT("Installing fixtures, landscaping, final cleaning, punch-list fixes")
	};
};
