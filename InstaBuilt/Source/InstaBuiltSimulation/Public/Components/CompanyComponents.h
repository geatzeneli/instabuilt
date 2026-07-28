// CompanyComponents.h — ECS components for Company, Contract, Building, Worker, Economy
// DATA_MODEL.md Sections 2.3-2.8: All component definitions
// Milestones 4-9: Prototype component set

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// MILESTONE 4: COMPANY COMPONENTS
// ============================================================

/** Financial state of the player company (DATA_MODEL Section 2.5) */
struct INSTABUILTSIMULATION_API C_Financials : public FComponentBase
{
	double CashOnHand = 250000.0;       // Starting funds
	double RevenueYTD = 0.0;
	double ExpensesYTD = 0.0;
	double TotalProfit = 0.0;
	int32 ProjectsCompleted = 0;
	
	virtual FString GetTypeName() const override { return TEXT("Financials"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << CashOnHand << RevenueYTD << ExpensesYTD << TotalProfit << ProjectsCompleted;
	}
};

/** Reputation scores (DATA_MODEL Section 2.8) */
struct INSTABUILTSIMULATION_API C_Reputation : public FComponentBase
{
	float QualityScore = 50.0f;
	float ReliabilityScore = 50.0f;
	float InnovationScore = 50.0f;
	float CommunityScore = 50.0f;
	float SafetyScore = 50.0f;
	float OverallRating = 50.0f;
	
	virtual FString GetTypeName() const override { return TEXT("Reputation"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << QualityScore << ReliabilityScore << InnovationScore
		   << CommunityScore << SafetyScore << OverallRating;
	}
	
	void RecalculateOverall()
	{
		OverallRating = QualityScore * 0.30f + ReliabilityScore * 0.25f
			+ InnovationScore * 0.15f + CommunityScore * 0.15f + SafetyScore * 0.15f;
	}
};

/** Company identity */
struct INSTABUILTSIMULATION_API C_CompanyIdentity : public FComponentBase
{
	FString CompanyName = TEXT("InstaBuilt");
	FString FounderName = TEXT("Player");
	int32 CompanyTier = 1; // 1-6
	
	virtual FString GetTypeName() const override { return TEXT("CompanyIdentity"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << CompanyName << FounderName << CompanyTier;
	}
};

// ============================================================
// MILESTONE 5: CONTRACT COMPONENTS
// ============================================================

/** Contract state enum */
UENUM()
enum class EContractState : uint8
{
	Available,      // Can be bid on
	Bidding,        // Player has bid, waiting for award
	Awarded,        // Player won the bid
	Active,         // Construction in progress
	Completed,      // Successfully delivered
	Cancelled,      // Terminated
	Failed          // Player failed to deliver
};

/** Contract entity (DATA_MODEL Section 1.2) */
struct INSTABUILTSIMULATION_API C_ContractData : public FComponentBase
{
	FString ContractName;
	FString ClientName;
	EContractState State = EContractState::Available;
	
	// Requirements
	FString BuildingType;     // "POP_UP_28", etc.
	int32 RequiredRooms = 3;
	float RequiredArea = 28.0f; // m²
	
	// Financial
	double BudgetMin = 80000.0;
	double BudgetMax = 120000.0;
	double PlayerBid = 0.0;
	double Reward = 100000.0;
	
	// Timeline
	int32 DeadlineDays = 90;
	int32 DaysRemaining = 90;
	
	// Completion
	int32 QualityScore = 0;
	bool bOnTime = false;
	
	virtual FString GetTypeName() const override { return TEXT("ContractData"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ContractName << ClientName;
		int32 StateInt = (int32)State; Ar << StateInt; State = (EContractState)StateInt;
		Ar << BuildingType << RequiredRooms << RequiredArea;
		Ar << BudgetMin << BudgetMax << PlayerBid << Reward;
		Ar << DeadlineDays << DaysRemaining << QualityScore << bOnTime;
	}
};

// ============================================================
// MILESTONE 6: BUILDING COMPONENTS
// ============================================================

/** Construction phase enum */
UENUM()
enum class EConstructionPhase : uint8
{
	None = 0,
	Foundation,
	Structure,
	Interior,
	Finalization,
	Complete
};

/** Building entity (DATA_MODEL Section 2.3) */
struct INSTABUILTSIMULATION_API C_BuildingData : public FComponentBase
{
	FString BuildingName;
	FString BuildingType;      // "POP_UP_28", "MULTIFAMILY", etc.
	float TotalArea = 28.0f;   // m²
	float FootprintArea = 28.0f;
	int32 FloorCount = 1;
	FEntityId ParcelId;        // The parcel this building sits on
	
	virtual FString GetTypeName() const override { return TEXT("BuildingData"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << BuildingName << BuildingType << TotalArea << FootprintArea << FloorCount;
	}
};

/** Construction state (DATA_MODEL Section 2.3) */
struct INSTABUILTSIMULATION_API C_ConstructionState : public FComponentBase
{
	EConstructionPhase CurrentPhase = EConstructionPhase::None;
	float PhaseProgress = 0.0f;     // 0.0 to 1.0 within current phase
	float OverallProgress = 0.0f;   // 0.0 to 1.0 total
	float QualityScore = 100.0f;    // Starts perfect, degrades with issues
	bool bIsPaused = false;
	FEntityId ContractId;           // Which contract this build fulfills
	
	virtual FString GetTypeName() const override { return TEXT("ConstructionState"); }
	virtual void Serialize(FArchive& Ar) override
	{
		int32 PhaseInt = (int32)CurrentPhase; Ar << PhaseInt; CurrentPhase = (EConstructionPhase)PhaseInt;
		Ar << PhaseProgress << OverallProgress << QualityScore << bIsPaused;
	}
};

/** Construction site — the active construction operation */
struct INSTABUILTSIMULATION_API C_ConstructionSite : public FComponentBase
{
	FEntityId BuildingId;
	FEntityId ContractId;
	float BuildSpeed = 1.0f;        // Multiplier from workers/equipment
	int32 AssignedWorkerCount = 0;
	
	virtual FString GetTypeName() const override { return TEXT("ConstructionSite"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << BuildSpeed << AssignedWorkerCount;
	}
};

// ============================================================
// MILESTONE 8: WORKER COMPONENTS
// ============================================================

/** Worker skill levels */
UENUM()
enum class EWorkerRole : uint8
{
	Laborer,
	Carpenter,
	Electrician,
	Plumber,
	Supervisor
};

/** Worker entity (DATA_MODEL Section 2.4) */
struct INSTABUILTSIMULATION_API C_WorkerStats : public FComponentBase
{
	FString WorkerName;
	EWorkerRole Role = EWorkerRole::Laborer;
	float SkillLevel = 50.0f;       // 0-100
	float Fatigue = 0.0f;            // 0-100
	float Morale = 75.0f;            // 0-100
	float Productivity = 1.0f;       // Computed efficiency
	float HourlyWage = 25.0f;
	bool bIsAssigned = false;
	FEntityId AssignedSiteId;
	
	virtual FString GetTypeName() const override { return TEXT("WorkerStats"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << WorkerName;
		int32 RoleInt = (int32)Role; Ar << RoleInt; Role = (EWorkerRole)RoleInt;
		Ar << SkillLevel << Fatigue << Morale << Productivity << HourlyWage << bIsAssigned;
	}
};

// ============================================================
// MILESTONE 9: ECONOMY COMPONENTS
// ============================================================

/** Transaction record */
struct INSTABUILTSIMULATION_API C_Transaction : public FComponentBase
{
	FString Description;
	double Amount = 0.0;
	FString Category; // "Labor", "Materials", "Payment", etc.
	double Timestamp = 0.0;
	
	virtual FString GetTypeName() const override { return TEXT("Transaction"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Description << Amount << Category << Timestamp;
	}
};
