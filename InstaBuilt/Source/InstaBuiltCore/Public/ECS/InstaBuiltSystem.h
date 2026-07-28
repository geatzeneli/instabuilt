// InstaBuiltSystem.h — Base class for all domain systems
// Architecture: ARCHITECTURE.md Section 3.0 (Core Modules)
// Simulation: SIMULATION.md — 28 systems total
//
// Every system in the game inherits from FInstaBuiltSystem.
// Systems are registered with the System Orchestrator and
// ticked in dependency order each frame.
//
// Systems communicate via Event Bus (publish/subscribe) and
// Commands (validated player actions). No direct system-to-system calls
// except through defined interfaces.

#pragma once

#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"
#include "EventBus/InstaBuiltEventBus.h"

// ============================================================
// SYSTEM BASE CLASS
// ============================================================

/**
 * FInstaBuiltSystem
 * 
 * Base class for every game system. Each system:
 * - Owns specific entity types and components
 * - Processes at a defined update frequency
 * - Communicates via Event Bus (publish) and Command Processor (receive)
 * - Has explicit dependencies declared for ordering
 * 
 * Lifecycle:
 *   OnRegister → OnInitialize → OnUpdate (each tick) → OnShutdown
 */
class INSTABUILTCORE_API FInstaBuiltSystem
{
public:
	FInstaBuiltSystem(FName InSystemName);
	virtual ~FInstaBuiltSystem() = default;
	
	// --- Identity ---
	FName GetSystemName() const { return SystemName; }
	uint32 GetSystemId() const { return SystemId; }
	
	// --- Lifecycle ---
	
	/** Called once when the system is registered. Set up subscriptions here. */
	virtual void OnRegister() {}
	
	/** Called when the game world is ready. Initialize owned entities here. */
	virtual void OnInitialize() {}
	
	/** Called each frame (or at configured frequency). Core update logic here. */
	virtual void OnUpdate(float DeltaTime) {}
	
	/** Called when the system is shutting down. Clean up resources. */
	virtual void OnShutdown() {}
	
	// --- Dependencies ---
	
	/** Systems that must update BEFORE this system */
	TArray<FName> GetDependencies() const { return Dependencies; }
	
	/** Declare a dependency on another system */
	void AddDependency(FName SystemName) { Dependencies.AddUnique(SystemName); }
	
	// --- Update Configuration ---
	
	/** How often does this system update? (0 = every frame) */
	float GetUpdateInterval() const { return UpdateInterval; }
	void SetUpdateInterval(float Interval) { UpdateInterval = Interval; }
	
	/** Is this system currently enabled? */
	bool IsEnabled() const { return bEnabled; }
	void SetEnabled(bool bEnable) { bEnabled = bEnable; }
	
	// --- Debug ---
	
	/** Time spent in OnUpdate last frame (for profiling) */
	float GetLastUpdateTimeMs() const { return LastUpdateTimeMs; }
	
	/** Number of entities this system manages */
	virtual int32 GetOwnedEntityCount() const { return 0; }
	
protected:
	FName SystemName;
	uint32 SystemId;
	TArray<FName> Dependencies;
	float UpdateInterval = 0.0f; // 0 = every frame
	bool bEnabled = true;
	float LastUpdateTimeMs = 0.0f;
	float TimeSinceLastUpdate = 0.0f;
	
	// Convenience accessors
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	FInstaBuiltEventBus& EventBus = FInstaBuiltEventBus::Get();
	
	friend class FSystemOrchestrator;
};

// ============================================================
// SYSTEM ORCHESTRATOR
// ============================================================

/**
 * FSystemOrchestrator
 * 
 * Manages all game systems: registration, dependency resolution,
 * ordered update scheduling, and lifecycle management.
 * 
 * This is the runtime heart of the simulation layer.
 * Systems register, declare dependencies, and the orchestrator
 * ensures they tick in the correct order each frame.
 * 
 * Architecture: ARCHITECTURE.md Section 9.1 (Frame Update Lifecycle)
 * Simulation: SIMULATION.md — Update Order Diagram
 */
class INSTABUILTCORE_API FSystemOrchestrator
{
public:
	static FSystemOrchestrator& Get();
	
	// --- Registration ---
	
	/** Register a system. Returns system ID. Systems tick in registration order unless dependencies force reordering. */
	uint32 RegisterSystem(TSharedPtr<FInstaBuiltSystem> System);
	
	/** Get a registered system by name */
	TSharedPtr<FInstaBuiltSystem> GetSystem(FName SystemName) const;
	
	/** Get a registered system by type */
	template<typename T>
	TSharedPtr<T> GetSystem() const;
	
	// --- Lifecycle ---
	
	/** Initialize all registered systems (calls OnRegister then OnInitialize) */
	void InitializeAll();
	
	/** Shutdown all systems in reverse dependency order */
	void ShutdownAll();
	
	// --- Update ---
	
	/** Tick all enabled systems in dependency order */
	void UpdateAll(float DeltaTime);
	
	/** Tick a specific system by name */
	void UpdateSystem(FName SystemName, float DeltaTime);
	
	// --- Dependency Resolution ---
	
	/** Build the ordered update list based on declared dependencies */
	void ResolveUpdateOrder();
	
	// --- Debug ---
	
	void PrintSystemOrder() const;
	FString GetStats() const;
	int32 GetSystemCount() const { return Systems.Num(); }
	
private:
	// All registered systems (by name)
	TMap<FName, TSharedPtr<FInstaBuiltSystem>> SystemMap;
	
	// Ordered list for updates (dependency-resolved)
	TArray<TSharedPtr<FInstaBuiltSystem>> UpdateOrder;
	
	// System ID counter
	uint32 NextSystemId = 1;
	
	bool bInitialized = false;
	bool bOrderResolved = false;
};

// ============================================================
// SYSTEM REGISTRATION MACRO
// ============================================================

/**
 * Register a system with the orchestrator.
 * Usage in system constructor or module startup:
 *   REGISTER_SYSTEM(FBuildingSystem)
 */
#define REGISTER_SYSTEM(SystemClass) \
	FSystemOrchestrator::Get().RegisterSystem(MakeShared<SystemClass>());

// ============================================================
// TEMPLATE IMPLEMENTATIONS
// ============================================================

template<typename T>
TSharedPtr<T> FSystemOrchestrator::GetSystem() const
{
	for (const auto& Pair : SystemMap)
	{
		TSharedPtr<T> Casted = StaticCastSharedPtr<T>(Pair.Value);
		if (Casted.IsValid())
		{
			return Casted;
		}
	}
	return nullptr;
}
