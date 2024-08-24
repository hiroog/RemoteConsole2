// RemoteConsole2 2020/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "RemoteConsoleServer3.h"
#include "RemoteConsole2.h"
#include "RemoteOutputDevice.h"
#include "Networking.h"
#include "SocketSubsystem.h"


#define	RC2_USE_LOWLEVEL_OUTPUT_3	1

#if RC2_USE_LOWLEVEL_OUTPUT_3
# include "HAL/PlatformMisc.h"
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FThreadInstance : public FRunnable {
	FRunnableThread*	iThread= nullptr;
	std::atomic<bool>	bActive= true;
	FSocket*			iSocket= nullptr;
public:
	void	Start( const TCHAR* thread_name, FSocket* sock )
	{
		if( iThread ){
			Join();
		}
		iSocket= sock;
		iThread= FRunnableThread::Create( this, thread_name, 0, TPri_BelowNormal );
	}
	void	CloseSocket()
	{
		if( iSocket ){
			iSocket->Shutdown( ESocketShutdownMode::ReadWrite );
			iSocket->Close();
			ISocketSubsystem*	SocketSubsystem= ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
			if( SocketSubsystem ){
				SocketSubsystem->DestroySocket( iSocket );
			}
			iSocket= nullptr;
		}
	}
	void	Join()
	{
		if( iThread ){
			CloseSocket();
			iThread->WaitForCompletion();
			delete iThread;
			iThread= nullptr;
		}
	}
	bool	IsActive()
	{
		return	bActive.load();
	}
	void	SetActive( bool flag )
	{
		bActive.store( flag );
	}
};

template<typename T>
class FServerThreadInstance : public FThreadInstance {
	T	Func;
public:
	explicit FServerThreadInstance( T&& func ) : Func( std::forward<T>( func ) )
	{
	}
	uint32	Run() override
	{
		Func( this );
		return	0;
	}
};

template<typename T>
inline FThreadInstance*	CreateServerThreadInstance( T&& func )
{
	return	new FServerThreadInstance<T>( std::forward<T>(func) );
}

inline void	ReleaseServerThreadInstance( FThreadInstance*& ithread )
{
	if( ithread ){
		ithread->Join();
		delete ithread;
		ithread= nullptr;
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FRemoteConsoleServer3::FRemoteConsoleServer3()
{
	ClearControllerStatus();
}

FRemoteConsoleServer3::~FRemoteConsoleServer3()
{
	StopServer();
}

bool	FRemoteConsoleServer3::Init()
{
	return	true;
}

void	FRemoteConsoleServer3::Stop()
{
}


//-----------------------------------------------------------------------------

bool	FRemoteConsoleServer3::RecvAll( FSocket* socket, uint8_t* buffer, int32_t data_size )
{
	uint8_t*	ptr= buffer;
	for(; !bStopRequest.load() ;){
		int32_t	read_size= 0;
		if( !socket->Recv( ptr, data_size, read_size ) ){ // error of eod
			return	false;
		}
		if( !read_size ){ // eod
			return	false;
		}
		data_size-= read_size;
		ptr+= read_size;
		if( data_size <= 0 ){
			return	true;
		}
	}
	return	false;
}


bool	FRemoteConsoleServer3::SendAll( FSocket* socket, const uint8_t* buffer, int32_t data_size )
{
	const uint8_t*	ptr= buffer;
	for(; !bStopRequest.load() ;){
		int32_t	wrote_size= 0;
		if( !socket->Send( ptr, data_size, wrote_size ) ){
			return	false;
		}
		data_size-= wrote_size;
		ptr+= wrote_size;
		if( data_size <= 0 ){
			return	true;
		}
	}
	return	false;
}


uint32	FRemoteConsoleServer3::Run()
{
	ISocketSubsystem*	SocketSubsystem= ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
	FSocket*	listen_sock= SocketSubsystem->CreateSocket( NAME_Stream, TEXT("default"), IPv == 6 ?  FNetworkProtocolTypes::IPv6 : FNetworkProtocolTypes::IPv4 );
	TSharedPtr<FInternetAddr>	addr= SocketSubsystem->CreateInternetAddr();
	bool	result= false;
	if( IPv == 6 ){
		addr->SetIp( TEXT("::"), result );
	}else{
		addr->SetIp( TEXT("0.0.0.0"), result );
	}
	addr->SetPort( Port );
	listen_sock->Bind( *addr );
	listen_sock->Listen( 5 );

	bServerLoop.store( true );
	for(; bServerLoop.load() ;){

		bool	link_flag= false;
		listen_sock->HasPendingConnection( link_flag );

		if( link_flag ){
			TSharedPtr<FInternetAddr>	connect_addr= SocketSubsystem->CreateInternetAddr();
			FSocket*	sock= listen_sock->Accept( *connect_addr, TEXT("tcpsock") );
			FString	addr_str= connect_addr->ToString( true );

			auto*	tp= CreateServerThreadInstance( [this,sock]( FThreadInstance* tptr ) mutable {
					ISocketSubsystem*	SocketSubsystem= ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
					bool	has_controller_cmd= false;
					bool	has_keyboard_cmd= false;
					bool	loop_flag= true;
					for(; loop_flag ;){
						DataHeader	header;
						uint8_t	data_buffer[COMMAND_BUFFER_SIZE];
						if( !RecvAll( sock, reinterpret_cast<uint8*>(&header), sizeof(DataHeader) ) ){
							break;
						}
						if( header.Magic != 0x70198fb3 ){
							UE_LOG( LogRemoteConsole2, Error, TEXT("RemoteConsole2: Invalid header %x"), header.Magic );
							break;
						}
						data_buffer[0]= '\0';
						if( header.DataSize ){
							int32_t	data_size= header.DataSize;
							if( data_size >= COMMAND_BUFFER_SIZE ){
								data_size= COMMAND_BUFFER_SIZE-1;
							}
							if( !RecvAll( sock, data_buffer, data_size ) ){
								break;
							}
							data_buffer[data_size]= '\0';
						}
						if( header.Command != CMD_CONTROLLER ){
#if RC2_USE_LOWLEVEL_OUTPUT_3
							FPlatformMisc::LowLevelOutputDebugString( *FString::Printf( TEXT("Server Recv cmd=%d dsize=%d tp=%p\n"), header.Command, header.DataSize, (void*)tptr ) );
#else
							UE_LOG( LogRemoteConsole2, Verbose, TEXT("Server Recv cmd=%d dsize=%d"), header.Command, header.DataSize );
#endif
						}
						switch( header.Command ){
						case CMD_NOP:
							break;
						case CMD_CLOSE:
							loop_flag= false;
							break;
						case CMD_EXITSERVER:
							loop_flag= false;
							this->bServerLoop.store( false );
							break;
						default:
							switch( header.Command ){
							case CMD_CONTROLLER:
								has_controller_cmd= true;
								break;
							case CMD_MOUSE:
							case CMD_KEYBOARD:
								has_keyboard_cmd= true;
								break;
							default:
								break;
							}
							if( !this->CommandExec( sock, header, data_buffer ) ){
								loop_flag= false;
							}
							break;
						}
						if( this->bStopRequest.load() ){
							break;
						}
					}
					if( has_controller_cmd ){
						this->ClearControllerStatus();
						this->bIsControllerOnline.store( false );
						has_controller_cmd= false;
					}
					if( has_keyboard_cmd ){
						this->bIsKeyboardOnline.store( false );
						has_keyboard_cmd= false;
					}
					tptr->CloseSocket();
					UE_LOG( LogRemoteConsole2, Verbose, TEXT("RemoteConsole2:Exit CommandServer Thread %p"), (void*)tptr );
					tptr->SetActive( false );
				} );
			ThreadArray.Add( tp );
			UE_LOG( LogRemoteConsole2, Verbose, TEXT("RemoteConsole2:Start CommandServer Thread %p"), (void*)tp );
			tp->Start( TEXT("RemoteConsoleServer2-Connection"), sock );

		}else{
			FPlatformProcess::Sleep( 0.1f );
		}

		CheckRelease();
	}
	listen_sock->Close();
	//SocketSubsystem= ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM );
	SocketSubsystem->DestroySocket( listen_sock );
	return	0;
}


//-----------------------------------------------------------------------------

void	FRemoteConsoleServer3::ClearControllerStatus()
{
	ControllerLock.Lock();
	memset( &Status, 0, sizeof(Status) );
	ControllerLock.Unlock();
}

bool	FRemoteConsoleServer3::SendResult( FSocket* sock, DataHeader& result_header, const char8_t* text )
{
	uint32_t	data_size= 0;
	if( text ){
		data_size= static_cast<uint32_t>( strlen( reinterpret_cast<const char*>(text) ) );
	}
	result_header.DataSize= data_size;
#if RC2_USE_LOWLEVEL_OUTPUT_3
	FPlatformMisc::LowLevelOutputDebugString( *FString::Printf( TEXT("Server Send cmd=%d dsize=%d\n"), result_header.Command, result_header.DataSize ) );
#else
	UE_LOG( LogRemoteConsole2, Verbose, TEXT("Server Send cmd=%d dsize=%d"), result_header.Command, result_header.DataSize );
#endif
	if( !SendAll( sock, reinterpret_cast<const uint8_t*>(&result_header), sizeof(result_header) ) ){
		return	false;
	}
	if( data_size ){
		if( !SendAll( sock, reinterpret_cast<const uint8_t*>(text), static_cast<int32_t>(data_size) ) ){
			return	false;
		}
	}
	return	true;
}

bool	FRemoteConsoleServer3::CommandExec( FSocket* sock, const DataHeader& header, const uint8_t* data )
{
	if( header.Command >= CMD_GAME_API_BASE ){
		FConsoleMessageQueue::Message	api;
		api.Command= header.Command;
		api.Param0= header.Param0;
		api.Param1= header.Param1;
		api.StringParam= FString( reinterpret_cast<const UTF8CHAR*>(data) );
		GameAPIBuffer.Push( api );
		return	true;
	}
	switch( header.Command ){
	case CMD_CONTROLLER:
		if( header.DataSize == sizeof(ControllerStatus) ){
			ControllerLock.Lock();
			memcpy( &Status, data, sizeof(ControllerStatus) );
			bIsControllerOnline.store( true );
			ControllerLock.Unlock();
		}else{
			UE_LOG( LogRemoteConsole2, Error, TEXT("RemoteConsoleServer2: Controller Status size unmatch dsize=%d"), header.DataSize );
		}
		return	true;
	case CMD_RECV_SERVER:
		if( header.DataSize == 0 ){
			TArray<FConsoleMessageQueue::Message>	result_array;
			for(; !bStopRequest.load() ;){
				if( ResultBuffer.WaitMessage( result_array, 1.0f ) ){
					DataHeader	result_header;
					memset( &result_header, 0, sizeof(result_header) );
					result_header.Magic= 0x70198fb3;
					for( const auto& result : result_array ){
						result_header.Command= result.Command;
						result_header.Param0= result.Param0;
						result_header.Param1= result.Param1;
						if( result.StringParam.IsEmpty() ){
							if( !SendResult( sock, result_header, nullptr ) ){
								return	false;
							}
						}else{
							if( !SendResult( sock, result_header, reinterpret_cast<char8_t*>( TCHAR_TO_UTF8( *result.StringParam ) ) ) ){
								return	false;
							}
						}
						if( bStopRequest.load() ){
							return	false;
						}
					}
				}
			}
		}else{
			UE_LOG( LogRemoteConsole2, Error, TEXT("RemoteConsoleServer2: Read Console Status size unmatch dsize=%d"), header.DataSize );
		}
		return	false;
	case CMD_MOUSE:
	case CMD_KEYBOARD:
		if( header.DataSize == 0 ){
			KeyboardStatus	key;
			key.KeyCode=  header.Param1 & 0xffff;	// or X
			key.CharCode= header.Param1 >> 16;		// or Y
			key.Action=   header.Param0;
			KeyboardLock.Lock();
			KeyboardBuffer.Add( key );
			bIsKeyboardOnline.store( true );
			KeyboardLock.Unlock();
		}else{
			UE_LOG( LogRemoteConsole2, Error, TEXT("RemoteConsoleServer2: Keyboard/Mouse Status size unmatch dsize=%d"), header.DataSize );
		}
		return	true;
	default:
		break;
	}
	UE_LOG( LogRemoteConsole2, Error, TEXT("RemoteConsoleServer2: Command Error cmd=%d dsize=%d"), header.Command, header.DataSize );
	return	true;
}


void	FRemoteConsoleServer3::CheckRelease()
{
	unsigned int	tcount= ThreadArray.Num();
	for( unsigned int ti= 0 ; ti< tcount ; ti++ ){
		auto*	tptr= ThreadArray[ti];
		if( tptr ){
			if( !tptr->IsActive() ){
				UE_LOG( LogRemoteConsole2, Verbose, TEXT("RemoteConsoleServer2:Join Thread %d/%d %p"), ti, tcount, (void*)tptr );
				ReleaseServerThreadInstance( ThreadArray[ti] );
				ThreadArray.RemoveAtSwap( ti, EAllowShrinking::No );
				tcount--;
				ti--;
			}
		}
	}
}


void	FRemoteConsoleServer3::StartServer( const char* host, int32_t port, int32_t ipv )
{
	if( iThread ){
		StopServer();
	}
	Port= port;
	IPv= ipv;
	bStopRequest= false;

	iThread= FRunnableThread::Create( this, TEXT("RemoteConsoleServer2"), 0, TPri_BelowNormal );
}


void	FRemoteConsoleServer3::StopConnection()
{
	bStopRequest.store( true );
	unsigned int	tcount= ThreadArray.Num();
	for( unsigned int ti= 0 ; ti< tcount ; ti++ ){
		if( ThreadArray[ti] ){
			ReleaseServerThreadInstance( ThreadArray[ti] );
		}
	}
}


void	FRemoteConsoleServer3::StopServer()
{
	bServerLoop.store( false );
	bStopRequest.store( true );
	StopConnection();
	if( iThread ){
		iThread->WaitForCompletion();
		delete iThread;
		iThread= nullptr;
	}
}


//-----------------------------------------------------------------------------

bool	FRemoteConsoleServer3::GetControllerStatus( ControllerStatus& status )
{
	if( bIsControllerOnline.load() ){
		ControllerLock.Lock();
		status= Status;
		ControllerLock.Unlock();
		return	true;
	}
	return	false;
}


bool	FRemoteConsoleServer3::GetKeyboardStatus( TArray<KeyboardStatus>& status )
{
	if( bIsKeyboardOnline.load() ){
		KeyboardLock.Lock();
		auto	buffer_size= KeyboardBuffer.Num();
		status.SetNum( buffer_size );
		memcpy( status.GetData(), KeyboardBuffer.GetData(), sizeof(KeyboardStatus) * buffer_size );
		KeyboardBuffer.Reset();
		KeyboardLock.Unlock();
		return	true;
	}
	return	false;
}


bool	FRemoteConsoleServer3::GetGameAPIStatus( TArray<FConsoleMessageQueue::Message>& status )
{
	if( GameAPIBuffer.HasMessage() ){
		if( GameAPIBuffer.GetMessage( status ) ){
			return	true;
		}
	}
	return	false;
}


void	FRemoteConsoleServer3::PushResult( const FConsoleMessageQueue::Message& result )
{
	ResultBuffer.Push( result );
}


void	FRemoteConsoleServer3::PushTextResult( const TCHAR* text, uint32_t param0, uint32_t param1 )
{
	FConsoleMessageQueue::Message	result;
	result.Command= CMD_RETURN_STRING;
	result.Param0= param0;
	result.Param1= param1;
	result.StringParam= FString( text );
	PushResult( result );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

