// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "RemoteOutputDevice.h"
#include "RemoteConsoleServer3.h"

#define	RC2_USE_LOWLEVEL_OUTPUT	0

#if RC2_USE_LOWLEVEL_OUTPUT
# include "HAL/PlatformMisc.h"
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void	FRemoteOutputDevice::Serialize( const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category )
{
	if( Data && *Data ){
		if( iRemoteServer ){
			FConsoleMessageQueue::Message	message;
			message.Command= FRemoteConsoleServer3::CMD_RETURN_LOG;
			message.Param0= 0;
			message.Param1= 0;
			message.StringParam= FString::Printf( TEXT("%s: %s"), *Category.ToString(), Data );
			iRemoteServer->PushResult( message );
#if	RC2_USE_LOWLEVEL_OUTPUT
FPlatformMisc::LowLevelOutputDebugString( *FString::Printf( TEXT("**** %s\n"), *message.StringParam ) );
#endif
		}
	}
}


void	FRemoteOutputDevice::SetServer( FRemoteConsoleServer3* server )
{
	iRemoteServer= server;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


