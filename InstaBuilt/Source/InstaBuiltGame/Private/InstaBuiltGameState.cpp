// InstaBuiltGameState.cpp — Game state implementation

#include "InstaBuiltGameState.h"
#include "ECS/InstaBuiltSystem.h"
#include "Logging/InstaBuiltLog.h"

AInstaBuiltGameState::AInstaBuiltGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInstaBuiltGameState::BeginPlay()
{
	Super::BeginPlay();
	IB_LOG_INFO("GameState: BeginPlay");
}

void AInstaBuiltGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IB_LOG_INFO("GameState: EndPlay");
	Super::EndPlay(EndPlayReason);
}

void AInstaBuiltGameState::InitializeNewGame()
{
	bLoading = true;
	IB_LOG_INFO("=== INITIALIZING NEW GAME WORLD ===");
	
	GameTime = 0.0;
	GameDay = 1;
	GameYear = 1;
	
	// Create the player's company (will be expanded in Milestone 4)
	CreatePlayerCompany(TEXT("InstaBuilt"), 250000.0);
	
	bWorldInitialized = true;
	bLoading = false;
	
	IB_LOG_INFO("New game world ready. Company: %s, Starting funds: $250,000",
		*PlayerCompanyId.ToString());
}

void AInstaBuiltGameState::InitializeFromSave(const FString& SaveFilePath)
{
	bLoading = true;
	IB_LOG_INFO("=== LOADING GAME FROM: %s ===", *SaveFilePath);
	
	// Will be implemented in Milestone 11 (Save System)
	// For now: initialize as new game with a warning
	IB_LOG_WARN("Save loading not yet implemented. Creating new game instead.");
	InitializeNewGame();
	
	bLoading = false;
}

void AInstaBuiltGameState::CreatePlayerCompany(const FString& CompanyName, double StartingMoney)
{
	FInstaBuiltECS& ECS = FInstaBuiltECS::Get();
	
	PlayerCompanyId = ECS.CreateEntity(EEntityType::PlayerCompany);
	
	// Company financial component (DATA_MODEL Section 2.5)
	// Will be defined in Milestone 4
	IB_LOG_INFO("Player company created: %s (ID: %s)",
		*CompanyName, *PlayerCompanyId.ToString());
}

FString AInstaBuiltGameState::GetGameDateString() const
{
	// Simple date: Day X, Year Y
	// Will expand to calendar system in later milestones
	return FString::Printf(TEXT("Day %d, Year %d"), GameDay, GameYear);
}

void AInstaBuiltGameState::AdvanceGameTime(float DeltaSeconds)
{
	GameTime += DeltaSeconds;
	TotalPlayTime += DeltaSeconds;
	
	// Simple day advancement: 360 seconds (6 min) = 1 day at 1x
	// This is placeholder — Time System (Milestone 6+) replaces this
	static const float SecondsPerDay = 360.0f; // Placeholder: 1 day = 6 real minutes
	while (GameTime >= GameDay * SecondsPerDay)
	{
		GameDay++;
		if (GameDay > 365)
		{
			GameDay = 1;
			GameYear++;
			IB_LOG_INFO("New year: Year %d", GameYear);
		}
	}
}
