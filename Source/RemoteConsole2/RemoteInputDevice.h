// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once

#include "RemoteConsole2.h"
#include "IInputDevice.h"
#include "RemoteConsoleServer3.h"
#include "RemoteOutputDevice.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FGameAccessAPI;

class FRemoteInputDevice : public IInputDevice {
public:
	constexpr static uint32_t	KEYCODE_MAX		=	256;
	constexpr static float		SAMPLING_RATE	=	60.0f;	// Hz
private:
	TSharedRef<FGenericApplicationMessageHandler>	MessageHandler;
	FRemoteConsoleServer3	Server;
	FRemoteOutputDevice		RemoteDevice;
	//-------------------------------------------------------------------------
	FPlatformUserId		UserID;
	FInputDeviceId		DeviceID;
	//-------------------------------------------------------------------------
	FRemoteConsoleServer3::ControllerStatus	PrevStatus;
	TArray<FRemoteConsoleServer3::KeyboardStatus>	KeyboardStatusBuffer;
	TArray<FConsoleMessageQueue::Message>	GameAPICommandBuffer;
	uint32_t	KeycodeStateMap[KEYCODE_MAX>>5];
	uint32_t	KeycodeMax= 0;
	//-------------------------------------------------------------------------
	FGameAccessAPI*	iGameAPI= nullptr;
	//-------------------------------------------------------------------------
	uint32	ControllerID= 0;
	float	TickTotal= 0.0f;
	bool	bEnabled= true;
	bool	bCaptureLog= false;
	bool	bInitialized= false;
private:
	void	Initialize();
	void	SetKeycodeState( uint32_t keycode );
	void	ResetKeycodeState( uint32_t keycode );
	bool	GetKeycodeState( uint32_t keycode ) const;
	void	ClearKeycodeState();
	void	SendStick( FRemoteConsoleServer3::ControllerStatus& status, uint32_t stick_id, uint32_t bit_minus, uint32_t bit_plus, const FName& stick_name, bool reverse= false );
	void	SendButton( uint32_t trigger, uint32_t button, uint32_t bit, const FName& button_name );
	void	ExecGameAPI();
public:

	FRemoteInputDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler );
	virtual ~FRemoteInputDevice();

	void	Tick( float delta_time ) override;
	void	SendControllerEvents() override;
	void	SetChannelValue( int32 controller_id, FForceFeedbackChannelType type, float value ) override;
	void	SetChannelValues( int32 controller_id, const FForceFeedbackValues& values ) override;
	void	SetMessageHandler( const TSharedRef<FGenericApplicationMessageHandler>& handler ) override;
	bool	Exec( UWorld* world, const TCHAR* cmd, FOutputDevice& Ar ) override;
	bool	IsGamepadAttached() const override;
	//-------------------------------------------------------------------------
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

