// InstaBuiltGameInstance.cpp

#include "InstaBuiltGameInstance.h"
#include "Logging/InstaBuiltLog.h"

void UInstaBuiltGameInstance::Init()
{
	Super::Init();
	IB_LOG_INFO("Game Instance initialized. New game: %s", bNewGame ? TEXT("Yes") : TEXT("No (loaded)"));
}

void UInstaBuiltGameInstance::Shutdown()
{
	IB_LOG_INFO("Game Instance shutting down.");
	Super::Shutdown();
}
