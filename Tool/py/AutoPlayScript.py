# RemoteConsole2 2024/08/03 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import RemoteConsole2API


options= RemoteConsole2API.Options()

with RemoteConsole2API.ConsoleAPI( options ) as api:
    api.connect()
    api.start_logger()
    api.sleep( 0.5 )
    level_name= api.get_level_name()
    resolution= api.get_console_var( 'r.SetRes' )
    print( '***** %s *****' % level_name.get() )
    print( '***** %s *****' % resolution.get() )
#    api.send_console_command( 'open ThirdPersonMap' )
    api.sleep( 0.5 )
    api.set_focus( None, True )
#    api.send_key( 'W', 1 )
#    api.sleep( 4 )
#    api.send_key( 'W', 0 )
#    api.sleep( 1 )
    for a in range(10):
        api.send_mouse_move( 80 + a, 0 )
        api.sleep( 2.0/60.0 )
#    api.send_mouse_button( 'L', RemoteConsole2API.Event.MOUSE_DOWN )
#    api.sleep( 0.2 )
#    api.send_mouse_button( 'L', RemoteConsole2API.Event.MOUSE_UP )
    api.sleep( 1 )
    #api.set_focus( 'HUDTopWidget_C_0,BPUserWidgetTest,UserButton001' )
    #api.send_ui_button( 'HUDTopWidget_C_0,BPUserWidgetTest,UserButton001', 0 )
    #api.sleep( 2 )
    #api.send_console_command( 'quit_editor' )




