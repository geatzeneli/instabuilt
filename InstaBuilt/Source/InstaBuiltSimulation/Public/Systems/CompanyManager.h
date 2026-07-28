// CompanyManager.h — VS M4: Full company management system
// Dashboard, employee lifecycle, project finance, reputation

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/CompanyComponents.h"
#include "Components/CompanyLayer.h"
#include "Components/QualityComponents.h"

class INSTABUILTSIMULATION_API FCompanyManager : public FInstaBuiltSystem
{
public:
	FCompanyManager();
	virtual void OnInitialize() override;
	
	// ============================================================
	// COMPANY DASHBOARD
	// ============================================================
	
	/** Get complete company dashboard as formatted string */
	FString GetFullDashboard() const;
	
	/** Get compact status for HUD */
	FString GetHUDStatus() const;
	
	/** Get financial overview */
	FString GetFinancialOverview() const;
	
	// ============================================================
	// EMPLOYEE MANAGEMENT
	// ============================================================
	
	/** Hire an employee with a formal contract */
	FEntityId HireEmployee(const FString& Name, const FString& Position, double AnnualSalary);
	
	/** Fire an employee */
	void TerminateEmployee(FEntityId EmployeeId);
	
	/** Update performance after a project */
	void RateEmployee(FEntityId EmployeeId, float PerformanceScore);
	
	/** Get all employees with their status */
	FString GetEmployeeRoster() const;
	
	/** Calculate total monthly payroll */
	double GetMonthlyPayroll() const;
	
	// ============================================================
	// PROJECT FINANCE
	// ============================================================
	
	/** Create a finance tracker for a new project */
	FEntityId CreateProjectFinance(FEntityId ProjectId, double Budget);
	
	/** Record a cost against a project */
	void RecordProjectCost(FEntityId ProjectId, const FString& Category, double Amount);
	
	/** Finalize project finances and create a history record */
	FEntityId FinalizeProject(FEntityId ProjectId, const FString& ProjectName,
		const FString& ClientName, float Quality, bool bOnTime);
	
	/** Get project finance summary */
	FString GetProjectFinanceReport(FEntityId ProjectId) const;
	
	// ============================================================
	// REPUTATION
	// ============================================================
	
	/** Update reputation after project completion */
	void UpdateReputation(float QualityScore, float ReliabilityScore,
		float InnovationScore = 50.0f, float CommunityScore = 50.0f, float SafetyScore = 80.0f);
	
	/** Get reputation dashboard string */
	FString GetReputationDashboard() const;
	
	// ============================================================
	// PERFORMANCE HISTORY
	// ============================================================
	
	/** Get company history summary */
	FString GetCompanyHistory() const;
	
	/** Get company valuation estimate */
	double EstimateCompanyValue() const;
	
private:
	FEntityId CompanyId;
	TArray<FEntityId> Employees;
	TArray<FEntityId> CompletedProjectRecords;
	
	FString FormatMoney(double Amount) const;
	FString ProgressBar(float Percent, int32 Width = 20) const;
};
