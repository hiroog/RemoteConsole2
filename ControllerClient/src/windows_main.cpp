// RemoteController 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/system/CoreContext.h>
#include	<flatlib/core/ut/LinearArray.h>
#include	<flatlib/core/text/LineBuffer.h>
#include	<flatlib/core/db/DBGlobal.h>
#include	<flatlib/window/windows/WindowFrame.h>
#include	"NetworkClient.h"
#include	<stdio.h>

using namespace flatlib;


void	SplitCommandArgs( text::LineBuffer& line, ut::LinearArray<const char*>& args, const char* args_line )
{
	size_t	arg_len= strlen( args_line );
	line.SetBufferSize( static_cast<unsigned int>( arg_len + 1 ) );
	line.Copy( args_line );
	char*	ptr= line.GetText();
	for(; *ptr ;){
		for(; *ptr == ' ' || *ptr == '\t' ; ptr++ );
		args.PushBack( ptr );
		for(; *ptr && (*ptr != ' ' && *ptr != '\t') ; ptr++ );
		if( *ptr ){
			*ptr++= '\0';
		}
	}
}


void	ParseCommandLineArgs( system::CoreContext* context, db::DBInterface* config, text::LineBuffer& line_buffer, const char* args_line )
{
	config->Set( "Port", 10101 );
	config->Set( "Host", "127.0.0.1" );	// "::1"
	config->Set( "IPV", 4 );
	config->Set( "PadTable", (const char*)nullptr );
	ut::LinearArray<const char*>	args;
	SplitCommandArgs( line_buffer, args, args_line );
	unsigned int	acount= args.GetDataSize();
	for( unsigned int ai= 0 ; ai< acount ; ai++ ){
		const char*	arg= args[ai];
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
			}
		}
		FL_LOG( "%d: %s\n", ai, arg );
	}
}


int WINAPI	WinMain( HINSTANCE hinstance, HINSTANCE, LPSTR args_line, int )
{
	auto*	context= system::CreateContext();
	network::InitNetwork();
	{
		auto*	config= context->RDBGlobal().Create( "Config", "Dictionary" );
		text::LineBuffer	line_buffer;
		ParseCommandLineArgs( context, config, line_buffer, args_line );

		int			Port= config->Get<int>( "Port" );
		int			ipv=  config->Get<int>( "IPV" );
		const char*	Host= config->Get<const char*>( "Host" );
		const char*	PadTableFile= config->Get<const char*>( "PadTable" );

		NetworkClient	client;
		auto*	window= window::CreateSimpleWindow(
			[&client,Port,Host]( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )->LRESULT
			{
				switch( msg ){
				case WM_DESTROY:
					PostQuitMessage( 0 );
					break;
				case WM_ACTIVATE:
					if( auto* pad= client.GetPad() ){
						switch( wparam & 0xffff ){
						case 0:
							//(*pad)->Acquire( false );
							break;
						case 1:
						case 2:
							(*pad)->Acquire( true );
							break;
						}
					}
					return	0;
				case WM_KEYDOWN:
					if( wparam == VK_HOME ){
						client.ToggleRecording();
					}else{
						client.PushKey( static_cast<uint32_t>( wparam ), true );
					}
					return	0;
				case WM_KEYUP:
					if( wparam != VK_HOME ){
						client.PushKey( static_cast<uint32_t>( wparam ), false );
					}
					return	0;
				case WM_PAINT: {
						PAINTSTRUCT	ps;
						HDC	hdc= BeginPaint( hwnd, &ps );
						FillRect( hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1) );
						text::StaticLineBuffer<256>	buffer;
						sprintf_s( buffer.GetText(), 256, "Host:%s  Port:%d", Host, Port );
						TextOutA( hdc, 5, 5, buffer.GetText(), buffer.GetDataSize() );
						sprintf_s( buffer.GetText(), 256, "Controller=%d %s", client.GetDeviceCount(), client.IsRecording() ? "    ● REC" : "" );
						TextOutA( hdc, 5, 5+24, buffer.GetText(), buffer.GetDataSize() );
						const char*	status_string= "";
						switch( client.GetStatus() ){
						case NetworkClient::STATUS_WAITSERVER:
							status_string= "Waiting for server";
							break;
						case NetworkClient::STATUS_CONNECTED:
							status_string= "Connected";
							break;
						}
						TextOutA( hdc, 5, 5+48, status_string, static_cast<int>(strlen(status_string)) );
						EndPaint( hwnd, &ps );
						return	0;
					}
					break;
				}
				return	DefWindowProc( hwnd, msg, wparam, lparam );
			});
		window->OpenSimpleWindow(
					"RemoteControllerClient",
					"Remote Controller v1.40",
					WS_OVERLAPPEDWINDOW|WS_VISIBLE,
					WS_EX_APPWINDOW,
					320,80
				);

		client.Start( (void*)window->GetWindow(), Host, Port, PadTableFile, ipv );

		MSG	msg;
		for(; GetMessage( &msg, nullptr, 0, 0 ) > 0 ;){
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		client.Stop();
		FL_MEMORY::ZDelete( window );
	}
	ZRelease( context );
	return	0;
}

