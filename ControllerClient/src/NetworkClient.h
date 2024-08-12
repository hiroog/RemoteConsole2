// 2021 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include	<flatlib/core/thread/ThreadInstance.h>
#include	<flatlib/core/thread/AtomicValue.h>
#include	<flatlib/input/pad/PadInput.h>
#include	<flatlib/core/network/CommandServer.h>
#include	<flatlib/core/text/ConstString.h>
#include	<flatlib/core/ut/StaticArray.h>


//-----------------------------------------------------------------------------

class KeyQueue {
public:
	enum : unsigned int {
		KEYARRAY_SIZE	=	32,
	};
	flatlib::ut::StaticArray<uint32_t,KEYARRAY_SIZE>	KeyArray;
	flatlib::thread::CriticalSection	KeyLock;
	uint32_t	DataSize= 0;
public:
	KeyQueue();
	void		Push( uint32_t code, bool down );
	uint32_t	GetDataSize() const;
	uint32_t	GetData( uint32_t index ) const;
	void		Reset();
	void		Lock();
	void		Unlock();
};


//-----------------------------------------------------------------------------

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
	struct ControllerStatus {
		short	Analog[6];
		unsigned int	Button;
	};
private:
	KeyQueue	Queue;
	flatlib::thread::ThreadInstance*	iThread;
	void*			iWindow;
	flatlib::input::PadInput		Input;
	flatlib::network::CommandClient	Client;
	flatlib::thread::AtomicValue<int>	LoopFlag;
	flatlib::thread::AtomicValue<unsigned int>	Status;
	flatlib::thread::AtomicValue<unsigned int>	DeviceCount;
	flatlib::text::ConstString		Host;
	unsigned int	Port;
	unsigned int	UpdateCounter;
	bool	bInitialized;
	bool	bIPv6;

	bool	Connect();
	void	ScanController( bool update= false );
	bool	UpdateController();
	bool	UpdateKeyboard();
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
};


