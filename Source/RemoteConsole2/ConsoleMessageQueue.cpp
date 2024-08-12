// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "ConsoleMessageQueue.h"
#include "HAL/Event.h"
#include "GenericPlatform/GenericPlatformProcess.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FConsoleMessageQueue::FConsoleMessageQueue( bool with_event, uint32_t queue_size )
{
	if( with_event ){
		iEvent= FGenericPlatformProcess::GetSynchEventFromPool( true );
		iEvent->Reset();
	}
	QueueSize= queue_size;
}

FConsoleMessageQueue::~FConsoleMessageQueue()
{
	if( iEvent ){
		iEvent->Trigger();
		FGenericPlatformProcess::ReturnSynchEventToPool( iEvent );
		iEvent= nullptr;
	}
}

bool	FConsoleMessageQueue::GetMessage( TArray<Message>& array )
{
	array.Reset();
	Lock.Lock();
	auto	queue_size= MessageArray.Num();
	if( queue_size ){
		for( const auto& message : MessageArray ){
			array.Add( message );
		}
		MessageArray.Reset();
	}
	if( iEvent ){
		iEvent->Reset();
	}
	bHasMessage.store( false );
	Lock.Unlock();
	return	queue_size != 0;
}

bool	FConsoleMessageQueue::WaitMessage( TArray<Message>& array, float timeout_sec )
{
	array.Reset();
	if( iEvent ){
		Lock.Lock();
		auto	queue_size= MessageArray.Num();
		Lock.Unlock();
		if( !queue_size ){
			if( !iEvent->Wait( FTimespan::FromSeconds( timeout_sec ) ) ){
				return	false;
			}
		}
	}
	return	GetMessage( array );
}

void	FConsoleMessageQueue::Push( const Message& data )
{
	Lock.Lock();
	if( static_cast<uint32_t>(MessageArray.Num()) < QueueSize ){
		MessageArray.Add( data );
		if( iEvent ){
			iEvent->Trigger();
		}
		bHasMessage.store( true );
	}
	Lock.Unlock();
}

bool	FConsoleMessageQueue::HasMessage() const
{
	return	bHasMessage.load();
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


