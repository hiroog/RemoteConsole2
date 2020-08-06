// RemoteConsole2 2020/07/25
// vim:ts=4 sw=4 noet:

#pragma once
#include "RemoteConsole2.h"
#include "IInputDevice.h"
#include "RemoteConsoleServer2.h"
#if USE_UALIBNET
# include "RemoteConsoleServer.h"
#endif


class FRemoteDevice : public IInputDevice {
	TSharedRef<FGenericApplicationMessageHandler>	MessageHandler;
#if USE_UALIBNET
	FRemoteConsoleServer	Server;
#else
	FRemoteConsoleServer2	Server;
#endif
public:

	FRemoteDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler );
	virtual ~FRemoteDevice();

	void	Tick( float delta_time ) override;
	void	SendControllerEvents() override;
	void	SetChannelValue( int32 controller_id, FForceFeedbackChannelType type, float value ) override;
	void	SetChannelValues( int32 controller_id, const FForceFeedbackValues& values ) override;
	void	SetMessageHandler( const TSharedRef<FGenericApplicationMessageHandler>& handler ) override;
	bool	Exec( UWorld* world, const TCHAR* cmd, FOutputDevice& Ar ) override;
	bool	IsGamepadAttached() const override;
};
