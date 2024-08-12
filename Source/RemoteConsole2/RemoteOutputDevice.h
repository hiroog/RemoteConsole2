// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"


class FRemoteConsoleServer3;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FRemoteOutputDevice : public FOutputDevice {
private:
	FRemoteConsoleServer3*	iRemoteServer= nullptr;
public:
	void	Serialize( const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category ) override;
	bool	CanBeUsedOnAnyThread() const override
	{
		return	true;
	}
	bool	CanBeUsedOnMultipleThreads() const override
	{
		return	true;
	}
	//-------------------------------------------------------------------------
	void	SetServer( FRemoteConsoleServer3* server );
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

