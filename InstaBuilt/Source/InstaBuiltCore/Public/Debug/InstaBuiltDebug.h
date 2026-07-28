// InstaBuiltDebug.h — Developer debug tools
// Architecture: ARCHITECTURE.md Section 11.2, 11.6 (Diagnostics & Debug Tools)

#pragma once
#include "CoreMinimal.h"

/**
 * FInstaBuiltDebug
 * 
 * In-game debug tools accessible via console commands and UI.
 * Available in dev/editor builds. Stripped from shipping.
 */
class INSTABUILTCORE_API FInstaBuiltDebug
{
public:
	static void Initialize();
	static void Shutdown();
	
	// --- Console Commands ---
	static void RegisterConsoleCommands();
	
	// --- Debug Visualization ---
	static void DrawEntityDebugInfo(UWorld* World);
	static void DrawEventBusDebugInfo(UWorld* World);
	
	// --- Stats ---
	static void PrintECSStats();
	static void PrintEventBusStats();
	static void PrintMemoryStats();
	
	// --- Cheats (dev only) ---
	static void Cheat_AddMoney(double Amount);
	static void Cheat_CompleteAllPhases();
	static void Cheat_SpawnWorkers(int32 Count);
};

// Console command registration macros
#define INSTABUILT_CONSOLE_COMMAND(Name, Help, Func) \
	static FAutoConsoleCommand ConsoleCmd_##Name( \
		TEXT("ib." #Name), \
		FText::FromString(Help), \
		FConsoleCommandDelegate::CreateStatic(&Func) \
	);
