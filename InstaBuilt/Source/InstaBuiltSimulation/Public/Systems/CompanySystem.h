// CompanySystem.h — Player company management (M4)
// Owns: Company entity, financials, reputation
// GSS Section 8: Company Management Workflow

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/CompanyComponents.h"

class FCompanySystem : public FInstaBuiltSystem
{
public:
	FCompanySystem() : FInstaBuiltSystem(TEXT("CompanySystem")) {}
	
	virtual void OnInitialize() override;
	
	/** Create the player company with starting funds */
	FEntityId CreateCompany(const FString& Name, double StartingMoney);
	
	/** Get the player company ID */
	FEntityId GetCompanyId() const { return CompanyId; }
	
	/** Get current cash */
	double GetCash() const;
	
	/** Spend money (returns false if insufficient funds) */
	bool SpendMoney(double Amount, const FString& Reason);
	
	/** Receive money */
	void ReceiveMoney(double Amount, const FString& Reason);
	
	/** Get reputation scores */
	const C_Reputation* GetReputation() const;
	
	/** Add reputation from a completed project */
	void AddProjectReputation(float Quality, bool bOnTime);
	
	/** Get company dashboard data as formatted strings */
	void GetDashboardData(FString& OutName, FString& OutTier, FString& OutCash,
		FString& OutReputation, FString& OutProjects) const;
	
	void PrintStatus() const;
	
private:
	FEntityId CompanyId;
};
