// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "RemoteConsoleServer.h"

#if USE_UALIB
#include <ualib/ualib.h>
#include <ualib/core/thread/ThreadInstance.h>
#include <ualib/core/thread/CriticalSection.h>
#include <ualib/core/network/network.h>
#include <ualib/core/network/CommandServer.h>

using namespace ualib;

namespace ualib {
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class ECommandServer : public network::CommandServer {
public:
	enum : unsigned int {
		CMD_USER_START	=	CMD_USER,
		CMD_CONSOLE_CMD,
	};
	constexpr static int	CONSOLE_COMMAND_MAX	=	256+4;
	thread::CriticalSection		Lock;
	thread::AtomicValue<int>	HasCommand= false;
	TArray<FString>		CommandArray;
public:
	bool	CommandExec( network::Socket& sock, network::CommandHeader& cmd, memory::MemoryBuffer& data )
	{
		switch( cmd.Command ){
		case CMD_CONSOLE_CMD: {
				if( !HasCommand.Get() ){
					Lock.Lock();
					if( data.GetDataSize() < CONSOLE_COMMAND_MAX-2 ){
						char	command_line[CONSOLE_COMMAND_MAX];
						memcpy( command_line, data.GetBuffer(), data.GetDataSize() );
						command_line[data.GetDataSize()]= '\0';
						CommandArray.Add( FString( command_line ) );
						HasCommand.Set( true );
					}
					Lock.Unlock();
				}
				return	false;
			}
			break;
		default:
			UA_LOG( "RemoteConsole: Unknown Command %d\n", cmd.Command );
			return	false;
		}
		return	true;
	}
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
}
#endif


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FRemoteConsoleServer::FRemoteConsoleServer()
{
}


FRemoteConsoleServer::~FRemoteConsoleServer()
{
	StopServer();
}


void	FRemoteConsoleServer::StartServer( const char* host, int port, int ipv )
{
#if USE_UALIB
	iServer= UA_MEMORY::New<ECommandServer>();
	const char*	hostname= host;
	if( !hostname ){
		if( ipv == 6 ){
			hostname= "::";
		}else{
			hostname= "0.0.0.0";
		}
	}
	Port= port;
	IPv= ipv;
	strcpy_s( Hostname, MAX_HOSTNAME-1, hostname );
	iThread= thread::CreateThreadFunction( [this](){
			//UA_LOG( "RemoteConsole::Start ServerTherad %d\n", Port );
			iServer->StartServer( Hostname, Port, IPv == 6 ? network::AddrInfo::IPV6 : network::AddrInfo::IPV4 );
			//UA_LOG( "RemoteConsole::Stop ServerTherad\n" );
		} );
	iThread->Start();
#endif
}


void	FRemoteConsoleServer::StopServer()
{
#if USE_UALIB
	if( iServer ){
		//UA_LOG( "RemoteConsole::Request Quit ServerTherad\n" );
		iServer->ExitServer();
		network::CommandClient	client;
		client.Connect( IPv == 6 ? "::1" : "127.0.0.1", Port );
		client.SendCloseCommand();
		memory::ZDelete( iServer );
	}
	if( iThread ){
		//UA_LOG( "RemoteConsole::Join ServerTherad\n" );
		iThread->Join();
		memory::ZDelete( iThread );
	}
#endif
}


void	FRemoteConsoleServer::Flush( UWorld* world )
{
#if USE_UALIB
	if( iServer ){
		if( iServer->HasCommand.Get() ){
			iServer->Lock.Lock();
			unsigned int	ccount= iServer->CommandArray.Num();
			for( unsigned int ci= 0 ; ci< ccount ; ci++ ){
				//UE_LOG( LogTemp, Log, TEXT("ExecCommand %s"), *iServer->CommandArray[ci] );
				GEngine->Exec( world, *iServer->CommandArray[ci] );
			}
			iServer->CommandArray.Empty();
			iServer->HasCommand.Set( false );
			iServer->Lock.Unlock();
		}
	}
#endif
}



