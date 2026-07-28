// InstaBuiltLog.cpp — Logging implementation

#include "Logging/InstaBuiltLog.h"

DEFINE_LOG_CATEGORY(LogInstaBuilt);
DEFINE_LOG_CATEGORY(LogIB_ECS);
DEFINE_LOG_CATEGORY(LogIB_EventBus);
DEFINE_LOG_CATEGORY(LogIB_Commands);
DEFINE_LOG_CATEGORY(LogIB_Building);
DEFINE_LOG_CATEGORY(LogIB_Construction);
DEFINE_LOG_CATEGORY(LogIB_Contract);
DEFINE_LOG_CATEGORY(LogIB_Worker);
DEFINE_LOG_CATEGORY(LogIB_Economy);
DEFINE_LOG_CATEGORY(LogIB_Save);

void FInstaBuiltLog::Initialize()
{
	IB_LOG_INFO("=== InstaBuilt Logging Initialized ===");
	IB_LOG_INFO("Game Version: %s", TEXT(INSTABUILT_VERSION));
}

void FInstaBuiltLog::Shutdown()
{
	IB_LOG_INFO("=== InstaBuilt Logging Shutdown ===");
}

void FInstaBuiltLog::SetShippingLogLevel(ELogVerbosity::Type Level)
{
	LogInstaBuilt.SetVerbosity(Level);
}

void FInstaBuiltLog::SetFileLogging(bool bEnable, const FString& LogFilePath)
{
	if (bEnable)
	{
		// UE5 provides file logging via GLog
		GLog->EnableBacklog(true);
		IB_LOG_INFO("File logging enabled");
	}
}
