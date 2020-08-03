// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once
#include "CoreMinimal.h"

namespace ualib {
	namespace thread {
		class ThreadInstance;
	}
	class ECommandServer;
}

class FRemoteConsoleServer {

	ualib::thread::ThreadInstance*	iThread= nullptr;
	ualib::ECommandServer*			iServer= nullptr;
	int	Port= 10101;
	int	IPv= 4;
	constexpr static int	MAX_HOSTNAME=	128;
	char	Hostname[MAX_HOSTNAME];

public:
	FRemoteConsoleServer();
	~FRemoteConsoleServer();
	void	StartServer( const char* host= nullptr, int port= 10101, int ipv= 4 );
	void	StopServer();
	void	Flush( UWorld* world );
};


