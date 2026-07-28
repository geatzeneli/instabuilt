// ContractSystem.cpp — M5: Contract lifecycle

#include "Systems/ContractSystem.h"
#include "Systems/InstaBuiltEvents.h"
#include "Logging/InstaBuiltLog.h"

void FContractSystem::OnInitialize()
{
	IB_LOG_INFO("ContractSystem initialized.");
}

void FContractSystem::OnUpdate(float DeltaTime)
{
	// Countdown deadlines on active contracts
	for (auto& ContractId : ActiveContracts)
	{
		auto* Contract = ECS.GetComponent<C_ContractData>(ContractId);
		if (Contract && Contract->State == EContractState::Active)
		{
			// Simple day counter — Time System replaces this later
		}
	}
}

FEntityId FContractSystem::GenerateContract(const FString& Name, const FString& Client,
	const FString& BuildingType, double Reward, int32 DeadlineDays)
{
	FEntityId ContractId = ECS.CreateEntity(EEntityType::Contract);
	
	auto* Data = ECS.AddComponent<C_ContractData>(ContractId);
	Data->ContractName = Name;
	Data->ClientName = Client;
	Data->BuildingType = BuildingType;
	Data->BudgetMin = Reward * 0.7;
	Data->BudgetMax = Reward * 1.3;
	Data->Reward = Reward;
	Data->DeadlineDays = DeadlineDays;
	Data->DaysRemaining = DeadlineDays;
	Data->RequiredRooms = 3;
	Data->RequiredArea = 28.0f;
	Data->State = EContractState::Available;
	
	AvailableContracts.Add(ContractId);
	
	EventBus.Publish<FContractGeneratedEvent>(ContractId, Name, Client, Reward);
	IB_LOG_INFO("Contract generated: %s ($%.0f)", *Name, Reward);
	
	return ContractId;
}

bool FContractSystem::AcceptContract(FEntityId ContractId, double BidAmount)
{
	auto* Contract = ECS.GetComponent<C_ContractData>(ContractId);
	if (!Contract || Contract->State != EContractState::Available) return false;
	
	Contract->PlayerBid = BidAmount;
	Contract->State = EContractState::Awarded;
	
	AvailableContracts.Remove(ContractId);
	ActiveContracts.Add(ContractId);
	
	EventBus.Publish<FContractAcceptedEvent>(ContractId, BidAmount);
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("Contract accepted: %s — $%.0f"), *Contract->ContractName, BidAmount),
		TEXT("Success"));
	
	IB_LOG_INFO("Contract accepted: %s (Bid: $%.0f)", *Contract->ContractName, BidAmount);
	return true;
}

bool FContractSystem::ActivateContract(FEntityId ContractId)
{
	auto* Contract = ECS.GetComponent<C_ContractData>(ContractId);
	if (!Contract || Contract->State != EContractState::Awarded) return false;
	
	Contract->State = EContractState::Active;
	IB_LOG_INFO("Contract now active: %s", *Contract->ContractName);
	return true;
}

bool FContractSystem::CompleteContract(FEntityId ContractId, int32 QualityScore, bool bOnTime)
{
	auto* Contract = ECS.GetComponent<C_ContractData>(ContractId);
	if (!Contract) return false;
	
	Contract->State = EContractState::Completed;
	Contract->QualityScore = QualityScore;
	Contract->bOnTime = bOnTime;
	
	double Payment = Contract->Reward;
	if (bOnTime) Payment *= 1.1; // 10% bonus
	if (QualityScore > 90) Payment *= 1.05; // Quality bonus
	if (!bOnTime) Payment *= 0.9; // Late penalty
	
	ActiveContracts.Remove(ContractId);
	
	EventBus.Publish<FContractCompletedEvent>(ContractId, Payment, QualityScore, bOnTime);
	EventBus.Publish<FNotificationEvent>(
		FString::Printf(TEXT("Contract complete: %s — Paid $%.0f"), *Contract->ContractName, Payment),
		TEXT("Success"));
	
	IB_LOG_INFO("Contract completed: %s — Payment: $%.0f (Quality: %d%%, OnTime: %s)",
		*Contract->ContractName, Payment, QualityScore, bOnTime ? TEXT("Yes") : TEXT("No"));
	
	return true;
}

TArray<FEntityId> FContractSystem::GetAvailableContracts() const
{
	return AvailableContracts;
}

FString FContractSystem::GetContractDetails(FEntityId ContractId) const
{
	auto* Contract = ECS.GetComponent<C_ContractData>(ContractId);
	if (!Contract) return TEXT("Invalid contract");
	
	return FString::Printf(TEXT("%s | Client: %s | Type: %s | Budget: $%.0f-$%.0f | Deadline: %d days"),
		*Contract->ContractName, *Contract->ClientName, *Contract->BuildingType,
		Contract->BudgetMin, Contract->BudgetMax, Contract->DeadlineDays);
}

void FContractSystem::GenerateStarterContracts()
{
	GenerateContract(TEXT("First Home"), TEXT("Johnson Family"),
		TEXT("POP_UP_28"), 100000.0, 90);
	GenerateContract(TEXT("Garage Addition"), TEXT("Smith Residence"),
		TEXT("POP_UP_28"), 45000.0, 45);
	GenerateContract(TEXT("Studio Shed"), TEXT("Riverside Arts"),
		TEXT("POP_UP_28"), 65000.0, 60);
	IB_LOG_INFO("3 starter contracts generated.");
}
