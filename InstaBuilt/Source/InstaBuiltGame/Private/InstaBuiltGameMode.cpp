// InstaBuiltGameMode.cpp — Game loop with System Orchestrator
// Architecture: ARCHITECTURE.md Section 9.1 Frame Update Lifecycle
// Process: Input → Commands → Simulation → Events → Render

#include "InstaBuiltGameMode.h"
#include "InstaBuiltGameInstance.h"
#include "InstaBuiltCoreModule.h"
#include "InstaBuiltSimulationModule.h"
#include "ECS/InstaBuiltECS.h"
#include "ECS/InstaBuiltSystem.h"
#include "EventBus/InstaBuiltEventBus.h"
#include "Commands/InstaBuiltCommandProcessor.h"
#include "Logging/InstaBuiltLog.h"

AInstaBuiltGameMode::AInstaBuiltGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;
}

void AInstaBuiltGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	IB_LOG_INFO("=== INSTABUILT GAME MODE BEGIN PLAY ===");
	
	// Verify core module
	if (!FInstaBuiltCoreModule::Get().IsInitialized())
	{
		IB_LOG_FATAL("Core module not initialized!");
		return;
	}
	
	// Initialize all simulation systems via the orchestrator
	FSystemOrchestrator::Get().InitializeAll();
	FSystemOrchestrator::Get().PrintSystemOrder();
	
	// Initialize game world (new game or loaded save)
	AInstaBuiltGameState* GS = GetInstaBuiltGameState();
	if (GS)
	{
		UInstaBuiltGameInstance* GI = Cast<UInstaBuiltGameInstance>(GetGameInstance());
		if (GI && GI->IsNewGame())
		{
			GS->InitializeNewGame();
		}
	}
	
	IB_LOG_INFO("All systems ready. Beginning game loop at %.1fx speed.", TimeScale);
}

void AInstaBuiltGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	GameLoopTick(DeltaTime);
}

void AInstaBuiltGameMode::GameLoopTick(float DeltaTime)
{
	// ARCHITECTURE.md Section 9.1: Frame Update Lifecycle
	
	// Step 1: Process queued player commands
	FInstaBuiltCommandProcessor::Get().ProcessCommands();
	
	// Step 2: Tick all simulation systems in dependency order
	float SimDelta = DeltaTime * TimeScale;
	FSystemOrchestrator::Get().UpdateAll(SimDelta);
	
	// Step 3: Promote deferred events to immediate
	FInstaBuiltEventBus::Get().PromoteDeferred();
	
	// Step 4: Flush immediate events
	FInstaBuiltEventBus::Get().FlushImmediate();
}

void AInstaBuiltGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IB_LOG_INFO("Game mode ending.");
	FSystemOrchestrator::Get().ShutdownAll();
	Super::EndPlay(EndPlayReason);
}

AInstaBuiltGameState* AInstaBuiltGameMode::GetInstaBuiltGameState() const
{
	return Cast<AInstaBuiltGameState>(GameState);
}
