// 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/memory/MemoryAllocator.h>
#include	<flatlib/core/thread/Sleep.h>
#include	<flatlib/core/thread/ScopedLock.h>
#include	<flatlib/core/thread/CriticalSection.h>
#include	<flatlib/core/text/FileName.h>
#include	<flatlib/core/system/Environment.h>
#include	<flatlib/core/text/LineBuffer.h>
#include	<flatlib/core/time/SystemClock.h>
#include	"NetworkClient.h"
#if FL_OS_WINDOWS
# include	<objbase.h>
#endif

using namespace flatlib;


//-----------------------------------------------------------------------------

KeyQueue::KeyQueue()
{
}

void	KeyQueue::Push( uint32_t code, bool down )
{
	thread::ScopedLock<KeyQueue>	lock( *this );
	auto	index= DataSize;
	if( index < KEYARRAY_SIZE ){
		if( down ){
			KeyArray[index]= code | (1<<31);
		}else{
			KeyArray[index]= code;
		}
		DataSize= index + 1;
	}
}

uint32_t	KeyQueue::GetDataSize() const
{
	return	DataSize;
}

uint32_t	KeyQueue::GetData( uint32_t index ) const
{
	return	KeyArray[index];
}

void	KeyQueue::Lock()
{
	KeyLock.Lock();
}

void	KeyQueue::Unlock()
{
	KeyLock.Unlock();
}

void	KeyQueue::Reset()
{
	DataSize= 0;
}


//-----------------------------------------------------------------------------

NetworkClient::NetworkClient() :
	iThread( nullptr ),
	iWindow( nullptr ),
	Port( 10101 ),
	UpdateCounter( 0 ),
	bInitialized( false ),
	bIPv6( false )
{
}

NetworkClient::~NetworkClient()
{
	Stop();
}

void	NetworkClient::Stop()
{
	bInitialized= false;
	if( iThread ){
		LoopFlag.Set( 0 );
		iThread->Join();
		FL_MEMORY::ZDelete( iThread );
	}
}

void	NetworkClient::Wait()
{
	if( iThread ){
		iThread->Join();
		FL_MEMORY::ZDelete( iThread );
	}
}

bool	NetworkClient::Connect()
{
	if( Client.Connect( Host.GetText(), Port, bIPv6 ? network::AddrInfo::IPV6 : network::AddrInfo::IPV4 ) ){
		return	true;
	}
	FL_LOG( "CONNECT ERROR\n" );
	return	false;
}


//-----------------------------------------------------------------------------

void	NetworkClient::ScanController( bool update )
{
	if( update ){
		Input->Update();
	}
	Input->UpdateDevices();
	unsigned int	controller_count= 0;
	unsigned int	device_count= Input->GetDeviceCount();
	for( unsigned int di= 0 ; di< device_count ; di++ ){
		input::EventStick	stick;
		if( Input->GetData( di, stick ) ){
			controller_count++;
		}
	}
	if( controller_count != DeviceCount.Get() ){
		DeviceCount.Set( controller_count );
		RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
	}
}

inline float	Clamp_Internal( float value )
{
	if( value > 1.0f ){
		return	1.0f;
	}
	if( value < -1.0f ){
		return	-1.0f;
	}
	return	value;
}

bool	NetworkClient::UpdateController()
{
	if( UpdateCounter++ >= 60 * 3 ){
		ScanController();
		UpdateCounter= 0;
	}
	Input->Update();
	input::EventStick	data;
	Input->GetData( 0, data );
	ControllerStatus	status;
	const float	STICK_SCALE= 32767.0f;
	status.Analog[0]= static_cast<short>( Clamp_Internal( data.LeftStick.x ) * STICK_SCALE );
	status.Analog[1]= static_cast<short>( Clamp_Internal( data.LeftStick.y ) * STICK_SCALE );
	status.Analog[2]= static_cast<short>( Clamp_Internal( data.RightStick.x ) * STICK_SCALE );
	status.Analog[3]= static_cast<short>( Clamp_Internal( data.RightStick.y ) * STICK_SCALE );
	status.Analog[4]= static_cast<short>( Clamp_Internal( data.Trigger.x ) * STICK_SCALE );
	status.Analog[5]= static_cast<short>( Clamp_Internal( data.Trigger.y ) * STICK_SCALE );
	status.Button= data.Button;
	return	Client.SendCommand( CMD_CONTROLLER_CMD, &status, sizeof(ControllerStatus) );
}


//-----------------------------------------------------------------------------

bool	NetworkClient::UpdateKeyboard()
{
	if( Queue.GetDataSize() ){
		thread::ScopedLock<KeyQueue>	lock( Queue );
		uint32_t	qcount= Queue.GetDataSize();
		for( uint32_t qi= 0 ; qi< qcount ; qi++ ){
			uint32_t	code= Queue.GetData( qi );
			uint32_t	keycode= code & 0xffff;
			if( !Client.SendCommand( CMD_KEYBOARD, nullptr, 0, (code >> 16) ? KEY_DOWN : KEY_UP, (keycode << 16) | keycode ) ){
				return	false;
			}
		}
		Queue.Reset();
	}
	return	true;
}


//-----------------------------------------------------------------------------

void	NetworkClient::Start( void* win, const char* host, unsigned int port, const char* pad_table_path, unsigned int ipv )
{
	Host.Copy( host );
	Port= port;
	bIPv6= ipv == 6;
	iWindow= win;

	text::LineBuffer	buffer( 256 );
	if( !pad_table_path ){
		*buffer.GetText()= '\0';
		if( !system::GetEnv( buffer.GetText(), buffer.GetBufferSize(), "FLATLIB5" ) ){
			buffer.Copy( "../../.." );
		}
		text::AppendPath( buffer.GetText(), buffer.GetBufferSize(), "data/pad_table.txt" );
		pad_table_path= buffer.GetText();
	}

	Input.Create( nullptr, { pad_table_path, win, true } );
	bInitialized= true;
	LoopFlag.Set( 1 );

	iThread= thread::CreateThreadFunction(
		[this](){
#if FL_OS_WINDOWS
			::CoInitialize( 0 );
#endif
			for(; LoopFlag.Get() ;){
				float	SleepTime= 0.5f;
				Status.Set( STATUS_WAITSERVER );
				RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
				for(; !Connect() && LoopFlag.Get() ;){
					thread::SleepThread( SleepTime );
					SleepTime*= 2.0f;
					if( SleepTime >= 5.0f ){
						SleepTime= 5.0f;
					}
					if( !LoopFlag.Get() ){
						break;
					}
					ScanController( true );
				}
				Status.Set( STATUS_CONNECTED );
				RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
				constexpr double	CLOCK_60FPS= 1.0f/60.0f;
				double	next_clock= time::GetPerfCounter() + CLOCK_60FPS;
				for(; LoopFlag.Get() ;){
					auto	clock= time::GetPerfCounter();
					if( clock >= next_clock ){
						if( !UpdateController() || !UpdateKeyboard() ){
							Client.Finalize();
							break;
						}
						next_clock= clock + CLOCK_60FPS;
						thread::SleepThread( 1.0f/180 );
					}
				}
			}
			bInitialized= false;
			Client.Finalize();
#if FL_OS_WINDOWS
			::CoUninitialize();
#endif
		});
	iThread->Start();
}

input::PadInput*	NetworkClient::GetPad()
{
	if( bInitialized ){
		return	&Input;
	}
	return	nullptr;
}



