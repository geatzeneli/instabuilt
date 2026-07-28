// InstaBuiltGame.cpp

#include "InstaBuiltGame.h"
#include "Logging/InstaBuiltLog.h"

void FInstaBuiltGameModule::StartupModule()
{
	IB_LOG_INFO("InstaBuiltGame module started.");
}

void FInstaBuiltGameModule::ShutdownModule()
{
	IB_LOG_INFO("InstaBuiltGame module shutdown.");
}

IMPLEMENT_MODULE(FInstaBuiltGameModule, InstaBuiltGame)
