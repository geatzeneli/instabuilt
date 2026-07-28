// CompanyLayer.h — VS M4: Employee contracts, project finance, reputation dashboard
// GSS Section 8: Company Management Workflow
// GSS Section 12: Reputation System (5-axis)

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// EMPLOYEE CONTRACT (expands C_WorkerStats from prototype)
// ============================================================

/** Formal employment contract with terms */
struct INSTABUILTSIMULATION_API C_EmployeeContract : public FComponentBase
{
	FString Position;              // "Junior Architect", "Site Supervisor", etc.
	FString ContractType;          // "Full-time", "Part-time", "Contract"
	
	double AnnualSalary = 45000.0;
	double HourlyRate = 22.0;
	int32 WeeklyHours = 40;
	
	// Benefits
	bool bHasHealthInsurance = false;
	bool bHasRetirementPlan = false;
	int32 PaidTimeOffDays = 10;
	
	// Performance
	float PerformanceRating = 75.0f;  // 0-100
	int32 ProjectsWorked = 0;
	float Efficiency = 1.0f;          // Productivity multiplier
	
	// Tenure
	int32 DaysEmployed = 0;
	bool bIsActive = true;
	
	void CalculateHourlyRate()
	{
		HourlyRate = AnnualSalary / (52.0 * WeeklyHours);
	}
	
	void CalculateEfficiency()
	{
		Efficiency = FMath::Clamp(0.5f + (PerformanceRating / 200.0f), 0.5f, 1.5f);
	}
	
	virtual FString GetTypeName() const override { return TEXT("EmployeeContract"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Position << ContractType << AnnualSalary << HourlyRate << WeeklyHours;
		Ar << bHasHealthInsurance << bHasRetirementPlan << PaidTimeOffDays;
		Ar << PerformanceRating << ProjectsWorked << Efficiency << DaysEmployed << bIsActive;
	}
};

// ============================================================
// PROJECT FINANCE REPORT
// ============================================================

/** Detailed cost breakdown for a single project */
struct INSTABUILTSIMULATION_API C_ProjectFinance : public FComponentBase
{
	// Budget
	double TotalBudget = 0.0;
	double ContingencyReserve = 0.0;   // 10% of budget typically
	
	// Actual costs (accumulated during construction)
	double LaborCost = 0.0;
	double MaterialCost = 0.0;
	double EquipmentCost = 0.0;
	double PermitCost = 0.0;
	double SubcontractorCost = 0.0;
	double OverheadCost = 0.0;
	
	// Computed
	double TotalActualCost = 0.0;
	double Profit = 0.0;
	double ProfitMargin = 0.0;          // Percentage
	bool bIsOverBudget = false;
	double OverBudgetAmount = 0.0;
	
	void Recalculate()
	{
		TotalActualCost = LaborCost + MaterialCost + EquipmentCost + PermitCost + SubcontractorCost + OverheadCost;
		Profit = TotalBudget - TotalActualCost;
		ProfitMargin = TotalBudget > 0 ? (Profit / TotalBudget) * 100.0 : 0.0;
		bIsOverBudget = TotalActualCost > TotalBudget;
		OverBudgetAmount = bIsOverBudget ? TotalActualCost - TotalBudget : 0.0;
	}
	
	/** Add a cost entry to the appropriate category */
	void AddCost(const FString& Category, double Amount)
	{
		if (Category == TEXT("Labor")) LaborCost += Amount;
		else if (Category == TEXT("Material")) MaterialCost += Amount;
		else if (Category == TEXT("Equipment")) EquipmentCost += Amount;
		else if (Category == TEXT("Permit")) PermitCost += Amount;
		else if (Category == TEXT("Subcontractor")) SubcontractorCost += Amount;
		else OverheadCost += Amount;
		Recalculate();
	}
	
	/** Get a formatted financial summary */
	FString GetSummary() const
	{
		return FString::Printf(
			TEXT("Budget: $%.0f | Actual: $%.0f | Profit: $%.0f (%.1f%%) | %s"),
			TotalBudget, TotalActualCost, Profit, ProfitMargin,
			bIsOverBudget ? TEXT("⚠ OVER BUDGET") : TEXT("✅ On Budget"));
	}
	
	virtual FString GetTypeName() const override { return TEXT("ProjectFinance"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << TotalBudget << ContingencyReserve;
		Ar << LaborCost << MaterialCost << EquipmentCost << PermitCost << SubcontractorCost << OverheadCost;
		Ar << TotalActualCost << Profit << ProfitMargin << bIsOverBudget << OverBudgetAmount;
	}
};

// ============================================================
// COMPANY PERFORMANCE HISTORY
// ============================================================

/** A single completed project record for the company history */
struct INSTABUILTSIMULATION_API C_ProjectRecord : public FComponentBase
{
	FString ProjectName;
	FString ClientName;
	FString BuildingType;
	
	double Revenue = 0.0;
	double Cost = 0.0;
	double Profit = 0.0;
	
	float QualityScore = 0.0f;
	bool bOnTime = false;
	bool bOnBudget = false;
	
	int32 CompletionDay = 0;
	FString ClientFeedback;
	
	virtual FString GetTypeName() const override { return TEXT("ProjectRecord"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ProjectName << ClientName << BuildingType;
		Ar << Revenue << Cost << Profit;
		Ar << QualityScore << bOnTime << bOnBudget << CompletionDay << ClientFeedback;
	}
};

/** Company-wide performance history */
struct INSTABUILTSIMULATION_API C_CompanyHistory : public FComponentBase
{
	TArray<FEntityId> CompletedProjects;    // Entity references to project records
	
	double TotalRevenue = 0.0;
	double TotalProfit = 0.0;
	double AverageQuality = 0.0f;
	float OnTimePercentage = 0.0f;
	float OnBudgetPercentage = 0.0f;
	int32 TotalProjectsCompleted = 0;
	
	/** Add a completed project to the history */
	void AddProject(FEntityId ProjectRecordId, double Revenue, double Profit, float Quality, bool bOnTime, bool bOnBudget)
	{
		CompletedProjects.Add(ProjectRecordId);
		TotalProjectsCompleted++;
		TotalRevenue += Revenue;
		TotalProfit += Profit;
		
		// Rolling averages
		AverageQuality = ((AverageQuality * (TotalProjectsCompleted - 1)) + Quality) / TotalProjectsCompleted;
		float OldOnTime = OnTimePercentage * (TotalProjectsCompleted - 1);
		float OldOnBudget = OnBudgetPercentage * (TotalProjectsCompleted - 1);
		OnTimePercentage = (OldOnTime + (bOnTime ? 100.0f : 0.0f)) / TotalProjectsCompleted;
		OnBudgetPercentage = (OldOnBudget + (bOnBudget ? 100.0f : 0.0f)) / TotalProjectsCompleted;
	}
	
	/** Get formatted history summary */
	FString GetSummary() const
	{
		return FString::Printf(
			TEXT("Projects: %d | Revenue: $%.0f | Profit: $%.0f\n")
			TEXT("Avg Quality: %.1f%% | On-Time: %.0f%% | On-Budget: %.0f%%"),
			TotalProjectsCompleted, TotalRevenue, TotalProfit,
			AverageQuality, OnTimePercentage, OnBudgetPercentage);
	}
	
	virtual FString GetTypeName() const override { return TEXT("CompanyHistory"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << TotalRevenue << TotalProfit << AverageQuality << OnTimePercentage << OnBudgetPercentage << TotalProjectsCompleted;
	}
};

// ============================================================
// REPUTATION DASHBOARD (5-axis — expands C_Reputation)
// ============================================================

/** Extended reputation with trend tracking */
struct INSTABUILTSIMULATION_API C_ReputationDashboard : public FComponentBase
{
	// Current scores (0-100)
	float Quality = 50.0f;
	float Reliability = 50.0f;
	float Innovation = 50.0f;
	float Community = 50.0f;
	float Safety = 80.0f;   // Starts higher — no incidents yet
	
	// Historical trends
	float PreviousQuality = 50.0f;
	float PreviousReliability = 50.0f;
	float PreviousInnovation = 50.0f;
	float PreviousCommunity = 50.0f;
	float PreviousSafety = 80.0f;
	
	// Change indicators (positive = improving)
	float QualityTrend = 0.0f;
	float ReliabilityTrend = 0.0f;
	float InnovationTrend = 0.0f;
	float CommunityTrend = 0.0f;
	float SafetyTrend = 0.0f;
	
	/** Update with new project results */
	void UpdateAfterProject(float NewQuality, float NewReliability, float NewInnovation, float NewCommunity, float NewSafety)
	{
		PreviousQuality = Quality;
		PreviousReliability = Reliability;
		PreviousInnovation = Innovation;
		PreviousCommunity = Community;
		PreviousSafety = Safety;
		
		// Smooth toward new values
		Quality = FMath::Clamp(Quality * 0.7f + NewQuality * 0.3f, 0.0f, 100.0f);
		Reliability = FMath::Clamp(Reliability * 0.7f + NewReliability * 0.3f, 0.0f, 100.0f);
		Innovation = FMath::Clamp(Innovation * 0.7f + NewInnovation * 0.3f, 0.0f, 100.0f);
		Community = FMath::Clamp(Community * 0.7f + NewCommunity * 0.3f, 0.0f, 100.0f);
		Safety = FMath::Clamp(Safety * 0.7f + NewSafety * 0.3f, 0.0f, 100.0f);
		
		// Calculate trends
		QualityTrend = Quality - PreviousQuality;
		ReliabilityTrend = Reliability - PreviousReliability;
		InnovationTrend = Innovation - PreviousInnovation;
		CommunityTrend = Community - PreviousCommunity;
		SafetyTrend = Safety - PreviousSafety;
	}
	
	/** Get overall rating (weighted) */
	float GetOverallRating() const
	{
		return Quality * 0.30f + Reliability * 0.25f + Innovation * 0.15f + Community * 0.15f + Safety * 0.15f;
	}
	
	/** Get trend indicator symbol */
	static FString TrendSymbol(float Trend)
	{
		if (Trend > 1.0f) return TEXT("▲");
		if (Trend < -1.0f) return TEXT("▼");
		return TEXT("—");
	}
	
	/** Get full dashboard string */
	FString GetDashboard() const
	{
		return FString::Printf(
			TEXT("REPUTATION DASHBOARD\n")
			TEXT("  Quality:     %.0f %s\n")
			TEXT("  Reliability: %.0f %s\n")
			TEXT("  Innovation:  %.0f %s\n")
			TEXT("  Community:   %.0f %s\n")
			TEXT("  Safety:      %.0f %s\n")
			TEXT("  ─────────────────\n")
			TEXT("  OVERALL:     %.1f"),
			Quality, *TrendSymbol(QualityTrend),
			Reliability, *TrendSymbol(ReliabilityTrend),
			Innovation, *TrendSymbol(InnovationTrend),
			Community, *TrendSymbol(CommunityTrend),
			Safety, *TrendSymbol(SafetyTrend),
			GetOverallRating());
	}
	
	virtual FString GetTypeName() const override { return TEXT("ReputationDashboard"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Quality << Reliability << Innovation << Community << Safety;
		Ar << PreviousQuality << PreviousReliability << PreviousInnovation << PreviousCommunity << PreviousSafety;
		Ar << QualityTrend << ReliabilityTrend << InnovationTrend << CommunityTrend << SafetyTrend;
	}
};
