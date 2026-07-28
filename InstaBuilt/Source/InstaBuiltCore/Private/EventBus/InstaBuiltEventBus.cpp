// InstaBuiltEventBus.cpp — Event Bus implementation

#include "EventBus/InstaBuiltEventBus.h"
#include "Logging/InstaBuiltLog.h"

FEventTypeId FInstaBuiltEventBus::NextEventTypeId = 1;

FInstaBuiltEventBus& FInstaBuiltEventBus::Get()
{
	static FInstaBuiltEventBus Instance;
	return Instance;
}

void FInstaBuiltEventBus::Initialize()
{
	check(!bInitialized);
	ImmediateQueue.Empty();
	DeferredQueue.Empty();
	Subscribers.Empty();
	bInitialized = true;
	IB_LOG_INFO("Event Bus initialized. NextEventTypeId=%d", NextEventTypeId);
}

void FInstaBuiltEventBus::Shutdown()
{
	check(bInitialized);
	IB_LOG_INFO("Event Bus shutting down. %d immediate, %d deferred events discarded.",
		ImmediateQueue.Num(), DeferredQueue.Num());
	FlushImmediate(); // Process remaining immediate events
	ImmediateQueue.Empty();
	DeferredQueue.Empty();
	Subscribers.Empty();
	bInitialized = false;
}

void FInstaBuiltEventBus::FlushImmediate()
{
	// Process immediate events. Subscribers may publish more events
	// during processing, so we iterate by index on a potentially growing array.
	for (int32 i = 0; i < ImmediateQueue.Num(); ++i)
	{
		const auto& Event = ImmediateQueue[i];
		const auto* SubList = Subscribers.Find(Event->GetEventTypeId());
		if (SubList)
		{
			for (const auto& Pair : *SubList)
			{
				Pair.Value.ExecuteIfBound(*Event);
			}
		}
	}
	ImmediateQueue.Empty();
}

void FInstaBuiltEventBus::PromoteDeferred()
{
	// Move all deferred events to the immediate queue for next frame
	ImmediateQueue.Append(DeferredQueue);
	DeferredQueue.Empty();
}

void FInstaBuiltEventBus::Unsubscribe(int32 SubscriptionId)
{
	for (auto& Pair : Subscribers)
	{
		Pair.Value.RemoveAll([SubscriptionId](const auto& P) {
			return P.Key == SubscriptionId;
		});
	}
}

FString FInstaBuiltEventBus::GetStats() const
{
	return FString::Printf(TEXT("Immediate: %d | Deferred: %d | Subscribers: %d"),
		ImmediateQueue.Num(), DeferredQueue.Num(), Subscribers.Num());
}
