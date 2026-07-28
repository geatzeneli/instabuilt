// CompanyManager.cpp — VS M4: Full company management implementation

#include "Systems/CompanyManager.h"
#include "Logging/InstaBuiltLog.h"

FCompanyManager::FCompanyManager()
	: FInstaBuiltSystem(TEXT("CompanyManager"))
{
	AddDependency(TEXT("CompanySystem"));
}

void FCompanyManager::OnInitialize()
{
	CompanyId = FInstaBuiltECS::Get().GetEntitiesOfType(EEntityType::PlayerCompany).Num() > 0
		? FInstaBuiltECS::Get().GetEntitiesOfType(EEntityType::PlayerCompany)[0]
		: FEntityId::Invalid();
	
	// Attach reputation dashboard if not present
	if (CompanyId.IsValid() && !ECS.HasComponent<C_ReputationDashboard>(CompanyId))
	{
		ECS.AddComponent<C_ReputationDashboard>(CompanyId);
	}
	
	// Attach company history
	if (CompanyId.IsValid() && !ECS.HasComponent<C_CompanyHistory>(CompanyId))
	{
		ECS.AddComponent<C_CompanyHistory>(CompanyId);
	}
	
	IB_LOG_INFO("CompanyManager initialized. Company: %s", *CompanyId.ToString());
}

// ============================================================
// DASHBOARD
// ============================================================

FString FCompanyManager::GetFullDashboard() const
{
	FString Report;
	Report += TEXT("═══════════════════════════════════════════\n");
	Report += TEXT("         INSTABUILT COMPANY DASHBOARD        \n");
	Report += TEXT("═══════════════════════════════════════════\n\n");
	
	Report += GetFinancialOverview() + TEXT("\n");
	Report += GetReputationDashboard() + TEXT("\n");
	Report += GetEmployeeRoster() + TEXT("\n");
	Report += GetCompanyHistory() + TEXT("\n");
	
	Report += TEXT("═══════════════════════════════════════════\n");
	
	return Report;
}

FString FCompanyManager::GetHUDStatus() const
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	auto* Dash = ECS.GetComponent<C_ReputationDashboard>(CompanyId);
	auto* History = ECS.GetComponent<C_CompanyHistory>(CompanyId);
	
	return FString::Printf(TEXT("$%.0f | ⭐ %.0f | 📋 %d projects"),
		Finances ? Finances->CashOnHand : 0.0,
		Dash ? Dash->GetOverallRating() : 50.0f,
		History ? History->TotalProjectsCompleted : 0);
}

FString FCompanyManager::GetFinancialOverview() const
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	if (!Finances) return TEXT("No financial data.");
	
	double Valuation = EstimateCompanyValue();
	
	return FString::Printf(
		TEXT("┌─ FINANCIAL OVERVIEW ──────────────────────┐\n")
		TEXT("│  Cash on Hand:     %s\n")
		TEXT("│  Revenue (YTD):    %s\n")
		TEXT("│  Expenses (YTD):   %s\n")
		TEXT("│  Total Profit:     %s\n")
		TEXT("│  Company Value:    %s\n")
		TEXT("│  Projects Done:    %d\n")
		TEXT("└────────────────────────────────────────────┘"),
		*FormatMoney(Finances->CashOnHand),
		*FormatMoney(Finances->RevenueYTD),
		*FormatMoney(Finances->ExpensesYTD),
		*FormatMoney(Finances->TotalProfit),
		*FormatMoney(Valuation),
		Finances->ProjectsCompleted);
}

// ============================================================
// EMPLOYEES
// ============================================================

FEntityId FCompanyManager::HireEmployee(const FString& Name, const FString& Position, double AnnualSalary)
{
	FEntityId EmployeeId = ECS.CreateEntity(EEntityType::Employee);
	
	auto* Contract = ECS.AddComponent<C_EmployeeContract>(EmployeeId);
	Contract->Position = Position;
	Contract->ContractType = TEXT("Full-time");
	Contract->AnnualSalary = AnnualSalary;
	Contract->CalculateHourlyRate();
	Contract->CalculateEfficiency();
	
	// Also add worker stats for construction contribution
	auto* Stats = ECS.AddComponent<C_WorkerStats>(EmployeeId);
	Stats->WorkerName = Name;
	Stats->HourlyWage = Contract->HourlyRate;
	Stats->SkillLevel = 50.0f + FMath::RandRange(-15.0f, 15.0f);
	
	Employees.Add(EmployeeId);
	
	IB_LOG_INFO("Employee hired: %s — %s ($%.0f/yr)", *Name, *Position, AnnualSalary);
	return EmployeeId;
}

void FCompanyManager::TerminateEmployee(FEntityId EmployeeId)
{
	auto* Contract = ECS.GetComponent<C_EmployeeContract>(EmployeeId);
	if (Contract) Contract->bIsActive = false;
	Employees.Remove(EmployeeId);
	IB_LOG_INFO("Employee terminated: %s", *EmployeeId.ToString());
}

void FCompanyManager::RateEmployee(FEntityId EmployeeId, float PerformanceScore)
{
	auto* Contract = ECS.GetComponent<C_EmployeeContract>(EmployeeId);
	if (Contract)
	{
		Contract->PerformanceRating = FMath::Clamp(PerformanceScore, 0.0f, 100.0f);
		Contract->ProjectsWorked++;
		Contract->CalculateEfficiency();
	}
}

FString FCompanyManager::GetEmployeeRoster() const
{
	FString Roster;
	Roster += FString::Printf(TEXT("┌─ EMPLOYEES (%d) ─────────────────────────────┐\n"), Employees.Num());
	
	for (auto& EmpId : Employees)
	{
		auto* Contract = ECS.GetComponent<C_EmployeeContract>(EmpId);
		auto* Stats = ECS.GetComponent<C_WorkerStats>(EmpId);
		
		if (Contract && Stats)
		{
			Roster += FString::Printf(TEXT("│  %-20s | %-18s | %.0f%% eff\n"),
				*Stats->WorkerName, *Contract->Position, Contract->Efficiency * 100.0f);
		}
	}
	
	double Payroll = GetMonthlyPayroll();
	Roster += FString::Printf(TEXT("│  Monthly Payroll: %s\n"), *FormatMoney(Payroll));
	Roster += TEXT("└────────────────────────────────────────────┘\n");
	
	return Roster;
}

double FCompanyManager::GetMonthlyPayroll() const
{
	double Total = 0.0;
	for (auto& EmpId : Employees)
	{
		auto* Contract = ECS.GetComponent<C_EmployeeContract>(EmpId);
		if (Contract && Contract->bIsActive)
		{
			Total += Contract->AnnualSalary / 12.0;
		}
	}
	return Total;
}

// ============================================================
// PROJECT FINANCE
// ============================================================

FEntityId FCompanyManager::CreateProjectFinance(FEntityId ProjectId, double Budget)
{
	auto* Finance = ECS.AddComponent<C_ProjectFinance>(ProjectId);
	Finance->TotalBudget = Budget;
	Finance->ContingencyReserve = Budget * 0.10;
	Finance->Recalculate();
	
	return ProjectId;
}

void FCompanyManager::RecordProjectCost(FEntityId ProjectId, const FString& Category, double Amount)
{
	auto* Finance = ECS.GetComponent<C_ProjectFinance>(ProjectId);
	if (Finance)
	{
		Finance->AddCost(Category, Amount);
	}
}

FEntityId FCompanyManager::FinalizeProject(FEntityId ProjectId, const FString& ProjectName,
	const FString& ClientName, float Quality, bool bOnTime)
{
	auto* Finance = ECS.GetComponent<C_ProjectFinance>(ProjectId);
	auto* Perf = ECS.GetComponent<C_ProjectPerformance>(ProjectId);
	
	if (!Finance) return FEntityId::Invalid();
	
	// Create history record
	FEntityId RecordId = ECS.CreateEntity(EEntityType::None);
	auto* Record = ECS.AddComponent<C_ProjectRecord>(RecordId);
	Record->ProjectName = ProjectName;
	Record->ClientName = ClientName;
	Record->Revenue = Finance->TotalBudget;
	Record->Cost = Finance->TotalActualCost;
	Record->Profit = Finance->Profit;
	Record->QualityScore = Quality;
	Record->bOnTime = bOnTime;
	Record->bOnBudget = !Finance->bIsOverBudget;
	
	// Update company history
	auto* History = ECS.GetComponent<C_CompanyHistory>(CompanyId);
	if (History)
	{
		History->AddProject(RecordId, Finance->TotalBudget, Finance->Profit, Quality, bOnTime, !Finance->bIsOverBudget);
	}
	
	CompletedProjectRecords.Add(RecordId);
	
	IB_LOG_INFO("Project finalized: %s | Profit: $%.0f | Quality: %.0f%%",
		*ProjectName, Finance->Profit, Quality);
	
	return RecordId;
}

FString FCompanyManager::GetProjectFinanceReport(FEntityId ProjectId) const
{
	auto* Finance = ECS.GetComponent<C_ProjectFinance>(ProjectId);
	if (!Finance) return TEXT("No finance data.");
	
	return FString::Printf(
		TEXT("┌─ PROJECT FINANCE ──────────────────────────┐\n")
		TEXT("│  Budget:          %s\n")
		TEXT("│  Labor:           %s\n")
		TEXT("│  Materials:       %s\n")
		TEXT("│  Equipment:       %s\n")
		TEXT("│  Overhead:        %s\n")
		TEXT("│  ─────────────────────────────────────\n")
		TEXT("│  Total Actual:    %s\n")
		TEXT("│  Profit:          %s (%.1f%%)\n")
		TEXT("│  Status:          %s\n")
		TEXT("└────────────────────────────────────────────┘"),
		*FormatMoney(Finance->TotalBudget),
		*FormatMoney(Finance->LaborCost),
		*FormatMoney(Finance->MaterialCost),
		*FormatMoney(Finance->EquipmentCost),
		*FormatMoney(Finance->OverheadCost),
		*FormatMoney(Finance->TotalActualCost),
		*FormatMoney(Finance->Profit), Finance->ProfitMargin,
		Finance->bIsOverBudget ? TEXT("⚠ OVER BUDGET") : TEXT("✅ On Budget"));
}

// ============================================================
// REPUTATION
// ============================================================

void FCompanyManager::UpdateReputation(float QualityScore, float ReliabilityScore,
	float InnovationScore, float CommunityScore, float SafetyScore)
{
	auto* Dash = ECS.GetComponent<C_ReputationDashboard>(CompanyId);
	if (Dash)
	{
		Dash->UpdateAfterProject(QualityScore, ReliabilityScore, InnovationScore, CommunityScore, SafetyScore);
		
		// Also update the basic reputation component
		auto* Rep = ECS.GetComponent<C_Reputation>(CompanyId);
		if (Rep)
		{
			Rep->QualityScore = Dash->Quality;
			Rep->ReliabilityScore = Dash->Reliability;
			Rep->InnovationScore = Dash->Innovation;
			Rep->CommunityScore = Dash->Community;
			Rep->SafetyScore = Dash->Safety;
			Rep->RecalculateOverall();
		}
	}
}

FString FCompanyManager::GetReputationDashboard() const
{
	auto* Dash = ECS.GetComponent<C_ReputationDashboard>(CompanyId);
	if (!Dash) return TEXT("No reputation data.");
	
	return Dash->GetDashboard();
}

// ============================================================
// HISTORY
// ============================================================

FString FCompanyManager::GetCompanyHistory() const
{
	auto* History = ECS.GetComponent<C_CompanyHistory>(CompanyId);
	if (!History) return TEXT("No company history.");
	
	FString Report;
	Report += TEXT("┌─ PROJECT HISTORY ──────────────────────────┐\n");
	
	// Show last 5 projects
	int32 Start = FMath::Max(0, CompletedProjectRecords.Num() - 5);
	for (int32 i = Start; i < CompletedProjectRecords.Num(); ++i)
	{
		auto* Record = ECS.GetComponent<C_ProjectRecord>(CompletedProjectRecords[i]);
		if (Record)
		{
			FString Status = Record->bOnTime ? TEXT("✅") : TEXT("⏰");
			Status += Record->bOnBudget ? TEXT("💰") : TEXT("💸");
			Report += FString::Printf(TEXT("│  %s %s — $%.0f profit\n"),
				*Status, *Record->ProjectName, Record->Profit);
		}
	}
	
	Report += TEXT("│  ─────────────────────────────────────\n");
	Report += FString::Printf(TEXT("│  %s\n"), *History->GetSummary());
	Report += TEXT("└────────────────────────────────────────────┘\n");
	
	return Report;
}

double FCompanyManager::EstimateCompanyValue() const
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	auto* History = ECS.GetComponent<C_CompanyHistory>(CompanyId);
	
	double BaseValue = Finances ? Finances->CashOnHand : 0.0;
	
	// Revenue multiple based on track record
	double RevenueMultiple = History ? FMath::Min(History->TotalProjectsCompleted * 0.5, 10.0) : 1.0;
	double ProfitValue = Finances ? Finances->TotalProfit * RevenueMultiple : 0.0;
	
	// Reputation modifier
	auto* Dash = ECS.GetComponent<C_ReputationDashboard>(CompanyId);
	double RepModifier = Dash ? (0.5 + Dash->GetOverallRating() / 200.0) : 1.0;
	
	return BaseValue + ProfitValue * RepModifier;
}

// ============================================================
// HELPERS
// ============================================================

FString FCompanyManager::FormatMoney(double Amount) const
{
	if (Amount >= 1000000.0)
		return FString::Printf(TEXT("$%.2fM"), Amount / 1000000.0);
	if (Amount >= 1000.0)
		return FString::Printf(TEXT("$%.0fK"), Amount / 1000.0);
	return FString::Printf(TEXT("$%.0f"), Amount);
}

FString FCompanyManager::ProgressBar(float Percent, int32 Width) const
{
	int32 Filled = FMath::RoundToInt((Percent / 100.0f) * Width);
	FString Bar;
	for (int32 i = 0; i < Width; ++i) Bar += (i < Filled) ? TEXT("█") : TEXT("░");
	return FString::Printf(TEXT("[%s] %.0f%%"), *Bar, Percent);
}
