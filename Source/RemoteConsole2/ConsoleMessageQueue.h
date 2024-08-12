// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once

#include "CoreMinimal.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FEvent;

class FConsoleMessageQueue {
public:
	struct Message {
		uint16_t	Command;
		uint16_t	Param0;
		uint32_t	Param1;
		FString		StringParam;
	};
private:
	TArray<Message>		MessageArray;
	FCriticalSection	Lock;
	FEvent*		iEvent= nullptr;
	uint32_t	QueueSize= 0;
	std::atomic<bool>	bHasMessage= false;
public:
	FConsoleMessageQueue( bool with_event, uint32_t queue_size );
	~FConsoleMessageQueue();
	void	Push( const Message& data );
	bool	GetMessage( TArray<Message>& array );
	bool	WaitMessage( TArray<Message>& array, float timeout_sec );
	bool	HasMessage() const;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


