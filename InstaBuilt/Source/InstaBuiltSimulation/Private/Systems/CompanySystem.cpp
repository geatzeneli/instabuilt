// CompanySystem.cpp — M4: Company creation, finances, reputation

#include "Systems/CompanySystem.h"
#include "Systems/InstaBuiltEvents.h"
#include "Logging/InstaBuiltLog.h"

void FCompanySystem::OnInitialize()
{
	IB_LOG_INFO("CompanySystem initialized.");
}

FEntityId FCompanySystem::CreateCompany(const FString& Name, double StartingMoney)
{
	CompanyId = ECS.CreateEntity(EEntityType::PlayerCompany);
	
	auto* Identity = ECS.AddComponent<C_CompanyIdentity>(CompanyId);
	Identity->CompanyName = Name;
	Identity->CompanyTier = 1;
	
	auto* Finances = ECS.AddComponent<C_Financials>(CompanyId);
	Finances->CashOnHand = StartingMoney;
	
	auto* Rep = ECS.AddComponent<C_Reputation>(CompanyId);
	
	EventBus.Publish<FCompanyCreatedEvent>(CompanyId, Name, StartingMoney);
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("Welcome to %s! Starting funds: $%.0f"), *Name, StartingMoney),
		TEXT("Success"));
	
	IB_LOG_INFO("Company created: %s with $%.0f", *Name, StartingMoney);
	return CompanyId;
}

double FCompanySystem::GetCash() const
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	return Finances ? Finances->CashOnHand : 0.0;
}

bool FCompanySystem::SpendMoney(double Amount, const FString& Reason)
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	if (!Finances || Finances->CashOnHand < Amount) return false;
	
	double OldCash = Finances->CashOnHand;
	Finances->CashOnHand -= Amount;
	Finances->ExpensesYTD += Amount;
	
	EventBus.Publish<FMoneyChangedEvent>(CompanyId, OldCash, Finances->CashOnHand, -Amount, Reason);
	IB_LOG_INFO("Spent $%.0f: %s (Balance: $%.0f)", Amount, *Reason, Finances->CashOnHand);
	return true;
}

void FCompanySystem::ReceiveMoney(double Amount, const FString& Reason)
{
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	if (!Finances) return;
	
	double OldCash = Finances->CashOnHand;
	Finances->CashOnHand += Amount;
	Finances->RevenueYTD += Amount;
	Finances->TotalProfit += Amount;
	
	EventBus.Publish<FMoneyChangedEvent>(CompanyId, OldCash, Finances->CashOnHand, +Amount, Reason);
	IB_LOG_INFO("Received $%.0f: %s (Balance: $%.0f)", Amount, *Reason, Finances->CashOnHand);
}

const C_Reputation* FCompanySystem::GetReputation() const
{
	return ECS.GetComponent<C_Reputation>(CompanyId);
}

void FCompanySystem::AddProjectReputation(float Quality, bool bOnTime)
{
	auto* Rep = ECS.GetComponent<C_Reputation>(CompanyId);
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	if (!Rep || !Finances) return;
	
	Finances->ProjectsCompleted++;
	
	Rep->QualityScore = FMath::Clamp(Rep->QualityScore + Quality * 0.1f - 2.5f, 0.0f, 100.0f);
	Rep->ReliabilityScore = FMath::Clamp(Rep->ReliabilityScore + (bOnTime ? 5.0f : -10.0f), 0.0f, 100.0f);
	Rep->RecalculateOverall();
	
	IB_LOG_INFO("Reputation updated: Overall %.1f (Quality: %.1f, Reliability: %.1f)",
		Rep->OverallRating, Rep->QualityScore, Rep->ReliabilityScore);
}

void FCompanySystem::GetDashboardData(FString& OutName, FString& OutTier,
	FString& OutCash, FString& OutReputation, FString& OutProjects) const
{
	auto* Identity = ECS.GetComponent<C_CompanyIdentity>(CompanyId);
	auto* Finances = ECS.GetComponent<C_Financials>(CompanyId);
	auto* Rep = ECS.GetComponent<C_Reputation>(CompanyId);
	
	OutName = Identity ? Identity->CompanyName : TEXT("Unknown");
	OutTier = Identity ? FString::Printf(TEXT("Tier %d"), Identity->CompanyTier) : TEXT("Tier ?");
	OutCash = Finances ? FString::Printf(TEXT("$%.0f"), Finances->CashOnHand) : TEXT("$0");
	OutReputation = Rep ? FString::Printf(TEXT("%.1f%%"), Rep->OverallRating) : TEXT("N/A");
	OutProjects = Finances ? FString::Printf(TEXT("%d"), Finances->ProjectsCompleted) : TEXT("0");
}

void FCompanySystem::PrintStatus() const
{
	FString Name, Tier, Cash, Rep, Projects;
	GetDashboardData(Name, Tier, Cash, Rep, Projects);
	IB_LOG_INFO("=== %s Status ===", *Name);
	IB_LOG_INFO("  Tier: %s | Cash: %s | Rep: %s | Projects: %s",
		*Tier, *Cash, *Rep, *Projects);
}
