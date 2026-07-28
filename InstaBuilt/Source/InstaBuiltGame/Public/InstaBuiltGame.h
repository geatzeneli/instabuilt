// InstaBuiltGame.h — Game module export header

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class INSTABUILTGAME_API FInstaBuiltGameModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
