// WorkerEconomy.cpp — M8+M9: Worker hiring + Economy costs/payments

#include "Systems/WorkerEconomy.h"
#include "Systems/CompanySystem.h"
#include "Systems/InstaBuiltEvents.h"
#include "Logging/InstaBuiltLog.h"

// ============================================================
// WORKER SYSTEM
// ============================================================

FWorkerSystem::FWorkerSystem()
	: FInstaBuiltSystem(TEXT("WorkerSystem"))
{
	AddDependency(TEXT("CompanySystem"));
}

void FWorkerSystem::OnUpdate(float DeltaTime)
{
	for (auto& WorkerId : Workers)
	{
		UpdateWorkerProductivity(WorkerId);
	}
}

FEntityId FWorkerSystem::HireWorker(const FString& Name, EWorkerRole Role, float Wage)
{
	FEntityId WorkerId = ECS.CreateEntity(EEntityType::Worker);
	
	auto* Stats = ECS.AddComponent<C_WorkerStats>(WorkerId);
	Stats->WorkerName = Name;
	Stats->Role = Role;
	Stats->HourlyWage = Wage;
	Stats->SkillLevel = 50.0f + FMath::RandRange(-20.0f, 20.0f);
	Stats->Morale = 75.0f;
	
	Workers.Add(WorkerId);
	
	EventBus.Publish<FWorkerHiredEvent>(WorkerId, Name, Role);
	IB_LOG_INFO("Worker hired: %s (%s, $%.0f/hr)", *Name,
		Role == EWorkerRole::Laborer ? TEXT("Laborer") : TEXT("Specialist"), Wage);
	
	return WorkerId;
}

bool FWorkerSystem::AssignToSite(FEntityId WorkerId, FEntityId SiteId)
{
	auto* Stats = ECS.GetComponent<C_WorkerStats>(WorkerId);
	if (!Stats) return false;
	
	Stats->bIsAssigned = true;
	Stats->AssignedSiteId = SiteId;
	
	EventBus.Publish<FWorkerAssignedEvent>(WorkerId, SiteId);
	IB_LOG_DEBUG("Worker %s assigned to site %s", *Stats->WorkerName, *SiteId.ToString());
	return true;
}

TArray<FEntityId> FWorkerSystem::GetAllWorkers() const { return Workers; }

FString FWorkerSystem::GetWorkerInfo(FEntityId WorkerId) const
{
	auto* Stats = ECS.GetComponent<C_WorkerStats>(WorkerId);
	if (!Stats) return TEXT("Unknown worker");
	
	return FString::Printf(TEXT("%s | Skill: %.0f | Morale: %.0f | Assigned: %s"),
		*Stats->WorkerName, Stats->SkillLevel, Stats->Morale,
		Stats->bIsAssigned ? TEXT("Yes") : TEXT("No"));
}

double FWorkerSystem::GetDailyLaborCost() const
{
	double Total = 0.0;
	for (auto& WorkerId : Workers)
	{
		auto* Stats = ECS.GetComponent<C_WorkerStats>(WorkerId);
		if (Stats) Total += Stats->HourlyWage * 8.0; // 8-hour day
	}
	return Total;
}

void FWorkerSystem::UpdateWorkerProductivity(FEntityId WorkerId)
{
	auto* Stats = ECS.GetComponent<C_WorkerStats>(WorkerId);
	if (!Stats || !Stats->bIsAssigned) return;
	
	// Productivity = skill × morale × fatigue penalty
	float SkillFactor = Stats->SkillLevel / 100.0f;
	float MoraleFactor = 0.5f + (Stats->Morale / 200.0f);
	Stats->Productivity = SkillFactor * MoraleFactor;
	
	// Slowly increase fatigue
	Stats->Fatigue = FMath::Min(Stats->Fatigue + 0.1f, 100.0f);
	if (Stats->Fatigue > 80.0f)
	{
		Stats->Productivity *= 0.7f;
		Stats->Morale -= 0.5f;
	}
}

// ============================================================
// ECONOMY SYSTEM
// ============================================================

FEconomySystem::FEconomySystem()
	: FInstaBuiltSystem(TEXT("EconomySystem"))
{
	AddDependency(TEXT("CompanySystem"));
	AddDependency(TEXT("ContractSystem"));
	AddDependency(TEXT("ConstructionSystem"));
	SetUpdateInterval(1.0f);
}

void FEconomySystem::OnInitialize()
{
	// Find CompanySystem for money operations
	auto Sys = FSystemOrchestrator::Get().GetSystem<FCompanySystem>(TEXT("CompanySystem"));
	CompanySystem = Sys.Get();
	IB_LOG_INFO("EconomySystem initialized. Company link: %s", CompanySystem ? TEXT("OK") : TEXT("MISSING"));
}

void FEconomySystem::OnUpdate(float DeltaTime)
{
	DayAccumulator += DeltaTime;
	if (DayAccumulator >= DayDuration)
	{
		DayAccumulator -= DayDuration;
		ProcessDailyCosts();
	}
}

bool FEconomySystem::ChargeConstructionCosts(FEntityId SiteId)
{
	if (!CompanySystem) return false;
	
	double MaterialCost = 500.0;  // Placeholder: per-tick material cost
	double LaborCost = 200.0;     // Placeholder: per-tick labor cost
	double Total = MaterialCost + LaborCost;
	
	return CompanySystem->SpendMoney(Total, FString::Printf(TEXT("Construction costs (Site %s)"), *SiteId.ToString()));
}

void FEconomySystem::ProcessContractPayment(FEntityId ContractId, double Amount)
{
	if (!CompanySystem) return;
	CompanySystem->ReceiveMoney(Amount, TEXT("Contract completion payment"));
	TotalProfit += Amount;
}

void FEconomySystem::ProcessDailyCosts()
{
	if (!CompanySystem) return;
	
	// Daily worker wages
	auto WorkerSys = FSystemOrchestrator::Get().GetSystem<FWorkerSystem>(TEXT("WorkerSystem"));
	double LaborCost = WorkerSys.IsValid() ? WorkerSys->GetDailyLaborCost() : 0.0;
	if (LaborCost > 0)
	{
		CompanySystem->SpendMoney(LaborCost, TEXT("Daily labor costs"));
	}
	
	// Daily overhead
	CompanySystem->SpendMoney(150.0, TEXT("Daily overhead (utilities, insurance, etc.)"));
}
