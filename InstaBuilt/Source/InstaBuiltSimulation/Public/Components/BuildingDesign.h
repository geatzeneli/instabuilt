// BuildingDesign.h — VS M2: Architecture design tool components
// GSS Section 6: Building Design Workflow (15 steps → condensed to 8 for VS)

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// DESIGN ELEMENTS
// ============================================================

/** A wall segment in the design */
struct INSTABUILTSIMULATION_API C_Wall : public FComponentBase
{
	float StartX, StartY;     // 2D position (meters, relative to building origin)
	float EndX, EndY;
	float Height = 2.7f;      // Standard ceiling height
	float Thickness = 0.15f;  // 15cm interior, 25cm exterior
	
	bool bIsExterior = false;
	bool bIsLoadBearing = false;
	
	int32 FloorIndex = 0;
	
	virtual FString GetTypeName() const override { return TEXT("Wall"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << StartX << StartY << EndX << EndY << Height << Thickness;
		Ar << bIsExterior << bIsLoadBearing << FloorIndex;
	}
};

/** A room defined by enclosing walls */
struct INSTABUILTSIMULATION_API C_Room : public FComponentBase
{
	FString RoomName;
	FString RoomType;          // "Bedroom", "Kitchen", "Bathroom", "Living", "Hallway", "Garage"
	float Area = 0.0f;         // Computed m²
	float MinX, MinY, MaxX, MaxY; // Bounding box
	
	int32 FloorIndex = 0;
	TArray<FEntityId> WallIds;     // Walls enclosing this room
	TArray<FEntityId> DoorIds;     // Doors in this room
	TArray<FEntityId> WindowIds;   // Windows in this room
	
	// Materials
	FString FloorMaterial = TEXT("Hardwood");
	FString WallFinish = TEXT("Paint_White");
	
	virtual FString GetTypeName() const override { return TEXT("Room"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << RoomName << RoomType << Area;
		Ar << MinX << MinY << MaxX << MaxY << FloorIndex;
		Ar << FloorMaterial << WallFinish;
	}
};

/** A door */
struct INSTABUILTSIMULATION_API C_Door : public FComponentBase
{
	float PositionX, PositionY;  // Center position on wall
	float Width = 0.9f;          // Standard door width
	FEntityId WallId;            // Which wall this door is on
	FEntityId RoomA, RoomB;      // Rooms this door connects
	bool bIsExteriorDoor = false;
	
	virtual FString GetTypeName() const override { return TEXT("Door"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << PositionX << PositionY << Width << bIsExteriorDoor;
	}
};

/** A window */
struct INSTABUILTSIMULATION_API C_Window : public FComponentBase
{
	float PositionX, PositionY;
	float Width = 1.2f;
	float Height = 1.5f;
	float SillHeight = 0.9f;
	FEntityId WallId;
	FEntityId RoomId;
	
	virtual FString GetTypeName() const override { return TEXT("Window"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << PositionX << PositionY << Width << Height << SillHeight;
	}
};

// ============================================================
// DESIGN STATE
// ============================================================

/** Material tier for the building */
UENUM()
enum class EMaterialTier : uint8
{
	Budget,
	Standard,
	Premium
};

/** The complete building design */
struct INSTABUILTSIMULATION_API C_BuildingDesignData : public FComponentBase
{
	FString DesignName;
	FString BuildingType;      // "POP_UP_28", "TRADITIONAL_HOME", etc.
	EMaterialTier MaterialTier = EMaterialTier::Standard;
	
	int32 FloorCount = 1;
	float TotalArea = 28.0f;
	float EstimatedCost = 0.0f;
	
	// Design state
	bool bIsApproved = false;
	bool bHasValidationErrors = false;
	TArray<FString> ValidationErrors;
	
	// Entity references
	TArray<FEntityId> RoomIds;
	TArray<FEntityId> WallIds;
	TArray<FEntityId> DoorIds;
	TArray<FEntityId> WindowIds;
	
	void AddRoom(FEntityId Id) { RoomIds.Add(Id); }
	void AddWall(FEntityId Id) { WallIds.Add(Id); }
	void AddDoor(FEntityId Id) { DoorIds.Add(Id); }
	void AddWindow(FEntityId Id) { WindowIds.Add(Id); }
	
	virtual FString GetTypeName() const override { return TEXT("BuildingDesignData"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << DesignName << BuildingType;
		int32 TierInt = (int32)MaterialTier; Ar << TierInt; MaterialTier = (EMaterialTier)TierInt;
		Ar << FloorCount << TotalArea << EstimatedCost << bIsApproved;
	}
};
