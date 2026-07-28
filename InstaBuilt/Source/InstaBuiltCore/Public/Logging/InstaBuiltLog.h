// InstaBuiltLog.h — Structured logging for all systems
// Architecture: ARCHITECTURE.md Section 11.1
// Standards: STANDARDS.md Section 10

#pragma once
#include "CoreMinimal.h"

// Log categories (STANDARDS.md 10.1) organized by system
DECLARE_LOG_CATEGORY_EXTERN(LogInstaBuilt, Log, All);        // Root category
DECLARE_LOG_CATEGORY_EXTERN(LogIB_ECS, Log, All);            // ECS Core
DECLARE_LOG_CATEGORY_EXTERN(LogIB_EventBus, Log, All);       // Event Bus
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Commands, Log, All);       // Commands
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Building, Log, All);       // Building System
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Construction, Log, All);   // Construction
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Contract, Log, All);       // Contracts
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Worker, Log, All);         // Workers
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Economy, Log, All);        // Economy
DECLARE_LOG_CATEGORY_EXTERN(LogIB_Save, Log, All);           // Save/Load

// ============================================================
// LOGGING MACROS (STANDARDS.md Section 10)
// ============================================================

// Standard format: [Category] Message
// Performance: format strings evaluated lazily

#define IB_LOG_TRACE(Category, Format, ...)  UE_LOG(Category, VeryVerbose, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_DEBUG(Category, Format, ...)  UE_LOG(Category, Verbose, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_INFO(Format, ...)             UE_LOG(LogInstaBuilt, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_WARN(Format, ...)             UE_LOG(LogInstaBuilt, Warning, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_ERROR(Format, ...)            UE_LOG(LogInstaBuilt, Error, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_FATAL(Format, ...)            UE_LOG(LogInstaBuilt, Fatal, TEXT(Format), ##__VA_ARGS__)

// Per-system logging macros
#define IB_LOG_BUILDING(Format, ...)         UE_LOG(LogIB_Building, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_CONSTRUCTION(Format, ...)     UE_LOG(LogIB_Construction, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_CONTRACT(Format, ...)         UE_LOG(LogIB_Contract, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_WORKER(Format, ...)           UE_LOG(LogIB_Worker, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_ECONOMY(Format, ...)          UE_LOG(LogIB_Economy, Log, TEXT(Format), ##__VA_ARGS__)
#define IB_LOG_SAVE(Format, ...)             UE_LOG(LogIB_Save, Log, TEXT(Format), ##__VA_ARGS__)

// ============================================================
// LOGGING SERVICE
// ============================================================

/**
 * FInstaBuiltLog
 * 
 * Centralized logging configuration. Controls log levels,
 * output targets, and privacy filtering.
 * 
 * Privacy rule (STANDARDS.md 10.3): No PII in logs. Ever.
 */
class INSTABUILTCORE_API FInstaBuiltLog
{
public:
	static void Initialize();
	static void Shutdown();
	
	/** Set minimum log level for shipping builds */
	static void SetShippingLogLevel(ELogVerbosity::Type Level);
	
	/** Enable/disable file logging */
	static void SetFileLogging(bool bEnable, const FString& LogFilePath = TEXT(""));
};
