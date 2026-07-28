// SystemOrchestratorTests.cpp — System framework integration tests
// Validates: system registration, dependency ordering, update lifecycle

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "Logging/InstaBuiltLog.h"

#if WITH_AUTOMATION_TESTS

// ============================================================
// Test Systems
// ============================================================

/** Simple test system that records its update count */
class FTestSystemA : public FInstaBuiltSystem
{
public:
	FTestSystemA() : FInstaBuiltSystem(TEXT("TestSystemA")) {}
	
	int32 UpdateCount = 0;
	
	virtual void OnUpdate(float DeltaTime) override
	{
		UpdateCount++;
	}
};

class FTestSystemB : public FInstaBuiltSystem
{
public:
	FTestSystemB() : FInstaBuiltSystem(TEXT("TestSystemB"))
	{
		AddDependency(TEXT("TestSystemA")); // B depends on A → A must tick first
	}
	
	int32 UpdateCount = 0;
	int32 LastAUpdateCount = 0; // Tracks A's update count when B ticks
	
	virtual void OnUpdate(float DeltaTime) override
	{
		// Verify A ticked before B (dependency contract)
		auto SysA = FSystemOrchestrator::Get().GetSystem<FTestSystemA>(TEXT("TestSystemA"));
		if (SysA.IsValid())
		{
			LastAUpdateCount = SysA->UpdateCount;
		}
		UpdateCount++;
	}
};

// ============================================================
// TESTS
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrchestratorRegisterTest,
	"InstaBuilt.Systems.Register",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOrchestratorRegisterTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	auto SysA = MakeShared<FTestSystemA>();
	uint32 IdA = Orch.RegisterSystem(SysA);
	TestTrue("System A registered with valid ID", IdA > 0);
	
	auto SysB = MakeShared<FTestSystemB>();
	uint32 IdB = Orch.RegisterSystem(SysB);
	TestTrue("System B registered with valid ID", IdB > 0);
	TestNotEqual("Systems have different IDs", IdA, IdB);
	
	TestEqual("Two systems registered", Orch.GetSystemCount(), 2);
	
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrchestratorDependencyOrderTest,
	"InstaBuilt.Systems.DependencyOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOrchestratorDependencyOrderTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	// Register A first, then B (B depends on A)
	auto SysA = MakeShared<FTestSystemA>();
	Orch.RegisterSystem(SysA);
	
	auto SysB = MakeShared<FTestSystemB>();
	Orch.RegisterSystem(SysB);
	
	// Initialize (resolves dependencies, calls OnRegister, OnInitialize)
	Orch.InitializeAll();
	
	// Tick once
	Orch.UpdateAll(0.016f);
	
	// Both systems should have updated
	TestEqual("System A updated once", SysA->UpdateCount, 1);
	TestEqual("System B updated once", SysB->UpdateCount, 1);
	
	// B should have seen A's update before B ticked (dependency order)
	TestEqual("B sees A's update before B ticks", SysB->LastAUpdateCount, 1);
	
	// Tick again
	Orch.UpdateAll(0.016f);
	TestEqual("System A updated twice", SysA->UpdateCount, 2);
	TestEqual("System B updated twice", SysB->UpdateCount, 2);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrchestratorUpdateIntervalTest,
	"InstaBuilt.Systems.UpdateInterval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOrchestratorUpdateIntervalTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	// System that only updates every 1.0 second
	auto SlowSys = MakeShared<FTestSystemA>();
	SlowSys->SetUpdateInterval(1.0f);
	Orch.RegisterSystem(SlowSys);
	Orch.InitializeAll();
	
	// Tick with small delta — system should NOT update
	Orch.UpdateAll(0.016f);
	TestEqual("Slow system not updated at 0.016s", SlowSys->UpdateCount, 0);
	
	// Accumulate time — after 1.0s+ it should update
	Orch.UpdateAll(0.5f);
	TestEqual("Still not at 1.0s threshold", SlowSys->UpdateCount, 0);
	
	Orch.UpdateAll(0.5f); // Total: 1.016s — should trigger
	TestEqual("Updated after crossing threshold", SlowSys->UpdateCount, 1);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrchestratorDisabledSystemTest,
	"InstaBuilt.Systems.Disabled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOrchestratorDisabledSystemTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	auto Sys = MakeShared<FTestSystemA>();
	Sys->SetEnabled(false);
	Orch.RegisterSystem(Sys);
	Orch.InitializeAll();
	
	// Tick — disabled system should NOT update
	Orch.UpdateAll(1.0f);
	TestEqual("Disabled system was not updated", Sys->UpdateCount, 0);
	
	// Enable and tick again
	Sys->SetEnabled(true);
	Orch.UpdateAll(0.016f);
	TestEqual("Re-enabled system updates", Sys->UpdateCount, 1);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOrchestratorCircularDependencyTest,
	"InstaBuilt.Systems.CircularDependency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOrchestratorCircularDependencyTest::RunTest(const FString& Parameters)
{
	FInstaBuiltECS::Get().Initialize();
	
	FSystemOrchestrator& Orch = FSystemOrchestrator::Get();
	
	// Create circular: X depends on Y, Y depends on X
	class FCircularSys : public FInstaBuiltSystem
	{
	public:
		FCircularSys(FName Name) : FInstaBuiltSystem(Name) {}
	};
	
	auto SysX = MakeShared<FCircularSys>(TEXT("CircularX"));
	auto SysY = MakeShared<FCircularSys>(TEXT("CircularY"));
	
	SysX->AddDependency(TEXT("CircularY"));
	SysY->AddDependency(TEXT("CircularX"));
	
	Orch.RegisterSystem(SysX);
	Orch.RegisterSystem(SysY);
	
	// Should not crash — circular deps are detected and logged
	Orch.InitializeAll();
	
	// Both systems should still be initialized (fallback to registration order)
	TestEqual("Both systems registered despite circular dep", Orch.GetSystemCount(), 2);
	
	Orch.ShutdownAll();
	FInstaBuiltECS::Get().Shutdown();
	return true;
}

#endif // WITH_AUTOMATION_TESTS
