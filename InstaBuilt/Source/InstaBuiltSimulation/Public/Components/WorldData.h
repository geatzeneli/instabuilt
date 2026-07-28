// WorldData.h — VS M5: District, Plot, Road, Environment data
// GSS Section 4.4: Site Inspection
// ARCHITECTURE: Region → City → District → Parcel hierarchy

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// DISTRICT & PLOT DATA
// ============================================================

/** A playable district containing construction plots */
struct INSTABUILTSIMULATION_API C_District : public FComponentBase
{
	FString DistrictName;
	FString Description;
	
	// Boundaries (world coordinates)
	float MinX, MinY, MaxX, MaxY;
	
	// Zoning info
	FString ZoningType;          // "Residential", "Commercial", "Mixed-Use"
	float MaxBuildingHeight;     // meters
	float MinSetback;            // meters from property line
	
	// Visual description (for UE5 level designer)
	FString EnvironmentType;     // "Suburban", "Urban", "Coastal", "Riverside"
	FString TimeOfDay;           // "Morning", "GoldenHour", "Night"
	FString WeatherPreset;       // "Clear", "LightClouds", "Foggy"
	
	TArray<FEntityId> PlotIds;
	TArray<FEntityId> RoadIds;
	
	virtual FString GetTypeName() const override { return TEXT("District"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << DistrictName << Description;
		Ar << MinX << MinY << MaxX << MaxY;
		Ar << ZoningType << MaxBuildingHeight << MinSetback;
		Ar << EnvironmentType << TimeOfDay << WeatherPreset;
	}
};

/** A single construction plot */
struct INSTABUILTSIMULATION_API C_Plot : public FComponentBase
{
	FString PlotAddress;
	float SizeX, SizeY;          // Plot dimensions (meters)
	float Area;                   // Computed m²
	float PositionX, PositionY;   // World position center
	float Rotation;               // Yaw rotation
	
	// State
	bool bIsAvailable = true;
	bool bIsOwned = false;
	bool bHasBuilding = false;
	FEntityId BuildingId;         // Current building on this plot
	FEntityId OwnerCompanyId;
	
	// Terrain
	float GroundElevation = 0.0f;
	FString SoilType = TEXT("Loam");   // "Clay", "Sand", "Loam", "Rocky"
	bool bRequiresGrading = false;
	
	// Utilities access
	bool bHasWater = true;
	bool bHasElectricity = true;
	bool bHasSewer = true;
	bool bHasGas = true;
	
	virtual FString GetTypeName() const override { return TEXT("Plot"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << PlotAddress << SizeX << SizeY << Area;
		Ar << PositionX << PositionY << Rotation;
		Ar << bIsAvailable << bIsOwned << bHasBuilding;
		Ar << GroundElevation << SoilType << bRequiresGrading;
		Ar << bHasWater << bHasElectricity << bHasSewer << bHasGas;
	}
};

/** Road segment for world layout */
struct INSTABUILTSIMULATION_API C_Road : public FComponentBase
{
	FString RoadName;
	FString RoadType;            // "Highway", "MainStreet", "Residential", "Alley"
	float Width = 8.0f;
	float StartX, StartY;
	float EndX, EndY;
	
	virtual FString GetTypeName() const override { return TEXT("Road"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << RoadName << RoadType << Width << StartX << StartY << EndX << EndY;
	}
};

// ============================================================
// SHOWCASE CONFIGURATION (for the vertical slice demo)
// ============================================================

/** Configuration for the vertical slice showcase district */
struct INSTABUILTSIMULATION_API C_ShowcaseConfig : public FComponentBase
{
	FString ShowcaseName = TEXT("Riverside District");
	FString Description = TEXT("A premium residential neighborhood along the river. "
		"Perfect for demonstrating InstaBuilt's traditional home construction capabilities.");
	
	// Camera presets for the demo
	float CameraOrbitDistance = 40.0f;
	float CameraOrbitHeight = 25.0f;
	float CameraOrbitAngle = 45.0f;
	
	// Showcase plot (where the player builds)
	float ShowcasePlotX = 100.0f;
	float ShowcasePlotY = 200.0f;
	float ShowcasePlotWidth = 30.0f;
	float ShowcasePlotDepth = 25.0f;
	
	// Completed showcase building (already built, for aspirational viewing)
	float ShowcaseBuildingX = 300.0f;
	float ShowcaseBuildingY = 200.0f;
	FString ShowcaseBuildingType = TEXT("TRADITIONAL_HOME");
	FString ShowcaseBuildingDesc = TEXT("A fully completed InstaBuilt Traditional Home. "
		"Custom brick facade, premium hardwood floors, KfW 40 certified. "
		"This is what your company can build.");
	
	virtual FString GetTypeName() const override { return TEXT("ShowcaseConfig"); }
};
