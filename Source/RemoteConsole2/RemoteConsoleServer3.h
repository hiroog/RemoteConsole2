// RemoteConsole2 2020/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once

#include "CoreMinimal.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include "HAL/Runnable.h"
#include "Templates/Atomic.h"
//#include "ConsoleStringPool.h"
#include "ConsoleMessageQueue.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FThreadInstance;

class FRemoteConsoleServer3 : public FRunnable {
public:
	struct DataHeader {
		uint32_t	Magic;		// 0x70198FB3
		uint16_t	Command;
		uint16_t	Param0;
		uint32_t	DataSize;
		uint32_t	Param1;
	};
	enum : uint16_t {
		CMD_NOP,
		CMD_EXITSERVER,
		CMD_CLOSE,
		CMD_ERROR,
		CMD_HEARTBEAT,
		CMD_USER	= 256,
		CMD_CONTROLLER,		// data:Controller
		CMD_KEYBOARD,		// param0:0=Down,1=Up,2=Char param1:code
		CMD_MOUSE,			//
		CMD_RETURN_LOG,
		CMD_RETURN_STRING,
		CMD_RECV_SERVER		=	384,
		CMD_GAME_API_BASE	=	512,
		CMD_CONSOLE_CMD,	// data:Command
		CMD_PRINT_LOG,		// param1:ResultKey  data:String
		CMD_UI_DUMP,		// param1:ResultKey
		CMD_UI_BUTTON,		// param0:0=Clicked,1=Pressed,2=Released,3=Hover,4=Unhover  param1:ResultKey  data:WidgetName
		CMD_UI_FOCUS,		// param0:0=GameOnly,1=UIOnly,2=GameAndUI  param1:ResultKey  data:WidgetName
		CMD_GET_LEVEL_NAME,	// pram1:ResultKey
	};
	static constexpr size_t		COMMAND_BUFFER_SIZE	=	256*2;
	static constexpr uint32_t	GAME_API_QUEUE_SIZE	=	128;
	static constexpr uint32_t	RESULT_QUEUE_SIZE	=	1024*8;
	//-------------------------------------------------------------------------
	struct ControllerStatus {
		int16_t		Analog[6];
		uint32_t	Button;
	};
	enum : uint16_t {
		KEY_UP,
		KEY_DOWN,
		KEY_CHAR,
	};
	struct KeyboardStatus {
		uint16_t	KeyCode;
		uint16_t	CharCode;
		uint16_t	Down;	// 0=Up, 1=Down, 2=Char, 0x80=Repeat
	};
	enum : uint16_t {
		UI_BUTTON_CLICKED,
		UI_BUTTON_PRESSED,
		UI_BUTTON_RELEASED,
		UI_BUTTON_HOVERED,
		UI_BUTTON_UNHOVERED,
	};
	enum : uint16_t {
		UI_FOCUS_GAMEONLY,
		UI_FOCUS_UIONLY,
		UI_FOCUS_GAMEANDUI,
	};
	struct CaptureStatus {
		ControllerStatus	Status;
		double		Time;
	};
	//-------------------------------------------------------------------------

private:
	FRunnableThread*		iThread= nullptr;
	//-------------------------------------------------------------------------
	FCriticalSection		ControllerLock;
	FCriticalSection		KeyboardLock;
	FCriticalSection		GameAPILock;
	//-------------------------------------------------------------------------
	TArray<FThreadInstance*>	ThreadArray;
	TArray<KeyboardStatus>		KeyboardBuffer;
	//TArray<CaptureStatus>		CaptureBuffer;
	FConsoleMessageQueue		GameAPIBuffer{ false, GAME_API_QUEUE_SIZE };
	FConsoleMessageQueue		ResultBuffer{ true, RESULT_QUEUE_SIZE };
	ControllerStatus			Status;
	//-------------------------------------------------------------------------
	int32_t	Port= 10101;
	int32_t	IPv= 4;
	//-------------------------------------------------------------------------
	std::atomic<bool>	bStopRequest= false;
	std::atomic<bool>	bServerLoop= true;
	std::atomic<bool>	bIsKeyboardOnline= false;
	std::atomic<bool>	bIsControllerOnline= false;
private:
	bool	Init() override;
	uint32	Run() override;
	void	Stop() override;
	//-------------------------------------------------------------------------
	bool	RecvAll( FSocket* socket, uint8_t* buffer, int32_t data_size );
	bool	SendAll( FSocket* socket, const uint8_t* buffer, int32_t data_size );
	void	CheckRelease();
	void	StopConnection();
	bool	CommandExec( FSocket* sock, const DataHeader& header, const uint8_t* data );
	//-------------------------------------------------------------------------
	void	ClearControllerStatus();
	bool	SendResult( FSocket* socket, DataHeader& result_header, const char8_t* text );
public:
	FRemoteConsoleServer3();
	virtual ~FRemoteConsoleServer3();
	//-------------------------------------------------------------------------
	void	StartServer( const char* host= nullptr, int port= 10101, int ipv= 4 );
	void	StopServer();
	//-------------------------------------------------------------------------
	bool	GetControllerStatus( ControllerStatus& status );
	bool	GetKeyboardStatus( TArray<KeyboardStatus>& status );
	bool	GetGameAPIStatus( TArray<FConsoleMessageQueue::Message>& status );
	void	PushResult( const FConsoleMessageQueue::Message& result );
	void	PushTextResult( const TCHAR* result, uint32_t param0, uint32_t param1 );
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

