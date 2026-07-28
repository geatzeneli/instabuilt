// InstaBuiltEventBus.h — Event Bus for inter-system communication
// Architecture: ARCHITECTURE.md Sections 7.1, 7.2
// Simulation: SIMULATION.md Section 1 (86 events total)
//
// The Event Bus decouples systems. Systems publish events;
// interested systems subscribe. Publishers don't know subscribers exist.
//
// Design decision (ADR-003): Events for inter-system, direct calls intra-system.
//
// Event types:
//   Immediate: delivered this frame (for state changes)
//   Deferred:  delivered next frame (for reactions)
//
// Performance: lock-free ring buffer for immediate events.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "ECS/InstaBuiltECS.h"

// ============================================================
// EVENT BASE
// ============================================================

/** Unique identifier for each event type */
using FEventTypeId = uint16;

/** Base struct for all game events. Events are value types — cheap to copy. */
struct INSTABUILTCORE_API FGameEvent
{
	virtual ~FGameEvent() = default;
	virtual FEventTypeId GetEventTypeId() const = 0;
	virtual FString GetEventName() const { return TEXT("UnknownEvent"); }
	
	/** Timestamp when event was published (in-game time) */
	double Timestamp = 0.0;
	
	/** Entity that triggered this event (if applicable) */
	FEntityId SourceEntity;
};

// ============================================================
// EVENT BUS
// ============================================================

/** Callback type for event subscribers */
DECLARE_DELEGATE_OneParam(FEventCallback, const FGameEvent&);

/**
 * FInstaBuiltEventBus
 * 
 * Central event dispatcher. All inter-system communication flows here.
 * 
 * Usage:
 *   // Publish
 *   EventBus.Publish<FContractCompletedEvent>(ContractId, PaymentAmount);
 *   
 *   // Subscribe
 *   EventBus.Subscribe<FContractCompletedEvent>(this, &MySystem::OnContractCompleted);
 */
class INSTABUILTCORE_API FInstaBuiltEventBus
{
public:
	static FInstaBuiltEventBus& Get();
	
	void Initialize();
	void Shutdown();
	
	// --- Publishing ---
	
	/** Publish an event immediately. Delivered this frame to all subscribers. */
	template<typename T, typename... Args>
	void Publish(Args&&... args);
	
	/** Publish an event for next-frame delivery. */
	template<typename T, typename... Args>
	void PublishDeferred(Args&&... args);
	
	// --- Subscribing ---
	
	/** Subscribe to an event type. Returns subscription handle for unsubscribing. */
	template<typename T>
	int32 Subscribe(FEventCallback Callback);
	
	/** Subscribe to an event type with a specific handler method. */
	template<typename T, typename UserClass>
	int32 Subscribe(UserClass* Object, void (UserClass::*Method)(const FGameEvent&));
	
	/** Remove a subscription. */
	void Unsubscribe(int32 SubscriptionId);
	
	// --- Processing ---
	
	/** Process all immediate events in the queue. Called each frame. */
	void FlushImmediate();
	
	/** Move deferred events to immediate queue for next frame. */
	void PromoteDeferred();
	
	// --- Statistics ---
	
	int32 GetImmediateQueueSize() const { return ImmediateQueue.Num(); }
	int32 GetDeferredQueueSize() const { return DeferredQueue.Num(); }
	int32 GetSubscriberCount() const { return Subscribers.Num(); }
	FString GetStats() const;
	
private:
	bool bInitialized = false;
	
	// Event storage — TSharedPtr for polymorphism
	TArray<TSharedPtr<FGameEvent>> ImmediateQueue;
	TArray<TSharedPtr<FGameEvent>> DeferredQueue;
	
	// Subscription registry: EventTypeId → list of subscribers
	TMap<FEventTypeId, TArray<TPair<int32, FEventCallback>>> Subscribers;
	
	// Monotonically increasing subscription ID
	int32 NextSubscriptionId = 1;
	
	// Monotonically increasing event type ID
	static FEventTypeId NextEventTypeId;
};

// ============================================================
// TEMPLATE IMPLEMENTATIONS
// ============================================================

template<typename T, typename... Args>
void FInstaBuiltEventBus::Publish(Args&&... args)
{
	auto Event = MakeShared<T>(Forward<Args>(args)...);
	ImmediateQueue.Add(Event);
}

template<typename T, typename... Args>
void FInstaBuiltEventBus::PublishDeferred(Args&&... args)
{
	auto Event = MakeShared<T>(Forward<Args>(args)...);
	DeferredQueue.Add(Event);
}

template<typename T>
int32 FInstaBuiltEventBus::Subscribe(FEventCallback Callback)
{
	int32 Id = NextSubscriptionId++;
	auto& SubList = Subscribers.FindOrAdd(T::StaticEventTypeId());
	SubList.Add(TPair<int32, FEventCallback>(Id, Callback));
	return Id;
}

template<typename T, typename UserClass>
int32 FInstaBuiltEventBus::Subscribe(UserClass* Object, void (UserClass::*Method)(const FGameEvent&))
{
	return Subscribe<T>(FEventCallback::CreateRaw(Object, Method));
}

// ============================================================
// EVENT TYPE REGISTRATION MACRO
// ============================================================

/**
 * Register a new event type with the Event Bus.
 * Usage in event struct definition:
 *   INSTABUILT_EVENT(FContractCompleted)
 */
#define INSTABUILT_EVENT(EventName) \
	static FEventTypeId StaticEventTypeId() { \
		static FEventTypeId Id = FInstaBuiltEventBus::NextEventTypeId++; \
		return Id; \
	} \
	virtual FEventTypeId GetEventTypeId() const override { return StaticEventTypeId(); } \
	virtual FString GetEventName() const override { return TEXT(#EventName); }
