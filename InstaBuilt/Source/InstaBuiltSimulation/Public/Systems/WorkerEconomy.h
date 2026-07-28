// WorkerSystem.h — Worker hiring, assignment, contribution (M8)
// EconomySystem.h — Costs, payments, profit tracking (M9)

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/CompanyComponents.h"

// ============================================================
// WORKER SYSTEM (M8)
// ============================================================

class FWorkerSystem : public FInstaBuiltSystem
{
public:
	FWorkerSystem();
	
	virtual void OnUpdate(float DeltaTime) override;
	
	/** Hire a new worker */
	FEntityId HireWorker(const FString& Name, EWorkerRole Role, float Wage);
	
	/** Assign worker to a construction site */
	bool AssignToSite(FEntityId WorkerId, FEntityId SiteId);
	
	/** Get all hired workers */
	TArray<FEntityId> GetAllWorkers() const;
	
	/** Get worker info for UI */
	FString GetWorkerInfo(FEntityId WorkerId) const;
	
	/** Calculate total daily labor cost */
	double GetDailyLaborCost() const;
	
private:
	TArray<FEntityId> Workers;
	void UpdateWorkerProductivity(FEntityId WorkerId);
};

// ============================================================
// ECONOMY SYSTEM (M9)
// ============================================================

class FEconomySystem : public FInstaBuiltSystem
{
public:
	FEconomySystem();
	
	virtual void OnInitialize() override;
	virtual void OnUpdate(float DeltaTime) override;
	
	/** Charge construction costs (materials, labor, overhead) */
	bool ChargeConstructionCosts(FEntityId SiteId);
	
	/** Process a client payment for a completed contract */
	void ProcessContractPayment(FEntityId ContractId, double Amount);
	
	/** Get total profit earned */
	double GetTotalProfit() const { return TotalProfit; }
	
	/** Process daily overhead (wages, insurance, loan interest) */
	void ProcessDailyCosts();
	
	/** Connect to CompanySystem for money operations */
	void SetCompanySystem(class FCompanySystem* InCompany) { CompanySystem = InCompany; }
	
private:
	class FCompanySystem* CompanySystem = nullptr;
	double TotalProfit = 0.0;
	float DayAccumulator = 0.0f;
	static constexpr float DayDuration = 6.0f; // 6 seconds = 1 game day (prototype)
};
