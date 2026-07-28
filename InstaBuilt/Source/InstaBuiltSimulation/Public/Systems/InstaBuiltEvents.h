// InstaBuiltEvents.h — Game event definitions for prototype
// SIMULATION.md: 86 events total — these are the prototype subset

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"
#include "EventBus/InstaBuiltEventBus.h"

// ============================================================
// COMPANY EVENTS
// ============================================================

struct FCompanyCreatedEvent : public FGameEvent
{
	FEntityId CompanyId;
	FString CompanyName;
	double StartingMoney;
	INSTABUILT_EVENT(CompanyCreated)
};

struct FMoneyChangedEvent : public FGameEvent
{
	FEntityId CompanyId;
	double OldAmount;
	double NewAmount;
	double Delta;
	FString Reason;
	INSTABUILT_EVENT(MoneyChanged)
};

// ============================================================
// CONTRACT EVENTS
// ============================================================

struct FContractGeneratedEvent : public FGameEvent
{
	FEntityId ContractId;
	FString ContractName;
	FString ClientName;
	double Reward;
	INSTABUILT_EVENT(ContractGenerated)
};

struct FContractAcceptedEvent : public FGameEvent
{
	FEntityId ContractId;
	double BidAmount;
	INSTABUILT_EVENT(ContractAccepted)
};

struct FContractCompletedEvent : public FGameEvent
{
	FEntityId ContractId;
	double PaymentAmount;
	int32 QualityScore;
	bool bOnTime;
	INSTABUILT_EVENT(ContractCompleted)
};

// ============================================================
// CONSTRUCTION EVENTS
// ============================================================

struct FConstructionStartedEvent : public FGameEvent
{
	FEntityId SiteId;
	FEntityId BuildingId;
	FEntityId ContractId;
	INSTABUILT_EVENT(ConstructionStarted)
};

struct FPhaseCompletedEvent : public FGameEvent
{
	FEntityId SiteId;
	FString PhaseName;
	float QualityScore;
	INSTABUILT_EVENT(PhaseCompleted)
};

struct FConstructionCompletedEvent : public FGameEvent
{
	FEntityId SiteId;
	FEntityId BuildingId;
	float FinalQuality;
	INSTABUILT_EVENT(ConstructionCompleted)
};

// ============================================================
// WORKER EVENTS
// ============================================================

struct FWorkerHiredEvent : public FGameEvent
{
	FEntityId WorkerId;
	FString WorkerName;
	EWorkerRole Role;
	INSTABUILT_EVENT(WorkerHired)
};

struct FWorkerAssignedEvent : public FGameEvent
{
	FEntityId WorkerId;
	FEntityId SiteId;
	INSTABUILT_EVENT(WorkerAssigned)
};

// ============================================================
// UI/GAME EVENTS
// ============================================================

struct FNotificationEvent : public FGameEvent
{
	FString Message;
	FString Category; // "Success", "Warning", "Error", "Info"
	INSTABUILT_EVENT(Notification)
};
