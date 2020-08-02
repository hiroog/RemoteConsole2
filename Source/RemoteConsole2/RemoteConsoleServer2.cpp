// RemoteConsole2 2020/08/01 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "RemoteConsoleServer2.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FRemoteConsoleServer2::FRemoteConsoleServer2()
{
}

FRemoteConsoleServer2::~FRemoteConsoleServer2()
{
	StopServer();
}

bool	FRemoteConsoleServer2::Init()
{
	return	true;
}

void	FRemoteConsoleServer2::Stop()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void	FRemoteConsoleServer2::RecvAll( FSocket* socket, uint8* buffer, int32 data_size )
{
	uint8*	ptr= buffer;
	for(; !bStopFlag ;){
		int32	read_size= 0;
		socket->Recv( ptr, data_size, read_size );
		if( !read_size ){
			break;
		}
		data_size-= read_size;
		ptr+= read_size;
		if( data_size <= 0 ){
			break;
		}
	}
}


uint32	FRemoteConsoleServer2::Run()
{
	//UE_LOG( LogTemp, Log, TEXT( "FRemoteConsoleServer2:Run()" ) );
	FIPv4Endpoint	endpoint( FIPv4Address::Any, Port );
//	FIPv6Endpoint	endpoint6( FIPv6Address::Any, Port );

	const int32	buffer_size= 1024*2;
	FSocket*	listen_sock= FTcpSocketBuilder( TEXT("listen") )
					.BoundToEndpoint( endpoint )
					.AsReusable()
					.WithReceiveBufferSize( buffer_size )
					.WithSendBufferSize( buffer_size );
	listen_sock->Listen(1);

	for(; !bStopFlag ;){
		bool	link_flag= false;
		listen_sock->HasPendingConnection( link_flag );
		if( link_flag ){
			TSharedPtr<FInternetAddr>	connect_addr= ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();
			FSocket*	sock= listen_sock->Accept( *connect_addr, TEXT("tcpsock") );

			FString	addr_str= connect_addr->ToString(true);
			//UE_LOG( LogTemp, Log, TEXT("**** Accept %s"), *addr_str );

			DataHeader	header;
			uint8	data_buffer[256+2];

			RecvAll( sock, reinterpret_cast<uint8*>(&header), sizeof(DataHeader) );
			if( header.DataLength ){
				RecvAll( sock, data_buffer, header.DataLength );
				data_buffer[header.DataLength]= '\0';
			}
			switch( header.Command ){
			case CMD_NOP:
				break;
			case CMD_EXIT:
				bStopFlag= true;
				break;
			case CMD_CONSOLE_CMD:
				Lock.Lock();
				CommandArray.Add( FString( (char*)data_buffer ) );
				Lock.Unlock();
				bRecvCommand= true;
				break;
			}
			sock->Close();
			continue;
		}

		FPlatformProcess::Sleep( 0.1f );
	}

	listen_sock->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(listen_sock);

	//UE_LOG( LogTemp, Log, TEXT( "FRemoteConsoleServer2:Run() Exit" ) );
	return	0;
}


void	FRemoteConsoleServer2::StartServer( const char* host, int port, int ipv )
{
	Port= port;
	IPv= ipv;
	bStopFlag= false;
	bRecvCommand= false;

	iThread= FRunnableThread::Create( this, TEXT("RemoteConsoleServer2"), 0, TPri_BelowNormal );
}


void	FRemoteConsoleServer2::StopServer()
{
	bStopFlag= true;

	if( iThread ){
		iThread->WaitForCompletion();
		delete iThread;
		iThread= nullptr;
	}
}


void	FRemoteConsoleServer2::Flush( UWorld* world )
{
	if( iThread ){
		if( bRecvCommand ){
			Lock.Lock();
			unsigned int	ccount= CommandArray.Num();
			for( unsigned int ci= 0 ; ci< ccount ; ci++ ){
				//UE_LOG( LogTemp, Log, TEXT("ExecCommand %s"), *CommandArray[ci] );
				GEngine->Exec( world, *CommandArray[ci] );
			}
			CommandArray.Empty();
			bRecvCommand= false;
			Lock.Unlock();
		}
	}
}



