// RemoteController 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/system/CoreContext.h>
#include	<flatlib/core/db/DBGLobal.h>
#include	"NetworkClient.h"
#include	<stdio.h>


using namespace flatlib;

static void	usage()
{
	FL_PRINT( "usage: remote_controller_client [<options>]\n" );
	FL_PRINT( " -p<Port>       Port\n" );
	FL_PRINT( " -h<Host>       Host\n" );
	FL_PRINT( " -t<TableFile>  pad_table.txt\n" );
	FL_PRINT( " -c<ConfigFile> config.txt\n" );
	FL_PRINT( " -6             ipv6\n" );
}

static bool	ParseCommandLineArgs( system::CoreContext* context, db::DBInterface* config, int argc, const char*const* argv )
{
	config->Set( "Port", 10101 );
	config->Set( "Host", "127.0.0.1" );	// "::1"
	config->Set( "IPV", 4 );
	config->Set( "PadTable", (const char*)nullptr );

	bool	bRun= true;
	for( int ai= 1 ; ai< argc ; ai++ ){
		const char*	arg= argv[ai];
		if( *arg == '-' ){
			switch( arg[1] ){
			case 'p':	// -p8080
				config->Set( "Port", atoi( arg+2 ) );
				break;
			case 'h':	// -p192.168.2.199
				config->Set( "Host", arg+2 );
				break;
			case 't':	// -tpad_talbe.txt
				config->Set( "PadTable", arg+2 );
				break;
			case '6':	// -6
				config->Set( "IPV", 6 );
				break;
			case '4':	// -4
				config->Set( "IPV", 4 );
				break;
			case 'c':	// -cconfig.txt
				if( arg[2] ){
					config= context->RDBGlobal().Load( "Config", arg+2 );
				}
				break;
			default:
				bRun= false;
				break;
			}
		}
		FL_LOG( "%d: %s\n", ai, arg );
	}
	return	bRun;
}

int main( int argc, char** argv )
{
	auto*	context= system::CreateContext();
	context->RConsoleLog().SetConsoleOutputMode( true );
	{
		auto*	config= context->RDBGlobal().Create( "Config", "Dictionary" );
		bool	bRun= ParseCommandLineArgs( context, config, argc, argv );

		int			Port= config->Get<int>( "Port" );
		int			ipv=  config->Get<int>( "IPV" );
		const char*	Host= config->Get<const char*>( "Host" );
		const char*	PadTableFile= config->Get<const char*>( "PadTable" );

		if( bRun ){
			network::InitNetwork();
			NetworkClient	client;
			client.Start( nullptr, Host, Port, PadTableFile, ipv );
			client.Wait();
			client.Stop();
		}
	}
	ZRelease( context );
	return	0;
}

