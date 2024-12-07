# RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import os
import sys
import RemoteConsole2API


options= RemoteConsole2API.Options()

with RemoteConsole2API.ConsoleAPI( options ) as api:
    api.connect()
    api.start_logger()
    api.sleep( 1.0 )

    api.send_console_command( 'open ThirdPersonMap' )
    api.wait_log( r'LevelBP\s+BeginPlay' )

    level_name= api.get_level_name()
    resolution= api.get_console_var( 'r.SetRes' )
    print( '***** %s *****' % level_name.get() )
    print( '***** %s *****' % resolution.get() )



    api.sleep( 0.5 )
    api.set_focus( None, True )
    api.sleep( 0.5 )

    controller= RemoteConsole2API.Controller()
    api.send_controller( controller.reset().lx( 1.0 ) )
    api.sleep( 1 )
    api.send_controller( controller.reset() )
    api.sleep( 1 )


    api.send_key( 'W', 1 )
    api.sleep( 2 )
    api.send_key( 'W', 0 )
    api.sleep( 1 )

    for a in range(10):
        api.send_mouse_move( 80 + a, 0 )
        api.sleep( 2.0/60.0 )

    api.send_ui_button( 'Button_01', RemoteConsole2API.Event.UI_BUTTON_CLICKED )
    api.wait_log( r'Button 01 Clicked' )

    api.sleep( 1 )

    api.send_ui_button( 'Button_01', RemoteConsole2API.Event.UI_BUTTON_CLICKED )
    api.wait_log( r'Button 01 Clicked' )

    api.sleep( 1 )

    api.send_ui_button( 'Button_02', RemoteConsole2API.Event.UI_BUTTON_CLICKED )
    api.wait_log( r'Button 02 Clicked' )

    api.sleep( 1 )

    api.send_ui_button( 'BPUserWidgetTest,UserButton001', 0 )
    api.wait_log( r'USER-BUTTON\s+Clicked' )


    api.sleep( 4 )

    api.send_console_command( 'QUIT_EDITOR' )



