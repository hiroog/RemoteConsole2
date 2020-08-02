// RemoteConsole2 2020/08/01 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once
#include "CoreMinimal.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include "HAL/Runnable.h"
#include "Templates/Atomic.h"


class FRemoteConsoleServer2 : public FRunnable {
public:
	struct DataHeader {
		uint16_t	Magic;		// 0x7083
		uint8_t		Command;
		uint8_t		DataLength;
	};
	enum : unsigned int {
		CMD_NOP,
		CMD_EXIT,
		CMD_CONSOLE_CMD,
	};

private:

	FRunnableThread*	iThread= nullptr;
	FCriticalSection	Lock;
	TArray<FString>		CommandArray;
	int	Port= 10101;
	int	IPv= 4;
	TAtomic<bool>		bStopFlag= false;
	TAtomic<bool>		bRecvCommand= false;

private:

	bool	Init() override;
	uint32	Run() override;
	void	Stop() override;

	void	RecvAll( FSocket* socket, uint8* buffer, int32 data_size );
public:
	FRemoteConsoleServer2();
	virtual ~FRemoteConsoleServer2();
	void	StartServer( const char* host= nullptr, int port= 10101, int ipv= 4 );
	void	StopServer();
	void	Flush( UWorld* world );



};


