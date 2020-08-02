// RemoteConsole2 2020/07/25
// vim:ts=4 sw=4 noet:

#include "RemoteDevice.h"


FRemoteDevice::FRemoteDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler ) :
	MessageHandler( handler )
{
	Server.StartServer( nullptr, 10101, 4 );
}

FRemoteDevice::~FRemoteDevice()
{
	Server.StopServer();
}

void	FRemoteDevice::Tick( float delta_time )
{
	if( GEngine && GWorld ){
		Server.Flush( GWorld );
	}
}

void	FRemoteDevice::SendControllerEvents()
{
}

void	FRemoteDevice::SetChannelValue( int32 controller_id, FForceFeedbackChannelType type, float value )
{
}

void	FRemoteDevice::SetChannelValues( int32 controller_id, const FForceFeedbackValues& values )
{
}

void	FRemoteDevice::SetMessageHandler( const TSharedRef<FGenericApplicationMessageHandler>& handler )
{
	MessageHandler= handler;
}

bool	FRemoteDevice::Exec( UWorld* world, const TCHAR* cmd, FOutputDevice& Ar )
{
	return	false;
}

bool	FRemoteDevice::IsGamepadAttached() const
{
	return	false;
}

