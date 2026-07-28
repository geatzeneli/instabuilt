// InstaBuiltCommandProcessor.cpp

#include "Commands/InstaBuiltCommandProcessor.h"
#include "Logging/InstaBuiltLog.h"

FInstaBuiltCommandProcessor& FInstaBuiltCommandProcessor::Get()
{
	static FInstaBuiltCommandProcessor Instance;
	return Instance;
}

void FInstaBuiltCommandProcessor::Initialize() { IB_LOG_INFO("Command Processor initialized."); }
void FInstaBuiltCommandProcessor::Shutdown() { CommandQueue.Empty(); History.Empty(); }

void FInstaBuiltCommandProcessor::QueueCommand(TSharedPtr<FGameCommand> Command)
{
	if (Command->Validate())
	{
		CommandQueue.Add(Command);
	}
	else
	{
		IB_LOG_WARN("Command validation failed: %s", *Command->GetCommandName());
	}
}

void FInstaBuiltCommandProcessor::ProcessCommands()
{
	for (auto& Cmd : CommandQueue)
	{
		Cmd->Execute();
		History.Add(Cmd);
		if (History.Num() > MaxHistory) History.RemoveAt(0);
	}
	CommandQueue.Empty();
}

void FInstaBuiltCommandProcessor::UndoLast()
{
	if (History.Num() > 0)
	{
		History.Last()->Undo();
		History.RemoveAt(History.Num() - 1);
	}
}

FString FInstaBuiltCommandProcessor::GetStats() const
{
	return FString::Printf(TEXT("Queue: %d | History: %d"), CommandQueue.Num(), History.Num());
}
