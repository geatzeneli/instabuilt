// ContractSystem.h — Contract generation, bidding, completion (M5)

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/CompanyComponents.h"

class FContractSystem : public FInstaBuiltSystem
{
public:
	FContractSystem() : FInstaBuiltSystem(TEXT("ContractSystem")) {}
	
	virtual void OnInitialize() override;
	virtual void OnUpdate(float DeltaTime) override;
	
	/** Generate a new available contract */
	FEntityId GenerateContract(const FString& Name, const FString& Client,
		const FString& BuildingType, double Reward, int32 DeadlineDays);
	
	/** Accept a contract (player bids on it) */
	bool AcceptContract(FEntityId ContractId, double BidAmount);
	
	/** Mark a contract as active (construction starting) */
	bool ActivateContract(FEntityId ContractId);
	
	/** Complete a contract and trigger payment */
	bool CompleteContract(FEntityId ContractId, int32 QualityScore, bool bOnTime);
	
	/** Get available contracts */
	TArray<FEntityId> GetAvailableContracts() const;
	
	/** Get contract details for UI */
	FString GetContractDetails(FEntityId ContractId) const;
	
	/** Generate 3 starter contracts for the prototype */
	void GenerateStarterContracts();
	
private:
	TArray<FEntityId> AvailableContracts;
	TArray<FEntityId> ActiveContracts;
};
