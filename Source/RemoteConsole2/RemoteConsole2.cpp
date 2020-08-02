// RemoteConsole2 2020/07/25
// vim:ts=4 sw=4 noet:

#include	"RemoteConsole2.h"
#include	"RemoteDevice.h"
#include	"IInputDeviceModule.h"


class FRemoteConsolel2 : public IInputDeviceModule {

	TSharedPtr<IInputDevice>	CreateInputDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler ) override
	{
		return	TSharedPtr<IInputDevice>( new FRemoteDevice( handler ) );
	}
};


IMPLEMENT_MODULE( FRemoteConsolel2, RemoteConsole2 )


