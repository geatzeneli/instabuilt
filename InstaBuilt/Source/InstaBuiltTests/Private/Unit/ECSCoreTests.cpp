// ECSCoreTests.cpp — ECS Core unit tests
// STANDARDS.md Section 6.2: Unit testing

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ECS/InstaBuiltECS.h"

#if WITH_AUTOMATION_TESTS

// Test component for ECS testing
struct C_TestHealth : public FComponentBase
{
	float Current = 100.0f;
	float Max = 100.0f;
	virtual FString GetTypeName() const override { return TEXT("TestHealth"); }
};

struct C_TestName : public FComponentBase
{
	FString Name;
	virtual FString GetTypeName() const override { return TEXT("TestName"); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FECSCreateEntityTest,
	"InstaBuilt.ECS.CreateEntity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FECSCreateEntityTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	ECS.Initialize();
	
	FEntityId Id = ECS.CreateEntity();
	TestTrue("Entity is valid after creation", ECS.IsEntityValid(Id));
	TestEqual("Entity count is 1", ECS.GetEntityCount(), 1);
	
	ECS.Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FECSAddComponentTest,
	"InstaBuilt.ECS.AddComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FECSAddComponentTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	ECS.Initialize();
	
	FEntityId Id = ECS.CreateEntity();
	
	C_TestHealth* Health = ECS.AddComponent<C_TestHealth>(Id);
	TestNotNull("Component added successfully", Health);
	TestEqual("Health value is default", Health->Current, 100.0f);
	
	C_TestName* Name = ECS.AddComponent<C_TestName>(Id);
	Name->Name = TEXT("TestEntity");
	TestEqual("Name component stored correctly", Name->Name, TEXT("TestEntity"));
	
	TestTrue("Has Health component", ECS.HasComponent<C_TestHealth>(Id));
	TestTrue("Has Name component", ECS.HasComponent<C_TestName>(Id));
	
	TestEqual("Total component count", ECS.GetComponentCount(), 2);
	
	ECS.Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FECSDestroyEntityTest,
	"InstaBuilt.ECS.DestroyEntity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FECSDestroyEntityTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	ECS.Initialize();
	
	FEntityId Id = ECS.CreateEntity();
	ECS.AddComponent<C_TestHealth>(Id);
	
	ECS.DestroyEntity(Id);
	TestFalse("Entity is invalid after destruction", ECS.IsEntityValid(Id));
	TestEqual("Entity count is 0", ECS.GetEntityCount(), 0);
	
	ECS.Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FECSQueryTest,
	"InstaBuilt.ECS.Query",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FECSQueryTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	ECS.Initialize();
	
	// Create entities: two with Health, one without
	FEntityId E1 = ECS.CreateEntity();
	FEntityId E2 = ECS.CreateEntity();
	FEntityId E3 = ECS.CreateEntity();
	
	ECS.AddComponent<C_TestHealth>(E1);
	ECS.AddComponent<C_TestHealth>(E2);
	
	auto HealthEntities = ECS.GetEntitiesWith<C_TestHealth>();
	TestEqual("Two entities have Health", HealthEntities.Num(), 2);
	
	auto NameEntities = ECS.GetEntitiesWith<C_TestName>();
	TestEqual("No entities have Name", NameEntities.Num(), 0);
	
	ECS.Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
