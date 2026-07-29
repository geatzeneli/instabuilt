// ProductLines.h — Phase 6 M2: All 7 InstaBuilt product line components
// GDD Section 10: Building Categories
// Each product line has unique mechanics, constraints, and construction behavior

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"
#include "Components/BuildingDesign.h"

// ============================================================
// 1. POP UP SOLUTIONS (28/52/104 m²)
// ============================================================

/** Modular prefab unit configuration */
struct INSTABUILTSIMULATION_API C_PopUpConfig : public FComponentBase
{
	int32 ModuleSize = 28;           // 28, 52, or 104 m²
	int32 ModuleCount = 1;           // How many modules
	bool bModulesStacked = false;    // Vertical stacking enabled at 104 m²
	
	// Transport
	float TransportDistance = 0.0f;  // km from factory
	float TransportCost = 0.0f;
	int32 DeliveryDays = 7;
	
	// Prefab stats
	float FactoryCompletion = 0.85f; // 85% complete when delivered
	float OnSiteWorkHours = 40.0f;   // Remaining work
	
	float GetTotalArea() const { return ModuleSize * ModuleCount; }
	float GetBuildTimeMultiplier() const { return 1.0f - (FactoryCompletion * 0.7f); }
	
	virtual FString GetTypeName() const override { return TEXT("PopUpConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ModuleSize << ModuleCount << bModulesStacked;
		Ar << TransportDistance << TransportCost << DeliveryDays;
		Ar << FactoryCompletion << OnSiteWorkHours;
	}
};

// ============================================================
// 2. MULTISTORY-MULTIFAMILY
// ============================================================

/** Multi-unit residential building configuration */
struct INSTABUILTSIMULATION_API C_MultifamilyConfig : public FComponentBase
{
	int32 TotalUnits = 4;
	int32 MaxStories = 3;
	
	// Unit mix
	int32 StudioUnits = 0;
	int32 OneBedroomUnits = 2;
	int32 TwoBedroomUnits = 2;
	int32 ThreeBedroomUnits = 0;
	
	// Shared spaces
	float CommonAreaPercent = 0.12f;    // Hallways, lobby, stairs
	bool bHasElevator = false;
	bool bHasUndergroundParking = false;
	int32 ParkingSpaces = 0;
	
	// MEP complexity
	float MEPComplexity = 1.0f;         // Multiplier: more units = more complex
	bool bRequiresFireSuppression = false;
	
	void CalculateMetrics()
	{
		TotalUnits = StudioUnits + OneBedroomUnits + TwoBedroomUnits + ThreeBedroomUnits;
		bRequiresFireSuppression = MaxStories >= 3;
		bHasElevator = MaxStories >= 4;
		MEPComplexity = 1.0f + (TotalUnits * 0.1f) + (MaxStories * 0.15f);
	}
	
	virtual FString GetTypeName() const override { return TEXT("MultifamilyConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << TotalUnits << MaxStories;
		Ar << StudioUnits << OneBedroomUnits << TwoBedroomUnits << ThreeBedroomUnits;
		Ar << CommonAreaPercent << bHasElevator << bHasUndergroundParking << ParkingSpaces;
		Ar << MEPComplexity << bRequiresFireSuppression;
	}
};

// ============================================================
// 3. SENIOR HOUSING
// ============================================================

/** Accessibility and care-focused housing configuration */
struct INSTABUILTSIMULATION_API C_SeniorHousingConfig : public FComponentBase
{
	int32 ResidentCapacity = 60;
	int32 PrivateUnits = 30;
	int32 AssistedUnits = 10;
	
	// Required spaces
	bool bHasMedicalSuite = false;
	bool bHasDiningHall = false;
	bool bHasActivityRoom = false;
	bool bHasTherapyRoom = false;
	bool bHasOutdoorGarden = false;
	
	// Accessibility requirements (all mandatory)
	bool bWideDoorways = false;          // Min 90cm
	bool bZeroStepEntries = false;
	bool bGrabBarsInBathrooms = false;
	bool bEmergencyCallSystem = false;
	bool bSlipResistantFloors = false;
	
	// Staff areas
	float StaffAreaPercent = 0.10f;
	int32 StaffCount = 8;
	
	bool ValidateAccessibility() const
	{
		return bWideDoorways && bZeroStepEntries && bGrabBarsInBathrooms
			&& bEmergencyCallSystem && bSlipResistantFloors;
	}
	
	virtual FString GetTypeName() const override { return TEXT("SeniorHousingConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ResidentCapacity << PrivateUnits << AssistedUnits;
		Ar << bHasMedicalSuite << bHasDiningHall << bHasActivityRoom;
		Ar << bHasTherapyRoom << bHasOutdoorGarden;
		Ar << bWideDoorways << bZeroStepEntries << bGrabBarsInBathrooms;
		Ar << bEmergencyCallSystem << bSlipResistantFloors;
		Ar << StaffAreaPercent << StaffCount;
	}
};

// ============================================================
// 4. MICRO APARTMENTS
// ============================================================

/** Space-optimized compact living configuration */
struct INSTABUILTSIMULATION_API C_MicroApartmentConfig : public FComponentBase
{
	float UnitSize = 25.0f;          // 25-35 m² per unit
	int32 UnitCount = 12;
	
	// Space optimization
	float StorageEfficiency = 0.0f;  // 0-100: how well space is used
	float NaturalLightScore = 0.0f;  // Required: good light despite size
	
	// Multi-function elements
	int32 MurphyBedCount = 0;
	int32 FoldDownTableCount = 0;
	int32 BuiltInStorageUnits = 0;
	
	// Community amenities (shared to reduce unit size)
	bool bHasSharedLaundry = true;
	bool bHasSharedKitchen = false;
	bool bHasCoworkingSpace = false;
	bool bHasBikeStorage = true;
	
	float GetDensity() const { return UnitCount / (UnitSize * UnitCount / 100.0f); }
	
	virtual FString GetTypeName() const override { return TEXT("MicroApartmentConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << UnitSize << UnitCount << StorageEfficiency << NaturalLightScore;
		Ar << MurphyBedCount << FoldDownTableCount << BuiltInStorageUnits;
		Ar << bHasSharedLaundry << bHasSharedKitchen << bHasCoworkingSpace << bHasBikeStorage;
	}
};

// ============================================================
// 5. TRADITIONAL HOMES
// ============================================================

/** Regional architectural style configuration */
struct INSTABUILTSIMULATION_API C_TraditionalHomeConfig : public FComponentBase
{
	FString ArchitecturalStyle;     // "Colonial", "Victorian", "Craftsman", "Mediterranean", "Tudor"
	FString Region;                  // "NewEngland", "Southern", "Coastal", "Mountain", "European"
	
	// Style-specific elements
	bool bHasPitchedRoof = true;
	bool bHasPorch = false;
	bool bHasBayWindows = false;
	bool bHasDormers = false;
	bool bHasChimney = true;
	bool bHasShutters = false;
	
	// Materials (style-specific defaults)
	FString ExteriorMaterial = TEXT("Brick");
	FString RoofMaterial = TEXT("AsphaltShingle");
	FString WindowStyle = TEXT("DoubleHung");
	FString DoorStyle = TEXT("Paneled");
	
	// Lot features
	bool bHasFrontYard = true;
	bool bHasBackYard = true;
	bool bHasDriveway = true;
	bool bHasFence = false;
	
	float GetAuthenticityScore() const
	{
		// How well the design matches the style
		float Score = 50.0f;
		if (bHasPitchedRoof) Score += 15;
		if (bHasPorch) Score += 10;
		if (bHasBayWindows) Score += 10;
		if (bHasDormers) Score += 10;
		if (bHasShutters) Score += 5;
		return FMath::Clamp(Score, 0.0f, 100.0f);
	}
	
	virtual FString GetTypeName() const override { return TEXT("TraditionalHomeConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ArchitecturalStyle << Region;
		Ar << bHasPitchedRoof << bHasPorch << bHasBayWindows << bHasDormers;
		Ar << bHasChimney << bHasShutters;
		Ar << ExteriorMaterial << RoofMaterial << WindowStyle << DoorStyle;
		Ar << bHasFrontYard << bHasBackYard << bHasDriveway << bHasFence;
	}
};

// ============================================================
// 6. SIGNATURE HOMES
// ============================================================

/** Premium custom residence configuration */
struct INSTABUILTSIMULATION_API C_SignatureHomeConfig : public FComponentBase
{
	FString ClientName;
	FString ArchitectName;
	
	// Premium features
	bool bHasCustomGeometry = true;       // Non-rectangular rooms
	bool bHasCantileveredVolumes = false;
	bool bHasFloorToCeilingWindows = false;
	bool bHasSmartHomeSystem = false;
	bool bHasWineCellar = false;
	bool bHasHomeTheater = false;
	bool bHasInfinityPool = false;
	bool bHasRooftopTerrace = false;
	
	// Materials (all premium)
	FString FloorMaterial = TEXT("ItalianMarble");
	FString CountertopMaterial = TEXT("Quartzite");
	FString CabinetMaterial = TEXT("CustomWalnut");
	
	// Budget (much higher than standard)
	double MinimumBudget = 500000.0;
	double LuxuryMarkup = 1.5;           // 50% above standard premium
	
	bool ValidateBudget(double ProposedBudget) const
	{
		return ProposedBudget >= MinimumBudget;
	}
	
	virtual FString GetTypeName() const override { return TEXT("SignatureHomeConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ClientName << ArchitectName;
		Ar << bHasCustomGeometry << bHasCantileveredVolumes << bHasFloorToCeilingWindows;
		Ar << bHasSmartHomeSystem << bHasWineCellar << bHasHomeTheater;
		Ar << bHasInfinityPool << bHasRooftopTerrace;
		Ar << FloorMaterial << CountertopMaterial << CabinetMaterial;
		Ar << MinimumBudget << LuxuryMarkup;
	}
};

// ============================================================
// 7. BATHPODS
// ============================================================

/** Prefabricated bathroom module configuration */
struct INSTABUILTSIMULATION_API C_BathpodConfig : public FComponentBase
{
	FString PodModel;
	float Width = 2.4f;
	float Depth = 2.8f;
	float Height = 2.5f;
	
	// Pre-installed fixtures
	bool bHasToilet = true;
	bool bHasSink = true;
	bool bHasShower = true;
	bool bHasBathtub = false;
	bool bHasBidet = false;
	
	// Finishes (pre-applied in factory)
	FString WallFinish = TEXT("CeramicTile");
	FString FloorFinish = TEXT("PorcelainTile");
	FString FixtureFinish = TEXT("Chrome");
	
	// Installation
	float FactoryCompletion = 1.0f;       // Fully complete when delivered
	float InstallationHours = 8.0f;       // Just connect plumbing/electric
	bool bPressureTested = true;
	bool bWaterproofed = true;
	
	virtual FString GetTypeName() const override { return TEXT("BathpodConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << PodModel << Width << Depth << Height;
		Ar << bHasToilet << bHasSink << bHasShower << bHasBathtub << bHasBidet;
		Ar << WallFinish << FloorFinish << FixtureFinish;
		Ar << FactoryCompletion << InstallationHours << bPressureTested << bWaterproofed;
	}
};

// ============================================================
// STRUCTURAL SYSTEMS (M2 expansion)
// ============================================================

/** Structural configuration for a building */
struct INSTABUILTSIMULATION_API C_StructuralSystem : public FComponentBase
{
	FString SystemType;             // "TimberFrame", "SteelFrame", "Concrete", "Hybrid"
	
	// Load-bearing elements
	bool bHasLoadBearingWalls = true;
	bool bHasColumns = false;
	bool bHasBeams = false;
	bool bHasTrusses = false;
	
	// Foundation type
	FString FoundationType = TEXT("SlabOnGrade");  // SlabOnGrade, CrawlSpace, FullBasement
	
	// Seismic zone
	int32 SeismicZone = 1;          // 1-4 (4 = highest risk)
	bool bRequiresSeismicEngineering = false;
	
	// Wind zone
	int32 WindZone = 1;             // 1-3
	bool bRequiresWindEngineering = false;
	
	// Span limits (meters)
	float MaxBeamSpan = 6.0f;
	float MaxJoistSpan = 4.5f;
	float MaxTrussSpan = 12.0f;
	
	void DetermineRequirements(int32 Stories, const FString& Region)
	{
		bRequiresSeismicEngineering = SeismicZone >= 3 || Stories >= 4;
		bRequiresWindEngineering = WindZone >= 2 || Stories >= 3;
		
		if (Stories >= 3) bHasColumns = true;
		if (Stories >= 5) SystemType = TEXT("SteelFrame");
	}
	
	virtual FString GetTypeName() const override { return TEXT("StructuralSystem"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << SystemType << bHasLoadBearingWalls << bHasColumns << bHasBeams << bHasTrusses;
		Ar << FoundationType << SeismicZone << WindZone;
		Ar << bRequiresSeismicEngineering << bRequiresWindEngineering;
		Ar << MaxBeamSpan << MaxJoistSpan << MaxTrussSpan;
	}
};

// ============================================================
// ROOF SYSTEM
// ============================================================

/** Roof configuration */
struct INSTABUILTSIMULATION_API C_RoofSystem : public FComponentBase
{
	FString RoofType;               // Flat, Gable, Hip, Mansard, Gambrel, Shed, Butterfly
	float Pitch = 30.0f;            // Degrees (0=flat, 60=steep)
	float Overhang = 0.6f;          // meters
	
	FString RoofingMaterial = TEXT("AsphaltShingle");
	FString Color = TEXT("Charcoal");
	
	// Features
	bool bHasDormers = false;
	int32 DormerCount = 0;
	bool bHasSkylights = false;
	int32 SkylightCount = 0;
	bool bHasGutters = true;
	bool bHasSolarPanels = false;
	
	// Drainage
	bool bHasInternalDrainage = false;   // Flat roofs
	float DrainageCapacity = 0.0f;       // Computed
	
	void CalculateDrainage()
	{
		bHasInternalDrainage = (Pitch < 5.0f); // Flat roofs need internal drainage
		DrainageCapacity = bHasInternalDrainage ? 100.0f : 0.0f;
	}
	
	virtual FString GetTypeName() const override { return TEXT("RoofSystem"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << RoofType << Pitch << Overhang << RoofingMaterial << Color;
		Ar << bHasDormers << DormerCount << bHasSkylights << SkylightCount;
		Ar << bHasGutters << bHasSolarPanels;
	}
};

// ============================================================
// BUILDING CATEGORY REGISTRY
// ============================================================

/** Registry entry for a building type — data-driven, moddable */
struct INSTABUILTSIMULATION_API C_BuildingCategory : public FComponentBase
{
	FString CategoryName;
	FString ProductLine;            // Which InstaBuilt product line
	int32 MinTier = 1;              // Company tier required to build
	int32 MaxStories = 2;
	float MinArea = 20.0f;
	float MaxArea = 500.0f;
	
	// Required specialists
	bool bRequiresArchitect = false;
	bool bRequiresEngineer = false;
	
	// Construction modifiers
	float BuildTimeMultiplier = 1.0f;
	float CostMultiplier = 1.0f;
	float QualityTarget = 80.0f;
	
	// Validation rules (comma-separated rule IDs)
	FString ValidationRules;
	
	virtual FString GetTypeName() const override { return TEXT("BuildingCategory"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << CategoryName << ProductLine << MinTier << MaxStories;
		Ar << MinArea << MaxArea << bRequiresArchitect << bRequiresEngineer;
		Ar << BuildTimeMultiplier << CostMultiplier << QualityTarget << ValidationRules;
	}
};
