// InstaBuiltSystem.cpp — System base + Orchestrator implementation

#include "ECS/InstaBuiltSystem.h"
#include "Logging/InstaBuiltLog.h"

// ============================================================
// FInstaBuiltSystem
// ============================================================

FInstaBuiltSystem::FInstaBuiltSystem(FName InSystemName)
	: SystemName(InSystemName)
	, SystemId(0)
{
}

// ============================================================
// FSystemOrchestrator
// ============================================================

FSystemOrchestrator& FSystemOrchestrator::Get()
{
	static FSystemOrchestrator Instance;
	return Instance;
}

uint32 FSystemOrchestrator::RegisterSystem(TSharedPtr<FInstaBuiltSystem> System)
{
	check(System.IsValid());
	
	System->SystemId = NextSystemId++;
	SystemMap.Add(System->GetSystemName(), System);
	
	IB_LOG_INFO("System registered: %s (ID: %u, deps: %d)",
		*System->GetSystemName().ToString(),
		System->GetSystemId(),
		System->GetDependencies().Num());
	
	bOrderResolved = false; // Need to re-resolve order on next InitializeAll
	
	return System->GetSystemId();
}

TSharedPtr<FInstaBuiltSystem> FSystemOrchestrator::GetSystem(FName SystemName) const
{
	const auto* Found = SystemMap.Find(SystemName);
	return Found ? *Found : nullptr;
}

void FSystemOrchestrator::InitializeAll()
{
	check(!bInitialized);
	
	// Step 1: Resolve update order from declared dependencies
	ResolveUpdateOrder();
	
	// Step 2: Call OnRegister on all systems (in dependency order)
	IB_LOG_INFO("=== Initializing %d systems ===", UpdateOrder.Num());
	for (const auto& System : UpdateOrder)
	{
		IB_LOG_INFO("  [REGISTER] %s", *System->GetSystemName().ToString());
		System->OnRegister();
	}
	
	// Step 3: Call OnInitialize on all systems
	for (const auto& System : UpdateOrder)
	{
		IB_LOG_INFO("  [INITIALIZE] %s", *System->GetSystemName().ToString());
		System->OnInitialize();
	}
	
	bInitialized = true;
	IB_LOG_INFO("=== All %d systems initialized ===", UpdateOrder.Num());
}

void FSystemOrchestrator::ShutdownAll()
{
	check(bInitialized);
	
	IB_LOG_INFO("=== Shutting down %d systems (reverse order) ===", UpdateOrder.Num());
	
	// Shutdown in reverse dependency order (dependents before dependencies)
	for (int32 i = UpdateOrder.Num() - 1; i >= 0; --i)
	{
		IB_LOG_INFO("  [SHUTDOWN] %s", *UpdateOrder[i]->GetSystemName().ToString());
		UpdateOrder[i]->OnShutdown();
	}
	
	UpdateOrder.Empty();
	SystemMap.Empty();
	bInitialized = false;
	bOrderResolved = false;
}

void FSystemOrchestrator::UpdateAll(float DeltaTime)
{
	if (!bInitialized) return;
	
	for (const auto& System : UpdateOrder)
	{
		if (!System->IsEnabled()) continue;
		
		// Respect update interval (0 = every frame)
		System->TimeSinceLastUpdate += DeltaTime;
		if (System->GetUpdateInterval() > 0.0f &&
			System->TimeSinceLastUpdate < System->GetUpdateInterval())
		{
			continue;
		}
		
		// Time the update for profiling
		double StartTime = FPlatformTime::Seconds();
		System->OnUpdate(System->TimeSinceLastUpdate);
		System->LastUpdateTimeMs = (float)((FPlatformTime::Seconds() - StartTime) * 1000.0);
		
		System->TimeSinceLastUpdate = 0.0f;
	}
}

void FSystemOrchestrator::UpdateSystem(FName SystemName, float DeltaTime)
{
	auto System = GetSystem(SystemName);
	if (System.IsValid() && System->IsEnabled())
	{
		System->OnUpdate(DeltaTime);
	}
}

void FSystemOrchestrator::ResolveUpdateOrder()
{
	// Topological sort of systems based on declared dependencies.
	// Systems with no dependencies come first.
	// Systems depending on others come after.
	
	UpdateOrder.Empty();
	TArray<FName> Sorted;
	TMap<FName, int32> InDegree;
	TMap<FName, TArray<FName>> AdjacencyList;
	
	// Build adjacency: for each system S depending on D, edge D → S (D must run first)
	for (const auto& Pair : SystemMap)
	{
		InDegree.Add(Pair.Key, 0);
		AdjacencyList.Add(Pair.Key, TArray<FName>());
	}
	
	for (const auto& Pair : SystemMap)
	{
		for (const FName& Dep : Pair.Value->GetDependencies())
		{
			if (SystemMap.Contains(Dep))
			{
				// Edge: Dep → Pair.Key (Dep must tick before Pair.Key)
				AdjacencyList[Dep].Add(Pair.Key);
				InDegree[Pair.Key]++;
			}
			else
			{
				IB_LOG_WARN("System '%s' depends on unknown system '%s'",
					*Pair.Key.ToString(), *Dep.ToString());
			}
		}
	}
	
	// Kahn's algorithm for topological sort
	TArray<FName> Queue;
	for (const auto& Pair : InDegree)
	{
		if (Pair.Value == 0)
		{
			Queue.Add(Pair.Key);
		}
	}
	
	while (Queue.Num() > 0)
	{
		FName Current = Queue[0];
		Queue.RemoveAt(0);
		Sorted.Add(Current);
		
		for (const FName& Neighbor : AdjacencyList[Current])
		{
			InDegree[Neighbor]--;
			if (InDegree[Neighbor] == 0)
			{
				Queue.Add(Neighbor);
			}
		}
	}
	
	// Detect cycles
	if (Sorted.Num() != SystemMap.Num())
	{
		IB_LOG_ERROR("CIRCULAR DEPENDENCY DETECTED in system graph!");
		IB_LOG_ERROR("Sorted %d of %d systems", Sorted.Num(), SystemMap.Num());
		
		// Fall back to registration order for unsorted systems
		for (const auto& Pair : SystemMap)
		{
			if (!Sorted.Contains(Pair.Key))
			{
				IB_LOG_ERROR("  Unresolved system: %s", *Pair.Key.ToString());
			}
		}
	}
	
	// Build update order from sorted names
	for (const FName& Name : Sorted)
	{
		UpdateOrder.Add(SystemMap[Name]);
	}
	
	bOrderResolved = true;
	IB_LOG_INFO("Update order resolved. %d systems in topological order.", UpdateOrder.Num());
}

void FSystemOrchestrator::PrintSystemOrder() const
{
	IB_LOG_INFO("=== SYSTEM UPDATE ORDER ===");
	for (int32 i = 0; i < UpdateOrder.Num(); ++i)
	{
		IB_LOG_INFO("  %2d. %s (interval: %.1fs, enabled: %s, deps: %d)",
			i + 1,
			*UpdateOrder[i]->GetSystemName().ToString(),
			UpdateOrder[i]->GetUpdateInterval(),
			UpdateOrder[i]->IsEnabled() ? TEXT("yes") : TEXT("no"),
			UpdateOrder[i]->GetDependencies().Num());
	}
}

FString FSystemOrchestrator::GetStats() const
{
	float TotalMs = 0.0f;
	for (const auto& System : UpdateOrder)
	{
		TotalMs += System->GetLastUpdateTimeMs();
	}
	
	return FString::Printf(TEXT("Systems: %d | Total tick: %.2fms | Registered: %d"),
		UpdateOrder.Num(), TotalMs, SystemMap.Num());
}
