// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "RemoteInputDevice.h"
#include "InputCoreTypes.h"
#include "RemoteConsole2.h"
#include "GameAccessAPI.h"
#include "Framework/Application/SlateApplication.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FRemoteInputDevice::FRemoteInputDevice( const TSharedRef<FGenericApplicationMessageHandler>& handler ) :
	MessageHandler( handler )
{
	memset( &PrevStatus, 0, sizeof(PrevStatus) );
	ClearKeycodeState();

	int	Port= 10101;
	int	IPV= 4;
	if( GConfig ){
		const FConfigFile*	engine_file= GConfig->Find( GEngineIni );
		if( engine_file ){
			static const TCHAR*	section_name_list[]= {
				TEXT("RemoteConsole2Plugin"),
				TEXT("RemoteConsolePlugin2"),
				TEXT("RemoteControllerPlugin"),
			};
			for( unsigned int pi= 0 ; pi< sizeof(section_name_list)/sizeof(TCHAR*) ; pi++ ){
				const TCHAR*	section_name= section_name_list[pi];
				if( engine_file->FindSection( section_name ) ){
					engine_file->GetBool( section_name, TEXT("bEnabled"),    bEnabled     );
					engine_file->GetBool( section_name, TEXT("bCaptureLog"), bCaptureLog  );
					engine_file->GetInt(  section_name, TEXT("Port"),        Port         );
					engine_file->GetInt(  section_name, TEXT("IPV"),         IPV          );
					break;
				}
			}
		}
	}

	const auto&	mapper= IPlatformInputDeviceMapper::Get();
	UserID= mapper.GetPrimaryPlatformUser();
	DeviceID= mapper.GetPrimaryInputDeviceForUser(UserID);

	if( bEnabled ){
		UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:Start Server"));
		Server.StartServer( nullptr, Port, IPV );
		if( bCaptureLog ){
			if( GLog ){
				UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:Start Logging"));
				RemoteDevice.SetServer( &Server );
				GLog->AddOutputDevice( &RemoteDevice );
			}
		}
		Initialize();
	}
}


FRemoteInputDevice::~FRemoteInputDevice()
{
	if( bEnabled ){
		if( bCaptureLog ){
			RemoteDevice.SetServer( nullptr );
			if( GLog ){
				UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:Stop Logging"));
				GLog->RemoveOutputDevice( &RemoteDevice );
			}
		}
		UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:Stop Server"));
		Server.StopServer();
		if( iGameAPI ){
			iGameAPI->Quit();
			delete iGameAPI;
			iGameAPI= nullptr;
		}
	}
}


void	FRemoteInputDevice::Initialize()
{
	if( !iGameAPI ){
		iGameAPI= new FGameAccessAPI();
		iGameAPI->Init();
	}
	bInitialized= false;
}


void	FRemoteInputDevice::Tick( float delta_time )
{
	if( bEnabled ){
		TickTotal+= delta_time;
		if( GEngine && GWorld ){
			if( !bInitialized ){
				Initialize();
			}
			ExecGameAPI();
		}
	}
}


//-----------------------------------------------------------------------------

void	FRemoteInputDevice::SendStick( FRemoteConsoleServer3::ControllerStatus& status, uint32_t stick_id, uint32_t bit_minus, uint32_t bit_plus, const FName& stick_name, bool reverse )
{
	const float	ANALOG_SHRESHOLD= 0.7f;
	int16_t		stick= status.Analog[stick_id];
	if( stick != PrevStatus.Analog[stick_id] ){
		float	fvalue= stick * (1.0f/32767.0f);
		if( reverse ){
			fvalue= -fvalue;
		}
		MessageHandler->OnControllerAnalog( stick_name, UserID, DeviceID, fvalue );
		if( bit_minus ){
			if( fvalue < -ANALOG_SHRESHOLD ){
				status.Button|= bit_minus;
			}else{
				status.Button&= ~bit_minus;
			}
			if( fvalue > ANALOG_SHRESHOLD ){
				status.Button|= bit_plus;
			}else{
				status.Button&= ~bit_plus;
			}
		}
	}
}


void	FRemoteInputDevice::SendButton( uint32_t trigger, uint32_t button, uint32_t bit, const FName& button_name )
{
	if( trigger & bit ){
		if( button & bit ){
			MessageHandler->OnControllerButtonPressed( button_name, UserID, DeviceID, false );
		}else{
			MessageHandler->OnControllerButtonReleased( button_name, UserID, DeviceID, false );
		}
	}
}


void	FRemoteInputDevice::ClearKeycodeState()
{
	memset( KeycodeStateMap, 0, sizeof(KeycodeStateMap) );
	KeycodeMax= 0;
}


void	FRemoteInputDevice::SetKeycodeState( uint32_t keycode )
{
	if( keycode <= KEYCODE_MAX ){
		KeycodeStateMap[ keycode >> 5 ]|= 1<< (keycode & 0x1f);
	}
}


void	FRemoteInputDevice::ResetKeycodeState( uint32_t keycode )
{
	if( keycode <= KEYCODE_MAX ){
		KeycodeStateMap[ keycode >> 5 ]&= ~(1<< (keycode & 0x1f));
	}
}


bool	FRemoteInputDevice::GetKeycodeState( uint32_t keycode ) const
{
	if( keycode <= KEYCODE_MAX ){
		if( KeycodeStateMap[ keycode >> 5 ] & (1<< (keycode & 0x1f)) ){
			return	true;
		}
	}
	return	false;
}


//-----------------------------------------------------------------------------

enum : uint32_t {
	PAD_UP		=	(1u<< 0),
	PAD_DOWN	=	(1u<< 1),
	PAD_LEFT	=	(1u<< 2),
	PAD_RIGHT	=	(1u<< 3),
	PAD_TA		=	(1u<< 4),	// Cross
	PAD_TB		=	(1u<< 5),	// Circle
	PAD_TX		=	(1u<< 6),	// Square
	PAD_TY		=	(1u<< 7),	// Triangle
	PAD_TL1		=	(1u<< 8),
	PAD_TR1		=	(1u<< 9),
	PAD_TL2		=	(1u<<10),
	PAD_TR2		=	(1u<<11),
	PAD_TL3		=	(1u<<12),	// ThumbL/L3
	PAD_TR3		=	(1u<<13),	// ThumbR/R3
	PAD_MODE0	=	(1u<<14),	// SELECT/BACK/SHARE
	PAD_MODE1	=	(1u<<15),	// START/OPTIONS
	PAD_MODE2	=	(1u<<16),	// PS/X/HOME
	PAD_MODE3	=	(1u<<17),	// TouchPad Click

	PAD_LS_U	=	(1u<<24),
	PAD_LS_D	=	(1u<<25),
	PAD_LS_L	=	(1u<<26),
	PAD_LS_R	=	(1u<<27),
	PAD_RS_U	=	(1u<<28),
	PAD_RS_D	=	(1u<<29),
	PAD_RS_L	=	(1u<<30),
	PAD_RS_R	=	(1u<<31),
};


void	FRemoteInputDevice::SendControllerEvents()
{
	static constexpr float	TICK_DELTA= 1.0f / SAMPLING_RATE;
	if( !bEnabled ){
		return;
	}
	if( TickTotal < TICK_DELTA ){
		return;
	}
	TickTotal-= TICK_DELTA;

	// Controller
	FRemoteConsoleServer3::ControllerStatus	status;
	if( !Server.GetControllerStatus( status ) ){
		memset( &status, 0, sizeof(status) );
	}

	// Analog Stick / Analog Trigger
	//				staick_id	minus		plus		name
	SendStick( status, 0,		PAD_LS_L,	PAD_LS_R,	FGamepadKeyNames::LeftAnalogX				);
	SendStick( status, 1,		PAD_LS_U,	PAD_LS_D,	FGamepadKeyNames::LeftAnalogY,		true	);
	SendStick( status, 2,		PAD_RS_L,	PAD_RS_R,	FGamepadKeyNames::RightAnalogX				);
	SendStick( status, 3,		PAD_RS_U,	PAD_RS_D,	FGamepadKeyNames::RightAnalogY,		true	);
	SendStick( status, 4,		0,			0,			FGamepadKeyNames::LeftTriggerAnalog			);
	SendStick( status, 5,		0,			0,			FGamepadKeyNames::RightTriggerAnalog		);

	// Digital Button
	{
		uint32_t	button= status.Button;
		uint32_t	trigger= PrevStatus.Button ^ button;

		if( trigger ){
			static const struct ButtonMap {
				uint32_t		Button;
				const FName*	Name;
			}	pad_button_table[]= {
				{	PAD_TA,		&FGamepadKeyNames::FaceButtonBottom			},
				{	PAD_TB,		&FGamepadKeyNames::FaceButtonRight			},
				{	PAD_TX,		&FGamepadKeyNames::FaceButtonLeft			},
				{	PAD_TY,		&FGamepadKeyNames::FaceButtonTop			},
				{	PAD_TL1,	&FGamepadKeyNames::LeftShoulder				},
				{	PAD_TR1,	&FGamepadKeyNames::RightShoulder			},
				{	PAD_TL2,	&FGamepadKeyNames::LeftTriggerThreshold		},
				{	PAD_TR2,	&FGamepadKeyNames::RightTriggerThreshold	},
				{	PAD_TL3,	&FGamepadKeyNames::LeftThumb				},
				{	PAD_TR3,	&FGamepadKeyNames::RightThumb				},
				{	PAD_UP, 	&FGamepadKeyNames::DPadUp					},
				{	PAD_DOWN, 	&FGamepadKeyNames::DPadDown					},
				{	PAD_LEFT, 	&FGamepadKeyNames::DPadLeft					},
				{	PAD_RIGHT,	&FGamepadKeyNames::DPadRight				},
				{	PAD_MODE0,	&FGamepadKeyNames::SpecialLeft				},
				{	PAD_MODE1,	&FGamepadKeyNames::SpecialRight				},
				{	PAD_MODE2,	&FGamepadKeyNames::SpecialLeft_X			},
				{	PAD_MODE3,	&FGamepadKeyNames::SpecialLeft_Y			},

				{	PAD_LS_U,	&FGamepadKeyNames::LeftStickUp				},
				{	PAD_LS_D,	&FGamepadKeyNames::LeftStickDown			},
				{	PAD_LS_L,	&FGamepadKeyNames::LeftStickLeft			},
				{	PAD_LS_R,	&FGamepadKeyNames::LeftStickRight			},
				{	PAD_RS_U,	&FGamepadKeyNames::RightStickUp				},
				{	PAD_RS_D,	&FGamepadKeyNames::RightStickDown			},
				{	PAD_RS_L,	&FGamepadKeyNames::RightStickLeft			},
				{	PAD_RS_R,	&FGamepadKeyNames::RightStickRight			},
			};
			for( uint32_t ti= 0 ; ti< sizeof(pad_button_table)/sizeof(ButtonMap) ; ti++ ){
				const auto&	pad= pad_button_table[ti];
				SendButton( trigger, button, pad.Button, *pad.Name );
			}
		}
	}

	PrevStatus= status;


	// Keyboard / Mouse
	if( Server.GetKeyboardStatus( KeyboardStatusBuffer ) ){
		check( sizeof(FRemoteConsoleServer3::KeyboardStatus) == sizeof(FRemoteConsoleServer3::MouseStatus) );
		// Online
		auto	buffer_size= KeyboardStatusBuffer.Num();
		for( int i= 0 ; i< buffer_size ; i++ ){
			const auto&	key= KeyboardStatusBuffer[i];
			const auto*	mouse= reinterpret_cast<const FRemoteConsoleServer3::MouseStatus*>( &key );
			bool		repeat= (key.Action & FRemoteConsoleServer3::KEY_REPEAT) != 0;
			uint32_t	action= key.Action & FRemoteConsoleServer3::KEY_ACTION_MASK;
			switch( action ){
			case FRemoteConsoleServer3::KEY_CHAR:
				MessageHandler->OnKeyChar( key.CharCode, repeat );
				break;
			case FRemoteConsoleServer3::KEY_DOWN:
				MessageHandler->OnKeyDown( key.KeyCode, key.CharCode, repeat );
				SetKeycodeState( key.KeyCode );
				if( key.KeyCode+1u > KeycodeMax ){
					KeycodeMax= key.KeyCode+1u;
				}
				break;
			case FRemoteConsoleServer3::KEY_UP:
				MessageHandler->OnKeyUp( key.KeyCode, key.CharCode, repeat );
				ResetKeycodeState( key.KeyCode );
				break;
			case FRemoteConsoleServer3::MOUSE_DOUBLE_CLICK:
			case FRemoteConsoleServer3::MOUSE_DOWN:
				if( GEngine && GEngine->GameViewport ){
					TSharedPtr<SWindow>	window= GEngine->GameViewport->GetWindow();
					if( window ){
						if( action == FRemoteConsoleServer3::MOUSE_DOWN ){
							MessageHandler->OnMouseDown( window->GetNativeWindow(), static_cast<EMouseButtons::Type>(mouse->Button) );
						}else{
							MessageHandler->OnMouseDoubleClick( window->GetNativeWindow(), static_cast<EMouseButtons::Type>(mouse->Button) );
						}
					}
				}
				break;
			case FRemoteConsoleServer3::MOUSE_UP:
				MessageHandler->OnMouseUp( static_cast<EMouseButtons::Type>(mouse->Button) );
				break;
			case FRemoteConsoleServer3::MOUSE_MOVE:
				MessageHandler->OnRawMouseMove( mouse->CursorX, mouse->CursorY );
				break;
			case FRemoteConsoleServer3::MOUSE_SETPOS:
				{
					int32	posx= 0;
					int32	posy= 0;
					if( GEngine && GEngine->GameViewport ){
						TSharedPtr<SWindow>	window= GEngine->GameViewport->GetWindow();
						if( window ){
							TSharedPtr<FGenericWindow>	native= window->GetNativeWindow();
							int32	width= 0;
							int32	height= 0;
							native->GetRestoredDimensions( posx, posy, width, height );
							int32	border_size= native->GetWindowBorderSize();
							int32	titlebar_size= native->GetWindowTitleBarSize();
							posx+= border_size;
							posy+= border_size + titlebar_size;
						}
					}
					auto&	app= FSlateApplication::Get();
					app.SetCursorPos( FVector2D( mouse->CursorX + posx, mouse->CursorY + posy ) );
					MessageHandler->OnMouseMove();
				}
				break;
			}

		}
	}else{
		// Offline
		for( uint32_t ki= 0 ; ki< KeycodeMax ; ki++ ){
			if( GetKeycodeState( ki ) ){
				MessageHandler->OnKeyUp( ki, ki, 0 );
				ResetKeycodeState( ki );
			}
		}
		KeycodeMax= 0;
	}
}


//-----------------------------------------------------------------------------

void	FRemoteInputDevice::SetChannelValue( int32 controller_id, FForceFeedbackChannelType type, float value )
{
}


void	FRemoteInputDevice::SetChannelValues( int32 controller_id, const FForceFeedbackValues& values )
{
}


void	FRemoteInputDevice::SetMessageHandler( const TSharedRef<FGenericApplicationMessageHandler>& handler )
{
	MessageHandler= handler;
}


bool	FRemoteInputDevice::Exec( UWorld* world, const TCHAR* cmd, FOutputDevice& Ar )
{
	return	false;
}


bool	FRemoteInputDevice::IsGamepadAttached() const
{
	return	true;
}


//-----------------------------------------------------------------------------

void	FRemoteInputDevice::ExecGameAPI()
{
	if( Server.GetGameAPIStatus( GameAPICommandBuffer ) ){
		for( const auto& command : GameAPICommandBuffer ){
			if( iGameAPI ){
				iGameAPI->SetResultKey( command.Param1, &Server );
			}
			switch( command.Command ){
			case FRemoteConsoleServer3::CMD_CONSOLE_CMD:
				if( iGameAPI ){
					iGameAPI->ExecConsoleCommand( *command.StringParam );
				}
				break;
			case FRemoteConsoleServer3::CMD_PRINT_LOG:
				UE_LOG( LogRemoteConsole2, Log,TEXT("%s"), *command.StringParam );
				break;
			case FRemoteConsoleServer3::CMD_UI_BUTTON:
				if( iGameAPI ){
					iGameAPI->WidgetButton( *command.StringParam, command.Param0 );
				}
				break;
			case FRemoteConsoleServer3::CMD_UI_DUMP:
				if( iGameAPI ){
					iGameAPI->DumpWidget( command.Param0 );
				}
				break;
			case FRemoteConsoleServer3::CMD_GET_LEVEL_NAME:
				if( iGameAPI ){
					iGameAPI->GetCurrentLevelName();
				}
				break;
			case FRemoteConsoleServer3::CMD_UI_FOCUS:
				if( iGameAPI ){
					iGameAPI->SetFocus( *command.StringParam, command.Param0 );
				}
				break;
			case FRemoteConsoleServer3::CMD_GET_CONSOLE_VAR:
				if( iGameAPI ){
					iGameAPI->GetConsoleVar( *command.StringParam );
				}
				break;
			default:
				UE_LOG(LogRemoteConsole2,Warning,TEXT("RemoteConsole2:Unknown Command=%d"), command.Command );
				break;
			}
		}
		GameAPICommandBuffer.Reset();
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

