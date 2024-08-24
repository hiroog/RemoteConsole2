// RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
// vim:ts=4 sw=4 noet:

#include "GameAccessAPI.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "RemoteConsole2.h"
#include "RemoteConsoleServer3.h"
#include "Components/Widget.h"
#include "Blueprint/GameViewportSubsystem.h"

//#include "Delegates/DelegateCombinations.h"
//#include "Delegates/IDelegateInterface.h"
//#include "Engine/GameViewportClient.h"

#define	RC2_USE_STARTSWITH	1

//-----------------------------------------------------------------------------

void	FGameAccessAPI::Init()
{
	if( GWorld ){
		auto*	subsystem= UGameViewportSubsystem::Get( GWorld );
		if( subsystem ){
			WidgetAddedHandle= subsystem->OnWidgetAdded.AddRaw( this, &FGameAccessAPI::CallbackAddWidget );
			WidgetRemovedHandle= subsystem->OnWidgetRemoved.AddRaw( this, &FGameAccessAPI::CallbackRemoveWidget );
			UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:GameAPI:Initialized") );
		}
	}
}


void	FGameAccessAPI::Quit()
{
	if( GWorld ){
		if( WidgetAddedHandle.IsValid() ){
			auto*	subsystem= UGameViewportSubsystem::Get( GWorld );
			if( subsystem ){
				subsystem->OnWidgetAdded.Remove( WidgetAddedHandle );
				subsystem->OnWidgetRemoved.Remove( WidgetRemovedHandle );
				UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:GameAPI:Uninitialized") );
			}
		}
	}
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::CallbackAddWidget( UWidget* widget, ULocalPlayer* player )
{
	WidgetArray.Add( MakeWeakObjectPtr( widget ) );
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:GameAPI:WidgetAdded %s (%d)"), *widget->GetName(), WidgetArray.Num() );
}


void	FGameAccessAPI::CallbackRemoveWidget( UWidget* widget )
{
	unsigned int	wcount= WidgetArray.Num();
	for( unsigned int wi= 0 ; wi< wcount ; wi++ ){
		if( WidgetArray[wi] == widget ){
			WidgetArray.RemoveAtSwap( wi, 1, false );
			break;
		}
	}
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:GameAPI:WidgetRemoved %s (%d)"), *widget->GetName(), WidgetArray.Num() );
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::DumpWidgetInternal( FString prefix, const UWidget* widget ) const
{
	auto*	user_widget= Cast<UUserWidget>( widget );
	if( user_widget ){
		OutputResult( TEXT("WidgetUser"), *widget->GetName() );
		FString	fullname;
		if( prefix.IsEmpty() ){
			fullname= widget->GetName();
		}else{
			fullname= prefix + TEXT(",") + widget->GetName();
		}
		TArray<UWidget*>	widget_array;
		user_widget->WidgetTree->GetAllWidgets( widget_array );
		for( const auto* child_widget : widget_array ){
			DumpWidgetInternal( fullname, child_widget );
		}
	}else{
		FString	fullname= prefix + TEXT(",") + widget->GetName();
		OutputResult( TEXT("Widget"), *FString::Printf( TEXT("%s (%s)"), *fullname, *widget->GetClass()->GetName() ) );
	}
}


void	FGameAccessAPI::DumpWidget() const
{
	if( GWorld ){
		for( const auto& widget : WidgetArray ){
			OutputResult( TEXT("WidgetRoot"), *widget->GetName() );
			if( widget.IsValid() ){
				DumpWidgetInternal( "", widget.Get() );
			}
		}
	}
}


//-----------------------------------------------------------------------------

inline bool	RC2_Widget_Compare_Internal( const FString& name, const TCHAR* widget_name, bool startswith )
{
	if( startswith ){
		return	name.StartsWith( widget_name );
	}
	return	name.Equals( widget_name );
}


UWidget*	FGameAccessAPI::FindWidgetInternal( UWidget* root_widget, const TCHAR* widget_name, bool startswith ) const
{
	if( !root_widget ){
		return	FindWidget( nullptr, widget_name, startswith );
	}
	auto*	user_widget= Cast<UUserWidget>( root_widget );
	if( user_widget ){
		TArray<UWidget*>	widget_array;
		user_widget->WidgetTree->GetAllWidgets( widget_array );
		for( UWidget* child_widget : widget_array ){
			if( RC2_Widget_Compare_Internal( child_widget->GetName(), widget_name, startswith ) ){
				return	child_widget;
			}
		}
		for( UWidget* child_widget : widget_array ){
			UWidget*	result= FindWidgetInternal( child_widget, widget_name, startswith );
			if( result ){
				return	result;
			}
		}
	}
	if( RC2_Widget_Compare_Internal( root_widget->GetName(), widget_name, startswith ) ){
		return	root_widget;
	}
	return	nullptr;
}


UWidget*	FGameAccessAPI::FindWidget( UWidget* root_widget, const TCHAR* widget_name, bool startswith ) const
{
	if( GWorld ){
		if( root_widget ){
			return	FindWidgetInternal( root_widget, widget_name, startswith );
		}
		for( const auto& widget : WidgetArray ){
			if( widget.IsValid() ){
				UWidget* result= FindWidgetInternal( widget.Get(), widget_name, startswith );
				if( result ){
					return	result;
				}
			}
		}
	}
	return	nullptr;
}


UWidget*	FGameAccessAPI::FindWidget( const TCHAR* widget_name ) const
{
	check( widget_name != nullptr );
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:FindWidget %s"), widget_name );
	if( !GWorld ){
		return	nullptr;
	}
	FString		name= FString( widget_name );
	UWidget*	root_widget= nullptr;
	for(;;){
		int32 index= 0;
		if( name.FindChar( ',', index ) ){
			FString	parent;
			FString	child;
			if( name.Split( ",", &parent, &child ) ){
				UWidget*	parent_widget= FindWidget( root_widget, *parent, false );
				if( parent_widget ){
					root_widget= parent_widget;
					name= child;
					continue;
				}
#if RC2_USE_STARTSWITH
				parent_widget= FindWidget( root_widget, *parent, true );
				if( parent_widget ){
					root_widget= parent_widget;
					name= child;
					continue;
				}
#endif
			}
		}
		break;
	}
	return	FindWidget( root_widget, *name, false );
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::WidgetButton( const TCHAR* widget_name, uint32_t action )
{
	check( widget_name != nullptr );
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:WidgetButton %s"), widget_name );
	if( GWorld ){
		auto*	widget= FindWidget( widget_name );
		if( widget ){
			auto*	button= Cast<UButton>( widget );
			if( button ){
				switch( action ){
				default:
				case FRemoteConsoleServer3::UI_BUTTON_CLICKED:
					button->OnClicked.Broadcast();
					break;
				case FRemoteConsoleServer3::UI_BUTTON_PRESSED:
					button->OnPressed.Broadcast();
					break;
				case FRemoteConsoleServer3::UI_BUTTON_RELEASED:
					button->OnReleased.Broadcast();
					break;
				case FRemoteConsoleServer3::UI_BUTTON_HOVERED:
					button->OnHovered.Broadcast();
					break;
				case FRemoteConsoleServer3::UI_BUTTON_UNHOVERED:
					button->OnUnhovered.Broadcast();
					break;
				}
				OutputResult( TEXT("ButtonResult"), TEXT("1") );
				return;
			}
			UE_LOG(LogRemoteConsole2,Error,TEXT("RemoteConsole2:WidgetButton: %s Is Not Button"), widget_name );
		}else{
			UE_LOG(LogRemoteConsole2,Warning,TEXT("RemoteConsole2:WidgetButton: %s Not Found"), widget_name );
		}
	}
	OutputResult( TEXT("ButtonResult"), TEXT("0") );
}


void	FGameAccessAPI::SetFocus( const TCHAR* widget_name, uint32_t mode )
{
	check( widget_name != nullptr );
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:SetFocus %s"), widget_name );
	bool	result= false;
	if( GWorld ){
		if( mode == FRemoteConsoleServer3::UI_FOCUS_GAMEONLY ){
			auto*	controller= UGameplayStatics::GetPlayerController( GWorld, 0 );
			if( controller ){
				UWidgetBlueprintLibrary::SetInputMode_GameOnly( controller );
				result= true;
			}
		}else{
			auto*	controller= UGameplayStatics::GetPlayerController( GWorld, 0 );
			if( controller ){
				UWidget*	widget= nullptr;
				if( *widget_name ){
					widget= FindWidget( widget_name );
				}
				switch( mode ){
				default:
					break;
				case FRemoteConsoleServer3::UI_FOCUS_UIONLY:
					UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx( controller, widget );
					result= true;
					break;
				case FRemoteConsoleServer3::UI_FOCUS_GAMEANDUI:
					UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx( controller, widget );
					result= true;
					break;
				case FRemoteConsoleServer3::UI_FOCUS_WINDOW:
					SetWindowFocus( 0 );
					result= true;
					break;
				case FRemoteConsoleServer3::UI_FOCUS_BTOF:
					SetWindowFocus( 1 );
					result= true;
					break;
				case FRemoteConsoleServer3::UI_FOCUS_BTOF2:
					SetWindowFocus( 2 );
					result= true;
					break;
				}
			}
		}
	}
	OutputResult( TEXT("FocusResult"), result ? TEXT("1"):TEXT("0") );
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::SetWindowFocus( uint32_t mode )
{
	UE_LOG(LogRemoteConsole2,Verbose,TEXT("RemoteConsole2:SetWindowFocus %d"), mode );
	bool	result= false;
	if( GEngine && GEngine->GameViewport ){
		TSharedPtr<SWindow> Window= GEngine->GameViewport->GetWindow();
		if( Window ){
			TSharedPtr<FGenericWindow>	native= Window->GetNativeWindow();
			switch( mode ){
			default:
			case 0:
				native->SetWindowFocus();
				break;
			case 1:
				native->BringToFront( true );
				break;
			case 2:
				native->HACK_ForceToFront();
				break;
			case 3:
				native->Enable( true );
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::GetCurrentLevelName()
{
	if( GWorld ){
		FString	level_name= UGameplayStatics::GetCurrentLevelName( GWorld, true );
		OutputResult( TEXT("Level"), *level_name );
		SendResult( *level_name );
	}
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::GetConsoleVar( const TCHAR* var_name )
{
	auto*	cvar= IConsoleManager::Get().FindConsoleVariable( var_name );
	if( cvar ){
		OutputResult( var_name, *cvar->GetString() );
		SendResult( *cvar->GetString() );
	}
}


//-----------------------------------------------------------------------------

void	FGameAccessAPI::SetResultKey( uint32_t result_key, FRemoteConsoleServer3* server )
{
	ResultKey= result_key;
	iServer= server;
}


void	FGameAccessAPI::SendResult( const TCHAR* value ) const
{
	if( iServer ){
		iServer->PushTextResult( value, 0, ResultKey );
	}
}


void	FGameAccessAPI::OutputResult( const TCHAR* name, const TCHAR* value ) const
{
	UE_LOG(LogRemoteConsole2,Log,TEXT("RC2:Result(%d):%s=%s"), ResultKey, name, value );
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
