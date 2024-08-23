// 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/thread/ThreadInstance.h>
#include	<flatlib/core/thread/AtomicValue.h>
#include	<flatlib/core/thread/Signal.h>
#include	<flatlib/input/pad/PadInput.h>
#include	<flatlib/core/network/CommandServer.h>
#include	<flatlib/core/text/ConstString.h>
#include	<flatlib/core/ut/StaticArray.h>
#include	<atomic>


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class QueueBase {
protected:
	flatlib::thread::CriticalSection	QueueLock;
	uint32_t	DataSize= 0;
public:
	QueueBase();
	uint32_t	GetDataSize() const;
	void		Reset();
	void		Lock();
	void		Unlock();
};


//-----------------------------------------------------------------------------

class KeyQueue : public QueueBase {
public:
	enum : unsigned int {
		KEYARRAY_SIZE	=	32,
	};
	flatlib::ut::StaticArray<uint32_t,KEYARRAY_SIZE>	KeyArray;
public:
	KeyQueue();
	void		Push( uint32_t key_code, bool down );
	uint32_t	GetData( uint32_t index ) const;
};


//-----------------------------------------------------------------------------

struct ControllerStatus {
	short	Analog[6];
	unsigned int	Button;
};

struct Event {
	ControllerStatus	Status;	// (4)
	double		EventTime;		// (2)
	uint32_t	KeyCode;		// (1)
	uint32_t	EventType;		// (1)
};

class EventQueue : public QueueBase {
public:
	enum : unsigned int {
		EVENT_CONTROLLER	=	0,
		EVENT_KEY,
	};
	enum : unsigned int {
		QUEUE_SIZE		=	512,
		TRIGGER_SIZE	=	(QUEUE_SIZE*2/3),
	};
	flatlib::ut::StaticArray<Event,QUEUE_SIZE>	EventArray;
	flatlib::thread::Signal		EventSignal;
	ControllerStatus	PrevStatus;
public:
	EventQueue();
	void	Push( const ControllerStatus& status, double time );
	void	PushKey( uint32_t key_code, double time );
	uint32_t	CopyData( flatlib::ut::StaticArray<Event,QUEUE_SIZE>& dest );
	bool	Wait( uint32_t ms );
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace flatlib {
	namespace text {
		class TextPool;
	}
	namespace file {
		class FileHandle;
	}
}

class NetworkClient {
public:
	enum : unsigned int {
		STATUS_WAITSERVER,
		STATUS_CONNECTED,
	};
private:
	enum : unsigned int {
		CMD_USER_START	=	flatlib::network::CommandServer::CMD_USER,
		CMD_CONTROLLER_CMD,
		CMD_KEYBOARD,
	};
	enum : uint32_t {
		KEY_UP,
		KEY_DOWN,
		KEY_CHAR,
	};
private:
	KeyQueue	Queue;
	EventQueue	RecQueue;
	flatlib::thread::ThreadInstance*	iThread= nullptr;
	flatlib::thread::ThreadInstance*	iRecordingThread= nullptr;
	void*			iWindow= nullptr;
	flatlib::input::PadInput		Input;
	flatlib::network::CommandClient	Client;
	flatlib::thread::AtomicValue<unsigned int>	Status;
	flatlib::thread::AtomicValue<unsigned int>	DeviceCount;
	flatlib::text::ConstString		Host;
	unsigned int	Port= 10101;
	unsigned int	UpdateCounter= 0;
	std::atomic<bool>	bLoopFlag= false;
	std::atomic<bool>	bRecordingFlag= false;
	std::atomic<bool>	bInitialized= false;
	bool	bIPv6= false;

	bool	Connect();
	void	ScanController( bool update= false );
	bool	UpdateController( double clock );
	bool	UpdateKeyboard( double clock );
	void	FlushEventQueue( flatlib::file::FileHandle& rec_file, flatlib::text::TextPool& pool, flatlib::ut::StaticArray<Event,EventQueue::QUEUE_SIZE>& event_array, uint32_t data_size );
	void	Redraw();
public:
	NetworkClient();
	~NetworkClient();
	void	Start( void* win, const char* host, unsigned int port, const char* pad_table_file, unsigned int ipv );
	void	Stop();
	void	Wait();
	flatlib::input::PadInput*	GetPad();
	unsigned int	GetStatus() const
	{
		return	Status.Get();
	}
	unsigned int	GetDeviceCount() const
	{
		return	DeviceCount.Get();
	}
	void	PushKey( uint32_t code, bool down )
	{
		Queue.Push( code, down );
	}
	bool	IsRecording() const
	{
		return	bRecordingFlag.load();
	}
	void	ToggleRecording();
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
