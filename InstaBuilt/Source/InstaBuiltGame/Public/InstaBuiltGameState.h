// InstaBuiltGameState.h — Runtime game state
// Milestone 3: Game State Foundation
// GSS Section 4.2: Company Dashboard state
// DATA_MODEL Sections 1 & 2: Entity and component ownership

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ECS/InstaBuiltECS.h"
#include "InstaBuiltGameState.generated.h"

/**
 * AInstaBuiltGameState
 * 
 * Holds runtime game state that persists for the session.
 * Contains references to: player company, active world, calendar.
 * 
 * This is the "save-able" state — everything here gets serialized.
 */
UCLASS()
class INSTABUILTGAME_API AInstaBuiltGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AInstaBuiltGameState();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// --- World State ---
	
	/** Initialize a new game world (new company, empty city, starting contract pool) */
	void InitializeNewGame();
	
	/** Restore game world from a save file */
	void InitializeFromSave(const FString& SaveFilePath);
	
	/** Is the game currently loading? */
	bool IsLoading() const { return bLoading; }
	
	// --- Player Company ---
	
	/** Entity ID of the player's company (singleton) */
	FEntityId GetPlayerCompanyId() const { return PlayerCompanyId; }
	
	/** Create the player company entity */
	void CreatePlayerCompany(const FString& CompanyName, double StartingMoney);
	
	// --- Game State Queries ---
	
	/** Get the current in-game time */
	double GetGameTime() const { return GameTime; }
	
	/** Get the current in-game date as a string */
	FString GetGameDateString() const;
	
	/** Total play time in this save (real seconds) */
	double GetTotalPlayTime() const { return TotalPlayTime; }
	
protected:
	/** The player's company (singleton entity) */
	FEntityId PlayerCompanyId;
	
	/** Current in-game time (seconds since game start) */
	double GameTime = 0.0;
	
	/** In-game day counter */
	int32 GameDay = 1;
	
	/** In-game year */
	int32 GameYear = 1;
	
	/** Total real playtime for this save */
	double TotalPlayTime = 0.0;
	
	/** Is the game currently loading state? */
	bool bLoading = false;
	
	/** Has the world been initialized? */
	bool bWorldInitialized = false;
	
	/** Advance game time by delta seconds */
	void AdvanceGameTime(float DeltaSeconds);
	
	friend class AInstaBuiltGameMode;
};
