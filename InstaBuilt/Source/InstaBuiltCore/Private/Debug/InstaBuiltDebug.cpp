// InstaBuiltDebug.cpp — Debug tool implementations (stubs for prototype)

#include "Debug/InstaBuiltDebug.h"
#include "ECS/InstaBuiltECS.h"
#include "EventBus/InstaBuiltEventBus.h"
#include "Logging/InstaBuiltLog.h"

void FInstaBuiltDebug::Initialize()
{
	RegisterConsoleCommands();
	IB_LOG_INFO("Debug tools initialized. Console commands: 'ib.ecsstats', 'ib.eventstats'");
}

void FInstaBuiltDebug::Shutdown() {}

void FInstaBuiltDebug::RegisterConsoleCommands()
{
	// ECS stats
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ib.ecsstats"),
		TEXT("Print ECS entity and component statistics"),
		FConsoleCommandDelegate::CreateStatic(&FInstaBuiltDebug::PrintECSStats)
	);
	
	// EventBus stats
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ib.eventstats"),
		TEXT("Print Event Bus queue and subscriber statistics"),
		FConsoleCommandDelegate::CreateStatic(&FInstaBuiltDebug::PrintEventBusStats)
	);
	
	// Memory stats
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ib.memstats"),
		TEXT("Print memory usage statistics"),
		FConsoleCommandDelegate::CreateStatic(&FInstaBuiltDebug::PrintMemoryStats)
	);
	
	// Cheat: add money
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("ib.cheat.addmoney"),
		TEXT("Add money to the player company. Usage: ib.cheat.addmoney 100000"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
			if (Args.Num() > 0)
			{
				double Amount = FCString::Atod(*Args[0]);
				IB_LOG_INFO("CHEAT: Adding %.0f money", Amount);
				// Will connect to Company System in Milestone 4
			}
		})
	);
}

void FInstaBuiltDebug::PrintECSStats()
{
	IB_LOG_INFO("=== ECS STATS ===");
	IB_LOG_INFO("%s", *FInstaBuiltECS::Get().GetStats());
}

void FInstaBuiltDebug::PrintEventBusStats()
{
	IB_LOG_INFO("=== EVENT BUS STATS ===");
	IB_LOG_INFO("%s", *FInstaBuiltEventBus::Get().GetStats());
}

void FInstaBuiltDebug::PrintMemoryStats()
{
	// Placeholder — will integrate with UE5 memory profiling
	IB_LOG_INFO("=== MEMORY STATS ===");
	IB_LOG_INFO("(Memory profiling available in UE Insights)");
}

void FInstaBuiltDebug::DrawEntityDebugInfo(UWorld* World) {}
void FInstaBuiltDebug::DrawEventBusDebugInfo(UWorld* World) {}
void FInstaBuiltDebug::Cheat_AddMoney(double Amount) {}
void FInstaBuiltDebug::Cheat_CompleteAllPhases() {}
void FInstaBuiltDebug::Cheat_SpawnWorkers(int32 Count) {}
