// InstaBuiltCoreModule.cpp — Core module implementation
// Initializes all foundation subsystems in dependency order

#include "InstaBuiltCoreModule.h"
#include "ECS/InstaBuiltECS.h"
#include "EventBus/InstaBuiltEventBus.h"
#include "Commands/InstaBuiltCommandProcessor.h"
#include "Logging/InstaBuiltLog.h"
#include "Debug/InstaBuiltDebug.h"

#define LOCTEXT_NAMESPACE "FInstaBuiltCoreModule"

void FInstaBuiltCoreModule::StartupModule()
{
	IB_LOG_INFO("InstaBuiltCore module starting up...");
	
	// Initialize subsystems in dependency order:
	// ECS must exist first (all data lives here)
	// EventBus needs ECS (events reference entities)
	// CommandProcessor needs EventBus (commands emit events)
	// Logging is standalone
	// Debug tools are standalone
	
	InitializeLogging();
	IB_LOG_INFO("Logging initialized");
	
	InitializeECS();
	IB_LOG_INFO("ECS Core initialized");
	
	InitializeEventBus();
	IB_LOG_INFO("Event Bus initialized");
	
	InitializeCommandProcessor();
	IB_LOG_INFO("Command Processor initialized");
	
	InitializeDebugTools();
	IB_LOG_INFO("Debug tools initialized");
	
	bInitialized = true;
	IB_LOG_INFO("InstaBuiltCore module ready");
}

void FInstaBuiltCoreModule::ShutdownModule()
{
	IB_LOG_INFO("InstaBuiltCore module shutting down...");
	
	ShutdownDebugTools();
	ShutdownCommandProcessor();
	ShutdownEventBus();
	ShutdownECS();
	ShutdownLogging();
	
	bInitialized = false;
	IB_LOG_INFO("InstaBuiltCore module shutdown complete");
}

FInstaBuiltCoreModule& FInstaBuiltCoreModule::Get()
{
	return FModuleManager::LoadModuleChecked<FInstaBuiltCoreModule>("InstaBuiltCore");
}

void FInstaBuiltCoreModule::InitializeECS()
{
	FInstaBuiltECS::Get().Initialize();
}

void FInstaBuiltCoreModule::InitializeEventBus()
{
	FInstaBuiltEventBus::Get().Initialize();
}

void FInstaBuiltCoreModule::InitializeCommandProcessor()
{
	FInstaBuiltCommandProcessor::Get().Initialize();
}

void FInstaBuiltCoreModule::InitializeLogging()
{
	FInstaBuiltLog::Initialize();
}

void FInstaBuiltCoreModule::InitializeDebugTools()
{
	FInstaBuiltDebug::Initialize();
}

void FInstaBuiltCoreModule::ShutdownDebugTools()
{
	FInstaBuiltDebug::Shutdown();
}

void FInstaBuiltCoreModule::ShutdownLogging()
{
	FInstaBuiltLog::Shutdown();
}

void FInstaBuiltCoreModule::ShutdownCommandProcessor()
{
	FInstaBuiltCommandProcessor::Get().Shutdown();
}

void FInstaBuiltCoreModule::ShutdownEventBus()
{
	FInstaBuiltEventBus::Get().Shutdown();
}

void FInstaBuiltCoreModule::ShutdownECS()
{
	FInstaBuiltECS::Get().Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstaBuiltCoreModule, InstaBuiltCore)
