// 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/memory/MemoryAllocator.h>
#include	<flatlib/core/thread/Sleep.h>
#include	<flatlib/core/thread/ScopedLock.h>
#include	<flatlib/core/thread/CriticalSection.h>
#include	<flatlib/core/text/FileName.h>
#include	<flatlib/core/text/TextPool.h>
#include	<flatlib/core/system/Environment.h>
#include	<flatlib/core/text/LineBuffer.h>
#include	<flatlib/core/time/SystemClock.h>
#include	<flatlib/core/file/FileSystem.h>
#include	<flatlib/core/system/CoreContext.h>
#include	"NetworkClient.h"
#if FL_OS_WINDOWS
# include	<objbase.h>
#endif

using namespace flatlib;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QueueBase::QueueBase()
{
}

uint32_t	QueueBase::GetDataSize() const
{
	return	DataSize;
}

void	QueueBase::Lock()
{
	QueueLock.Lock();
}

void	QueueBase::Unlock()
{
	QueueLock.Unlock();
}

void	QueueBase::Reset()
{
	DataSize= 0;
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

KeyQueue::KeyQueue()
{
}

void	KeyQueue::Push( uint32_t key_code, bool down )
{
	thread::ScopedLock<KeyQueue>	lock( *this );
	auto	index= DataSize;
	if( index < KEYARRAY_SIZE ){
		if( down ){
			KeyArray[index]= key_code | (1<<16);
		}else{
			KeyArray[index]= key_code;
		}
		DataSize= index + 1;
	}
}

uint32_t	KeyQueue::GetData( uint32_t index ) const
{
	return	KeyArray[index];
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

EventQueue::EventQueue()
{
	memory::MemClear( PrevStatus );
	EventSignal.Initialize( true, false );
}

void	EventQueue::Push( const ControllerStatus& status, double time )
{
	thread::ScopedLock<EventQueue>	lock( *this );
	auto	index= DataSize;
	if( index >= QUEUE_SIZE ){
		return;
	}
	if( memcmp( &status, &PrevStatus, sizeof(ControllerStatus) ) != 0 ){
		Event	event;
		event.Status= status;
		event.EventTime= time;
		event.EventType= EVENT_CONTROLLER;
		EventArray[index]= std::move(event);
		DataSize= index + 1;
		PrevStatus= status;
		if( DataSize >= TRIGGER_SIZE ){
			EventSignal.Set();
		}
	}
}

void	EventQueue::PushKey( uint32_t key_code, double time )
{
	thread::ScopedLock<EventQueue>	lock( *this );
	auto	index= DataSize;
	if( index >= QUEUE_SIZE ){
		return;
	}
	Event	event;
	event.EventTime= time;
	event.KeyCode= key_code;
	event.EventType= EVENT_KEY;
	EventArray[index]= std::move(event);
	DataSize= index + 1;
	if( DataSize >= TRIGGER_SIZE ){
		EventSignal.Set();
	}
}

uint32_t	EventQueue::CopyData( ut::StaticArray<Event,QUEUE_SIZE>& dest )
{
	thread::ScopedLock<EventQueue>	lock( *this );
	uint32_t	data_size= DataSize;
	for( uint32_t di= 0 ; di< data_size ; di++ ){
		dest[di]= EventArray[di];
	}
	Reset();
	EventSignal.Reset();
	return	data_size;
}

bool	EventQueue::Wait( uint32_t ms )
{
	return	EventSignal.Wait( ms );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

NetworkClient::NetworkClient()
{
}

NetworkClient::~NetworkClient()
{
	Stop();
}

void	NetworkClient::Stop()
{
	bInitialized.store( false );
	bLoopFlag.store( false );
	if( iThread ){
		iThread->Join();
		FL_MEMORY::ZDelete( iThread );
	}
	if( iRecordingThread ){
		iRecordingThread->Join();
		FL_MEMORY::ZDelete( iRecordingThread );
	}
}

void	NetworkClient::Wait()
{
	if( iThread ){
		iThread->Join();
		FL_MEMORY::ZDelete( iThread );
	}
	if( iRecordingThread ){
		iRecordingThread->Join();
		FL_MEMORY::ZDelete( iRecordingThread );
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
	input::EventStick	stick;
	if( Input->GetData( 0, stick ) ){
		controller_count++;
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

bool	NetworkClient::UpdateController( double clock )
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
	bool	result= Client.SendCommand( CMD_CONTROLLER_CMD, &status, sizeof(ControllerStatus) );
	if( result ){
		if( bRecordingFlag.load() ){
			RecQueue.Push( status, clock );
		}
	}
	return	result;
}


//-----------------------------------------------------------------------------

bool	NetworkClient::UpdateKeyboard( double clock )
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
			if( bRecordingFlag.load() ){
				RecQueue.PushKey( code, clock );
			}
		}
		Queue.Reset();
	}
	return	true;
}


//-----------------------------------------------------------------------------

void	NetworkClient::FlushEventQueue( file::FileHandle& rec_file, text::TextPool& pool, ut::StaticArray<Event,EventQueue::QUEUE_SIZE>& event_array, uint32_t data_size )
{
	const char*		rec_file_name= "key_log.txt";
	if( !data_size ){
		return;
	}
	if( !rec_file.IsOpen() ){
		rec_file.Create( rec_file_name );
	}
	if( !rec_file.IsOpen() ){
		return;
	}
	pool.Truncate( 0 );
	for( uint32_t di= 0 ; di< data_size ; di++ ){
		const auto&	event= event_array[di];
		switch( event.EventType ){
		default:
		case EventQueue::EVENT_CONTROLLER:
			pool.AddFormat( "C %.4f %08x %d %d %d %d %d %d\n",
					event.EventTime,
					event.Status.Button,
					event.Status.Analog[0],
					event.Status.Analog[1],
					event.Status.Analog[2],
					event.Status.Analog[3],
					event.Status.Analog[4],
					event.Status.Analog[5]
				);
			break;
		case EventQueue::EVENT_KEY:
			pool.AddFormat( "K %.4f %05x\n",
					event.EventTime,
					event.KeyCode
				);
			break;
		}
	}
	rec_file.Write( pool.GetBuffer(), pool.GetDataSize() );
	pool.Truncate( 0 );
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
	bInitialized.store( true );
	bLoopFlag.store( true );

	iThread= thread::CreateThreadFunction(
		[this](){
#if FL_OS_WINDOWS
			::CoInitialize( 0 );
#endif
			for(; bLoopFlag.load() ;){
				float	SleepTime= 0.5f;
				Status.Set( STATUS_WAITSERVER );
				RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
				for(; !Connect() && bLoopFlag.load() ;){
					thread::SleepThread( SleepTime );
					SleepTime*= 2.0f;
					if( SleepTime >= 5.0f ){
						SleepTime= 5.0f;
					}
					if( !bLoopFlag.load() ){
						break;
					}
					ScanController( true );
				}
				Status.Set( STATUS_CONNECTED );
				RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
				constexpr double	CLOCK_60FPS= 1.0f/60.0f;
				double	base_clock= time::GetPerfCounter();
				double	next_clock= base_clock + CLOCK_60FPS;
				for(; bLoopFlag.load() ;){
					auto	clock= time::GetPerfCounter();
					if( clock >= next_clock ){
						if( !UpdateController( clock-base_clock ) || !UpdateKeyboard( clock-base_clock ) ){
							Client.Finalize();
							break;
						}
						next_clock= clock + CLOCK_60FPS;
					}else{
						float	dclock= static_cast<float>( next_clock - clock );
						if( dclock >= 1.0f/90.0f ){
							thread::SleepThread( dclock );
						}else{
							thread::SleepThread( 0 );
						}
					}
				}
			}
			bInitialized.store( false );
			Client.Finalize();
#if FL_OS_WINDOWS
			::CoUninitialize();
#endif
		});
	iRecordingThread= thread::CreateThreadFunction(
		[this](){
			ut::StaticArray<Event,EventQueue::QUEUE_SIZE>	event_array;
			file::FileHandle	rec_file( system::RCore().RFileSystem() );
			text::TextPool		pool;
			bool	rec_flag= false;
			for(; bLoopFlag.load() ;){
				if( RecQueue.Wait( 1000 ) ){
					FlushEventQueue( rec_file, pool, event_array, RecQueue.CopyData( event_array ) );
				}
				if( bRecordingFlag.load() ){
					rec_flag= true;
				}else if( rec_flag ){
					FlushEventQueue( rec_file, pool, event_array, RecQueue.CopyData( event_array ) );
					rec_flag= false;
				}
			}
			FlushEventQueue( rec_file, pool, event_array, RecQueue.CopyData( event_array ) );
			if( rec_file.IsOpen() ){
				rec_file.Close();
			}
		});
	iThread->Start();
	iRecordingThread->Start();
}

input::PadInput*	NetworkClient::GetPad()
{
	if( bInitialized.load() ){
		return	&Input;
	}
	return	nullptr;
}

void	NetworkClient::ToggleRecording()
{
	if( bInitialized.load() ){
		if( bRecordingFlag.load() ){
			bRecordingFlag.store( false );
		}else{
			bRecordingFlag.store( true );
		}
		RedrawWindow( (HWND)iWindow, nullptr, nullptr, RDW_INVALIDATE|RDW_INTERNALPAINT );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

