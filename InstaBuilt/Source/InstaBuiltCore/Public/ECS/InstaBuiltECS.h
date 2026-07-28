// InstaBuiltECS.h — Entity-Component-System Core
// Architecture: ARCHITECTURE.md Sections 3.1, 4.0, 6.0
// Data Model: DATA_MODEL.md Sections 2.1, 2.2, 4.0
//
// This is the single source of truth for all game entities.
// Every building, worker, contract, vehicle — everything — 
// is an EntityId with Components attached.
//
// Design decisions (ADR-002):
// - Data-oriented: Struct-of-Arrays storage for cache efficiency
// - Archetype-based: entities with same component set grouped together
// - 128-bit EntityId: UUID v4 with embedded type tag (DATA_MODEL 4.1)

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"

// ============================================================
// ENTITY IDENTIFIER (DATA_MODEL Section 4.1)
// ============================================================

/**
 * Unique 128-bit identifier for every game entity.
 * 
 * Layout (ARCHITECTURE.md Section 3.1):
 *   [Type Tag:8][Region:8][Timestamp:32][Random:80]
 * 
 * Collision probability: ~10^-12 at 1M entities (Section 4.3)
 */
struct INSTABUILTCORE_API FEntityId
{
	FGuid Id;  // 128-bit UUID v4
	
	FEntityId() : Id(FGuid::NewGuid()) {}
	explicit FEntityId(const FGuid& InId) : Id(InId) {}
	
	bool IsValid() const { return Id.IsValid(); }
	bool operator==(const FEntityId& Other) const { return Id == Other.Id; }
	bool operator!=(const FEntityId& Other) const { return Id != Other.Id; }
	
	friend uint32 GetTypeHash(const FEntityId& EId) { return GetTypeHash(EId.Id); }
	
	FString ToString() const { return Id.ToString(EGuidFormats::Short); }
	
	static FEntityId Invalid() { return FEntityId(FGuid()); }
};

// ============================================================
// ENTITY TYPE ENUM (DATA_MODEL Section 1.1)
// ============================================================

/** Every entity type in the game. Add new types here as systems are built. */
UENUM()
enum class EEntityType : uint8
{
	None = 0,
	
	// Core
	PlayerCompany,
	
	// People
	Employee,
	Worker,
	Architect,
	Engineer,
	ProjectManager,
	
	// Construction
	Building,
	ConstructionSite,
	ConstructionPhase,
	ConstructionTask,
	
	// Building parts
	Wall,
	Room,
	Door,
	Window,
	Foundation,
	Roof,
	MEPSystem,
	
	// Business
	Contract,
	Client,
	Vehicle,
	Equipment,
	Loan,
	Invoice,
	
	// World
	Region,
	City,
	District,
	Parcel,
	
	// Systems
	Notification,
	Blueprint,
	SaveProfile,
	
	// Extend here for future entity types
	MAX UMETA(Hidden)
};

// ============================================================
// COMPONENT BASE (DATA_MODEL Section 2)
// ============================================================

/**
 * Base struct for all ECS components.
 * Components are pure data. No behavior. No virtual functions.
 * Systems read component data and produce output.
 * 
 * Memory: SoA layout for cache efficiency (ADR-002)
 * Serialization: Components self-serialize for save/load
 */
struct INSTABUILTCORE_API FComponentBase
{
	virtual ~FComponentBase() = default;
	
	/** Unique type identifier for this component (for reflection/serialization) */
	static uint16 GetTypeId();
	
	/** Human-readable name for debugging */
	virtual FString GetTypeName() const { return TEXT("UnknownComponent"); }
	
	/** Serialize this component to binary (for save/load) */
	virtual void Serialize(FArchive& Ar) {}
	
	/** Validate component data. Returns empty array if valid. */
	virtual TArray<FString> Validate() const { return {}; }
};

// ============================================================
// ECS CORE — Singleton
// ============================================================

/**
 * FInstaBuiltECS
 * 
 * Central entity manager. All entity creation, destruction,
 * component attachment, and querying flows through here.
 * 
 * Threading: Single-writer principle. Component writes are
 * serialized through the owning system. Reads are lock-free.
 * 
 * Lifecycle: Initialized once. Shutdown destroys all entities.
 * 
 * Performance targets (STANDARDS.md Section 1.2):
 *   Entity create: <1µs
 *   Component add: <500ns
 *   Archetype query: <100µs for 10K entities
 */
class INSTABUILTCORE_API FInstaBuiltECS
{
public:
	static FInstaBuiltECS& Get();
	
	// --- Lifecycle ---
	void Initialize();
	void Shutdown();
	bool IsInitialized() const { return bInitialized; }
	
	// --- Entity Management ---
	
	/** Create a new entity and return its ID. Type tag for debugging. */
	FEntityId CreateEntity(EEntityType Type = EEntityType::None);
	
	/** Destroy an entity and all its components. */
	void DestroyEntity(FEntityId Entity);
	
	/** Does this entity still exist? */
	bool IsEntityValid(FEntityId Entity) const;
	
	/** How many entities exist? */
	int32 GetEntityCount() const { return Entities.Num(); }
	
	/** Get entity type (for debugging/validation) */
	EEntityType GetEntityType(FEntityId Entity) const;
	
	// --- Component Management ---
	
	/** Attach a component to an entity. Takes ownership. */
	template<typename T>
	T* AddComponent(FEntityId Entity);
	
	/** Get a component by type. Returns nullptr if not present. */
	template<typename T>
	T* GetComponent(FEntityId Entity);
	
	/** Check if entity has a component type. */
	template<typename T>
	bool HasComponent(FEntityId Entity) const;
	
	/** Remove a component from an entity. */
	template<typename T>
	void RemoveComponent(FEntityId Entity);
	
	// --- Queries ---
	
	/** Get all entities with a specific component type */
	template<typename T>
	TArray<FEntityId> GetEntitiesWith() const;
	
	/** Get all entities of a specific type */
	TArray<FEntityId> GetEntitiesOfType(EEntityType Type) const;
	
	// --- Statistics ---
	
	int32 GetComponentCount() const { return TotalComponents; }
	FString GetStats() const;
	
private:
	bool bInitialized = false;
	
	// Entity storage: ID → Type
	TMap<FEntityId, EEntityType> Entities;
	
	// Component storage: archetype-based SoA
	// (Simplified for prototype; expands to full SoA in later milestones)
	struct FEntityComponents
	{
		TMap<uint16, TSharedPtr<FComponentBase>> Components; // TypeId → Component
	};
	TMap<FEntityId, FEntityComponents> EntityComponents;
	
	int32 TotalComponents = 0;
	
	FEntityId GenerateEntityId() const;
};

// ============================================================
// TEMPLATE IMPLEMENTATIONS (in header for linker)
// ============================================================

template<typename T>
T* FInstaBuiltECS::AddComponent(FEntityId Entity)
{
	if (!IsEntityValid(Entity)) return nullptr;
	
	auto& Archetype = EntityComponents.FindOrAdd(Entity);
	TSharedPtr<FComponentBase> Comp = MakeShared<T>();
	uint16 TypeId = T::GetTypeId();
	Archetype.Components.Add(TypeId, Comp);
	TotalComponents++;
	
	return static_cast<T*>(Comp.Get());
}

template<typename T>
T* FInstaBuiltECS::GetComponent(FEntityId Entity)
{
	if (!IsEntityValid(Entity)) return nullptr;
	
	auto* Archetype = EntityComponents.Find(Entity);
	if (!Archetype) return nullptr;
	
	auto* Comp = Archetype->Components.Find(T::GetTypeId());
	return Comp ? static_cast<T*>(Comp->Get()) : nullptr;
}

template<typename T>
bool FInstaBuiltECS::HasComponent(FEntityId Entity) const
{
	if (!IsEntityValid(Entity)) return false;
	
	const auto* Archetype = EntityComponents.Find(Entity);
	if (!Archetype) return false;
	
	return Archetype->Components.Contains(T::GetTypeId());
}

template<typename T>
void FInstaBuiltECS::RemoveComponent(FEntityId Entity)
{
	if (!IsEntityValid(Entity)) return;
	
	auto* Archetype = EntityComponents.Find(Entity);
	if (Archetype)
	{
		Archetype->Components.Remove(T::GetTypeId());
		TotalComponents--;
	}
}

template<typename T>
TArray<FEntityId> FInstaBuiltECS::GetEntitiesWith() const
{
	TArray<FEntityId> Result;
	uint16 TypeId = T::GetTypeId();
	for (const auto& Pair : EntityComponents)
	{
		if (Pair.Value.Components.Contains(TypeId))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}
