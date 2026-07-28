// InstaBuiltGameMode.h — Updated with GameState ref

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InstaBuiltGameMode.generated.h"

class AInstaBuiltGameState;

UCLASS()
class INSTABUILTGAME_API AInstaBuiltGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AInstaBuiltGameMode();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Get the game state cast to our type */
	AInstaBuiltGameState* GetInstaBuiltGameState() const;
	
protected:
	void GameLoopTick(float DeltaTime);
	
	/** Simulation speed multiplier */
	float TimeScale = 1.0f;
};
