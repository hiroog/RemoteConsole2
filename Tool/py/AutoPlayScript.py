# RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import RemoteConsole2API


options= RemoteConsole2API.Options()

with RemoteConsole2API.ConsoleAPI( options ) as api:
    api.connect()
    api.start_logger()
    api.get_level_name()
    #api.wait_log( r'RemoteConsole2:GameAPI:Initialized' )
    api.sleep( 2 )
    api.send_console_command( 'open ThirdPersonMap' )
    api.sleep( 3 )
    api.set_focus()
    api.send_key( 'W', 1 )
    api.sleep( 2 )
    api.send_key( 'W', 0 )
    api.sleep( 2 )
    api.set_focus( 'HUDTopWidget_C_0,BPUserWidgetTest,UserButton001' )
    api.send_ui_button( 'HUDTopWidget_C_0,BPUserWidgetTest,UserButton001', 0 )
    api.sleep( 2 )
    api.send_console_command( 'quit_editor' )




