# RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import sys
import RemoteConsole2API
import time


options= RemoteConsole2API.Options()
controller= RemoteConsole2API.Controller()

def wait_level( api, level_name ):
    while True:
        ret= api.get_level_name()
        name= ret.get()
        print( '[%s]' % name )
        if name != level_name:
            api.sleep( 1 )
            continue
        break


with RemoteConsole2API.ConsoleAPI( options ) as api:
    api.connect()
    api.start_logger()
    time.sleep( 1 )
    api.sleep( 4 )

    wait_level( api, 'MainMenu' )
    api.send_console_command( 'open MainMenu' )
    api.sleep( 4 )

    api.send_ui_button( 'UMG_Button_Start,ButtonWidget', 0 )
    wait_level( api, 'MainWorld' )
    api.sleep( 4 )

    controller.ly( 1 )
    api.send_controller( controller )
    api.sleep( 2 )

    controller.ly( 0 )
    controller.on( 'A' )
    api.send_controller( controller )
    api.sleep( 1 )

    controller.reset()
    api.send_controller( controller )
    api.sleep( 1 )

    controller.ly( -1 )
    api.send_controller( controller )
    api.sleep( 2 )

    controller.ly( 0 )
    controller.on( 'A' )
    api.send_controller( controller )
    api.sleep( 1 )

    controller.reset()
    api.send_controller( controller )
    api.sleep( 3 )

    api.send_key( 'ESCAPE', 1 )
    api.sleep( 1 )
    api.send_key( 'ESCAPE', 0 )
    api.sleep( 4 )
    
    api.send_ui_button( 'UMG_Button_Reset,ButtonWidget', 0 )
    api.sleep( 4 )

    api.send_key( 'ESCAPE', 1 )
    api.sleep( 1 )
    api.send_key( 'ESCAPE', 0 )
    api.sleep( 4 )

    api.send_ui_button( 'UMG_Button_Quit,ButtonWidget', 0 )
    api.sleep( 4 )



