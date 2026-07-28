// InstaBuiltCommandProcessor.h — Command Bus
// Architecture: ARCHITECTURE.md Section 7.2
// GSS Section 3.2: All player actions are Commands

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltECS.h"

/** Base class for all game commands. Commands are validated, loggable, undoable. */
struct INSTABUILTCORE_API FGameCommand
{
	virtual ~FGameCommand() = default;
	virtual FString GetCommandName() const { return TEXT("UnknownCommand"); }
	virtual bool Validate() const { return true; }
	virtual void Execute() = 0;
	virtual void Undo() {}
};

class INSTABUILTCORE_API FInstaBuiltCommandProcessor
{
public:
	static FInstaBuiltCommandProcessor& Get();
	void Initialize();
	void Shutdown();
	
	void QueueCommand(TSharedPtr<FGameCommand> Command);
	void ProcessCommands();
	void UndoLast();
	
	int32 GetQueueSize() const { return CommandQueue.Num(); }
	FString GetStats() const;
	
private:
	TArray<TSharedPtr<FGameCommand>> CommandQueue;
	TArray<TSharedPtr<FGameCommand>> History; // for undo
	static constexpr int32 MaxHistory = 50; // GSS Section 3.4
};
