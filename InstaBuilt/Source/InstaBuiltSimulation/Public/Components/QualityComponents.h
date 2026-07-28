// QualityComponents.h — VS M1: Quality, Client Satisfaction, Budget/Timeline tracking
// Adds depth to the prototype: buildings have measurable quality, clients have feelings

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// BUILDING QUALITY (GSS Section 7.8: Quality System)
// ============================================================

/** Tracks quality per construction phase */
struct INSTABUILTSIMULATION_API C_BuildingQuality : public FComponentBase
{
	// Per-phase quality scores (0-100)
	float FoundationQuality = 100.0f;
	float StructureQuality = 100.0f;
	float InteriorQuality = 100.0f;
	float FinalizationQuality = 100.0f;
	
	// Overall computed quality
	float OverallQuality = 100.0f;
	
	// Quality notes (for inspection report)
	TArray<FString> Defects;
	
	void Recalculate()
	{
		OverallQuality = (FoundationQuality * 0.25f + StructureQuality * 0.35f
			+ InteriorQuality * 0.25f + FinalizationQuality * 0.15f);
	}
	
	void ApplyDefect(const FString& Phase, float Penalty, const FString& Description)
	{
		if (Phase == TEXT("Foundation")) FoundationQuality -= Penalty;
		else if (Phase == TEXT("Structure")) StructureQuality -= Penalty;
		else if (Phase == TEXT("Interior")) InteriorQuality -= Penalty;
		else if (Phase == TEXT("Finalization")) FinalizationQuality -= Penalty;
		Defects.Add(Description);
		Recalculate();
	}
	
	virtual FString GetTypeName() const override { return TEXT("BuildingQuality"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << FoundationQuality << StructureQuality << InteriorQuality << FinalizationQuality << OverallQuality;
	}
};

// ============================================================
// CLIENT SATISFACTION
// ============================================================

/** Client entity with expectations and satisfaction tracking */
struct INSTABUILTSIMULATION_API C_ClientData : public FComponentBase
{
	FString ClientName;
	FString CompanyName;
	
	// Expectations (set when contract created)
	float ExpectedQuality = 85.0f;
	int32 ExpectedTimeline = 90; // days
	double ExpectedBudget = 100000.0;
	
	// Satisfaction after delivery (computed)
	float QualitySatisfaction = 0.0f;   // 0-100
	float BudgetSatisfaction = 0.0f;    // 0-100
	float TimelineSatisfaction = 0.0f;  // 0-100
	float OverallSatisfaction = 0.0f;   // 0-100
	
	// Feedback
	FString Feedback;
	bool bWouldHireAgain = false;
	
	void CalculateSatisfaction(float ActualQuality, double ActualCost, int32 ActualDays, bool bOnTime)
	{
		QualitySatisfaction = FMath::Clamp(ActualQuality, 0.0f, 100.0f);
		BudgetSatisfaction = FMath::Clamp(100.0f - ((ActualCost - ExpectedBudget) / ExpectedBudget) * 50.0f, 0.0f, 100.0f);
		TimelineSatisfaction = bOnTime ? 95.0f : FMath::Clamp(100.0f - ((ActualDays - ExpectedTimeline) / (float)ExpectedTimeline) * 100.0f, 0.0f, 100.0f);
		OverallSatisfaction = QualitySatisfaction * 0.5f + BudgetSatisfaction * 0.25f + TimelineSatisfaction * 0.25f;
		bWouldHireAgain = OverallSatisfaction > 70.0f;
		
		// Generate feedback
		if (OverallSatisfaction > 90.0f)
			Feedback = TEXT("Exceptional work! We'll definitely recommend you.");
		else if (OverallSatisfaction > 75.0f)
			Feedback = TEXT("Very satisfied. The building meets our expectations.");
		else if (OverallSatisfaction > 60.0f)
			Feedback = TEXT("Acceptable work. A few things could have been better.");
		else if (OverallSatisfaction > 40.0f)
			Feedback = TEXT("Disappointed with several aspects. We expected more.");
		else
			Feedback = TEXT("Very unhappy. We will not be working with you again.");
	}
	
	virtual FString GetTypeName() const override { return TEXT("ClientData"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << ClientName << CompanyName;
		Ar << ExpectedQuality << ExpectedTimeline << ExpectedBudget;
		Ar << QualitySatisfaction << BudgetSatisfaction << TimelineSatisfaction << OverallSatisfaction;
		Ar << Feedback << bWouldHireAgain;
	}
};

// ============================================================
// PROJECT PERFORMANCE (budget/timeline tracking)
// ============================================================

/** Tracks budget and timeline performance for a project */
struct INSTABUILTSIMULATION_API C_ProjectPerformance : public FComponentBase
{
	double BudgetedCost = 0.0;
	double ActualCost = 0.0;
	double CostVariance = 0.0;      // Positive = under budget
	
	int32 BudgetedDays = 90;
	int32 ActualDays = 0;
	int32 DaysVariance = 0;          // Positive = ahead of schedule
	
	bool bOnBudget = true;
	bool bOnSchedule = true;
	
	// Performance rating
	float BudgetScore = 100.0f;     // 100 = on budget, lower = over
	float ScheduleScore = 100.0f;   // 100 = on time, lower = late
	
	void Finalize()
	{
		CostVariance = BudgetedCost - ActualCost;
		DaysVariance = BudgetedDays - ActualDays;
		bOnBudget = CostVariance >= 0;
		bOnSchedule = DaysVariance >= 0;
		
		BudgetScore = bOnBudget ? 100.0f :
			FMath::Clamp(100.0f - (FMath::Abs(CostVariance) / BudgetedCost) * 100.0f, 0.0f, 100.0f);
		ScheduleScore = bOnSchedule ? 100.0f :
			FMath::Clamp(100.0f - (FMath::Abs(DaysVariance) / (float)BudgetedDays) * 100.0f, 0.0f, 100.0f);
	}
	
	virtual FString GetTypeName() const override { return TEXT("ProjectPerformance"); }
	virtual void Serialize(FArchive& Ar) override
	{
		Ar << BudgetedCost << ActualCost << CostVariance;
		Ar << BudgetedDays << ActualDays << DaysVariance;
		Ar << bOnBudget << bOnSchedule << BudgetScore << ScheduleScore;
	}
};
