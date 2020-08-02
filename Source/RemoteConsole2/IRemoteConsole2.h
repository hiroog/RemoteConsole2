// RemoteConsole2 2020/07/25
// vim:ts=4 sw=4 noet:

#pragma once

#include "Modules/ModuleManager.h"
#include "IInputDeviceModule.h"

class IRemoteConsole2 : public IInputDeviceModule {
public:

	static inline IRemoteConsole2&	Get()
	{
		return FModuleManager::LoadModuleChecked<IRemoteConsole2>( "RemoteConsole2" );
	}
	static inline bool	IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "RemoteConsole2" );
	}

};

