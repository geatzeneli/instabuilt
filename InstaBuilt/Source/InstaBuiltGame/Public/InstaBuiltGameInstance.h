// InstaBuiltGameInstance.h — Persistent game instance
// Owns: world state reference, save profile, game mode transition

#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "InstaBuiltGameInstance.generated.h"

UCLASS()
class INSTABUILTGAME_API UInstaBuiltGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	/** Is this a new game or a loaded save? */
	bool IsNewGame() const { return bNewGame; }
	void SetNewGame(bool bNew) { bNewGame = bNew; }
	
private:
	bool bNewGame = true;
};
