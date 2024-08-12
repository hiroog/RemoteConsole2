// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Components/Widget.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class FRemoteConsoleServer3;

class FGameAccessAPI {
private:
	FDelegateHandle	WidgetAddedHandle;
	FDelegateHandle	WidgetRemovedHandle;
	TArray<TWeakObjectPtr<UWidget>>	WidgetArray;
	FRemoteConsoleServer3*	iServer= nullptr;
	uint32_t	ResultKey= 0;
private:
	void	CallbackAddWidget( UWidget* widget, ULocalPlayer* player );
	void	CallbackRemoveWidget( UWidget* widget );
	void	DumpWidgetInternal( FString prefix, const UWidget* widget ) const;
	UWidget*	FindWidgetInternal( UWidget* root_widget, const TCHAR* widget_name, bool startswith ) const;
	UWidget*	FindWidget( UWidget* root_widget, const TCHAR* widget_name, bool startswith ) const;
public:
	void	Init();
	void	Quit();
	void	SetResultKey( uint32_t result_key, FRemoteConsoleServer3* server );
	void	OutputResult( const TCHAR* name, const TCHAR* value ) const;
	void	SendResult( const TCHAR* value ) const;
	//-------------------------------------------------------------------------
	void		DumpWidget() const;
	UWidget*	FindWidget( const TCHAR* widget_name ) const;
	//!  @param[in]	action	0=clicked, 1=pressed, 2=released, 3=hovered, 4=unhovered
	void		WidgetButton( const TCHAR* widget_name, uint32_t action );
	//!  @param[in]	mode	0=game-only, 1=ui-only, 2=game and ui
	void		SetFocus( const TCHAR* widget_name, uint32_t mode );
	//-------------------------------------------------------------------------
	void		GetCurrentLevelName();
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

