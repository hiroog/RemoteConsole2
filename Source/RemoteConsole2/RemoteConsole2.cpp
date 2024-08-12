// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	"RemoteConsole2.h"
#include	"RemoteInputDevice.h"
#include	"IInputDeviceModule.h"


class FRemoteConsolel2 : public IInputDeviceModule {
public:
	TSharedPtr<IInputDevice>	CreateInputDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler ) override
	{
		return	TSharedPtr<IInputDevice>( new FRemoteInputDevice( handler ) );
	}
};


IMPLEMENT_MODULE( FRemoteConsolel2, RemoteConsole2 )

DEFINE_LOG_CATEGORY( LogRemoteConsole2 );

