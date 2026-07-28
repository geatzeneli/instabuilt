// InstaBuiltECS.cpp — ECS Core implementation
// Milestone 1: Entity lifecycle, component storage, queries

#include "ECS/InstaBuiltECS.h"
#include "Logging/InstaBuiltLog.h"

FInstaBuiltECS& FInstaBuiltECS::Get()
{
	static FInstaBuiltECS Instance;
	return Instance;
}

void FInstaBuiltECS::Initialize()
{
	check(!bInitialized);
	
	Entities.Empty();
	EntityComponents.Empty();
	TotalComponents = 0;
	
	bInitialized = true;
	IB_LOG_INFO("ECS initialized. Ready for entities.");
}

void FInstaBuiltECS::Shutdown()
{
	check(bInitialized);
	
	IB_LOG_INFO("ECS shutting down. Destroying %d entities, %d components.",
		Entities.Num(), TotalComponents);
	
	EntityComponents.Empty();
	Entities.Empty();
	TotalComponents = 0;
	bInitialized = false;
	
	IB_LOG_INFO("ECS shutdown complete.");
}

FEntityId FInstaBuiltECS::CreateEntity(EEntityType Type)
{
	check(bInitialized);
	
	FEntityId NewId = GenerateEntityId();
	Entities.Add(NewId, Type);
	EntityComponents.Add(NewId, FEntityComponents());
	
	IB_LOG_DEBUG("Entity created: %s (Type: %d)", *NewId.ToString(), (int32)Type);
	return NewId;
}

void FInstaBuiltECS::DestroyEntity(FEntityId Entity)
{
	check(bInitialized);
	
	if (!IsEntityValid(Entity))
	{
		IB_LOG_WARN("Attempted to destroy invalid entity: %s", *Entity.ToString());
		return;
	}
	
	auto* Archetype = EntityComponents.Find(Entity);
	if (Archetype)
	{
		TotalComponents -= Archetype->Components.Num();
		EntityComponents.Remove(Entity);
	}
	
	Entities.Remove(Entity);
	IB_LOG_DEBUG("Entity destroyed: %s", *Entity.ToString());
}

bool FInstaBuiltECS::IsEntityValid(FEntityId Entity) const
{
	return Entity.IsValid() && Entities.Contains(Entity);
}

EEntityType FInstaBuiltECS::GetEntityType(FEntityId Entity) const
{
	const auto* Type = Entities.Find(Entity);
	return Type ? *Type : EEntityType::None;
}

TArray<FEntityId> FInstaBuiltECS::GetEntitiesOfType(EEntityType Type) const
{
	TArray<FEntityId> Result;
	for (const auto& Pair : Entities)
	{
		if (Pair.Value == Type)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

FString FInstaBuiltECS::GetStats() const
{
	return FString::Printf(TEXT("Entities: %d | Components: %d | Archetypes: %d"),
		Entities.Num(), TotalComponents, EntityComponents.Num());
}

FEntityId FInstaBuiltECS::GenerateEntityId() const
{
	// DATA_MODEL Section 4.2: UUID v4, thread-safe, no central authority needed
	return FEntityId(FGuid::NewGuid());
}

// ============================================================
// Component Type ID Registration
// 
// Each component type gets a unique uint16 ID for fast lookup.
// New components register here.
// 
// DATA_MODEL Section 2: 87 components planned.
// TypeId space: 65,535 slots. We'll use sequential registration.
// ============================================================

// TypeId counter (monotonically increasing)
static uint16 GNextComponentTypeId = 0;
static TMap<FName, uint16> GComponentTypeRegistry;

uint16 FComponentBase::GetTypeId()
{
	// Base class always returns 0 — subclasses override via REGISTER_COMPONENT
	return 0;
}

// Registration macro for new component types
// Usage: REGISTER_COMPONENT_TYPE(C_BuildingDesign, "BuildingDesign")
#define REGISTER_COMPONENT_TYPE(ComponentClass, TypeName) \
	static uint16 ComponentClass##_TypeId = []() { \
		uint16 Id = GNextComponentTypeId++; \
		GComponentTypeRegistry.Add(FName(TypeName), Id); \
		return Id; \
	}(); \
	template<> uint16 ComponentClass::GetTypeId() { return ComponentClass##_TypeId; }
