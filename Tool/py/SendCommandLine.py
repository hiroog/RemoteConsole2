# RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import os
import sys

sys.path.append( os.path.dirname( sys.argv[0] ) )
import RemoteConsole2API


#------------------------------------------------------------------------------

class CommandTool:

    def __init__( self, options ):
        self.options= options
        self.controller= RemoteConsole2API.Controller()
        self.console= RemoteConsole2API.ConsoleAPI( options )
        self.console.connect()

    def __enter__( self ):
        return  self

    def __exit__( self, *arg ):
        self.close()

    def close( self ):
        self.console.close()

    #--------------------------------------------------------------------------

    def f_close( self ):
        self.console.send_close()

    def f_sleep( self, params ):
        self.console.sleep( params.time )

    def f_console_cmd( self, params ):
        self.console.send_console_command( params.text )

    def f_print( self, params ):
        self.console.print_string( params.text )

    def f_wait_log( self, params ):
        self.console.wait_log( params.text )

    def f_bg_logger( self, params ):
        self.console.start_logger()

    #--------------------------------------------------------------------------

    def f_on( self, params ):
        button_list= params.pad_button.split( ',' )
        for button_name in button_list:
            self.controller.on( button_name )
        self.console.send_controller( self.controller )

    def f_off( self, params ):
        button_list= params.pad_button.split( ',' )
        for button_name in button_list:
            self.controller.off( button_name )
        self.console.send_controller( self.controller )

    def f_lx( self, params ):
        self.controller.lx( float(params.value) )
        self.console.send_controller( self.controller )

    def f_ly( self, params ):
        self.controller.ly( float(params.value) )
        self.console.send_controller( self.controller )

    def f_rx( self, params ):
        self.controller.rx( float(params.value) )
        self.console.send_controller( self.controller )

    def f_ry( self, params ):
        self.controller.ry( float(params.value) )
        self.console.send_controller( self.controller )

    def f_tl( self, params ):
        self.controller.tl( float(params.value) )
        self.console.send_controller( self.controller )

    def f_tr( self, params ):
        self.controller.tr( float(params.value) )
        self.console.send_controller( self.controller )

    def f_pad_reset( self, params ):
        self.controller.reset()
        self.console.send_controller( self.controller )

    #--------------------------------------------------------------------------

    def f_key_down( self, params ):
        self.console.send_key( params.key, RemoteConsole2API.Event.KEY_DOWN )

    def f_key_up( self, params ):
        self.console.send_key( params.key, RemoteConsole2API.Event.KEY_UP )

    def f_key_char( self, params ):
        self.console.send_key( params.key, RemoteConsole2API.Event.KEY_CHAR )

    #--------------------------------------------------------------------------

    def f_mouse_down( self, params ):
        self.console.send_mouse_button( params.key, RemoteConsole2API.Event.MOUSE_DOWN )

    def f_mouse_up( self, params ):
        self.console.send_mouse_button( params.key, RemoteConsole2API.Event.MOUSE_UP )

    def f_mouse_move( self, params ):
        self.console.send_mouse_move( params.posx, params.posy )

    def f_mouse_setpos( self, params ):
        self.console.send_mouse_setpos( params.posx, params.posy )

    #--------------------------------------------------------------------------

    def f_ui_clicked( self, params ):
        self.console.send_ui_button( params.widget_name, RemoteConsole2API.Event.UI_BUTTON_CLICKED );

    def f_ui_dump( self, params ):
        self.console.send_ui_dump()

    def f_focus_ui( self, params ):
        self.console.set_focus( params.widget_name )

    def f_focus_game( self, params ):
        self.console.set_focus( None )

    #--------------------------------------------------------------------------

    def f_level_name( self, params ):
        print( self.console.get_level_name().get() )

    def f_get_var( self, params ):
        print( self.console.get_console_var( params.text ).get() )

    #--------------------------------------------------------------------------

    def f_replay( self, params ):
        self.console.replay( params.replay )

    def f_decode( self, params ):
        self.console.decode( params.replay )

    #--------------------------------------------------------------------------

    def f_run( self ):
        for func,params in self.options.func_list:
            if hasattr( self, func ):
                getattr( self, func )( params )
            else:
                self.console.print( 'unknown command error', func )
        self.f_close()



#------------------------------------------------------------------------------

def usage():
    print( 'SendCommand v2.30 Hiroyuki Ogasawara' )
    print( 'usage: SendCommand [<options>] <cmd>...' )
    print( '  -4                   use ipv4' )
    print( '  -6                   use ipv6' )
    print( '  -p <port>            port (default 10101)' )
    print( '  -h <host>            host (default 127.0.0.1)' )
    print( '  --timeout <sec>      connection timeout (default 1200)' )
    print( '  --sleep <sec>        sleep' )
    print( '  --cmd <command>      send console command' )
    print( '  --on <buttons>' )
    print( '  --off <buttons>' )
    print( '  --lx <value>' )
    print( '  --ly <value>' )
    print( '  --rx <value>' )
    print( '  --ry <value>' )
    print( '  --tl <value>' )
    print( '  --tr <value>' )
    print( '  --pad                send controller status' )
    print( '  --pad_reset' )
    print( '  --print <text>' )
    print( '  --get_log' )
    print( '  --get_log_loop' )
    print( '  --key_down <keycode>' )
    print( '  --key_up <keycode>' )
    print( '  --key_char <keycode>' )
    print( '  --mx <mouse cursor_x>' )
    print( '  --my <mouse cursor_y>' )
    print( '  --mouse_move' )
    print( '  --mouse_down <button>     L,M,R' )
    print( '  --mouse_up <button>       L,M,R' )
    print( '  --ui_clicked <widget-name>' )
    print( '  --ui_dump' )
    print( '  --focus_ui <widget-name>' )
    print( '  --focus_game' )
    print( '  --level_name' )
    print( '  --get_var <var-name>' )
    print( '  --wait_log <text>' )
    print( '  --bg_logger          start logging thread' )
    print( '  --replay <replay_file>' )
    print( '  --log_echo' )
    print( '  --net_echo' )
    print( 'ex. SendCommand -h 192.168.0.10 --cmd stat fps' )
    print( 'ex. SendCommand -6 --cmd stat unit' )
    print( 'ex. SendCommand --on A,B,U,L --sleep 1 --pad_reset' )
    print( 'ex. SendCommand --ry -1 --sleep 1' )
    print( 'ex. SendCommand --key_down W --sleep 1 --key_up W' )
    print( 'ex. SendCommand --wait_log BeginPlay --sleep 1 --key_down W --sleep 1' )
    sys.exit( 0 )


class Params( RemoteConsole2API.OptionBase ):
    def __init__( self ):
        super().__init__()
        self.text= ''
        self.key= ''
        self.time= 1.0
        self.pad_button= ''
        self.value= 0.0
        self.widget_name= ''
        self.replay= 'key_log.txt'
        self.posx= 0
        self.posy= 0


def main( argv ):
    options= RemoteConsole2API.Options()
    acount= len(argv)
    ai= 1
    last_params= None
    params= Params()
    while ai < acount:
        func= None
        arg= argv[ai]
        if arg[0] == '-':
            last_params= None
            if arg == '-4':
                options.ipv= 4
            elif arg == '-6':
                options.ipv= 6
            elif arg == '-h' or arg == '--host':
                ai= options.set_str( ai, argv, 'host' )
            elif arg == '-p' or arg == '--port':
                ai= options.set_int( ai, argv, 'port' )
            elif arg == '--timeout':
                ai= options.set_int( ai, argv, 'timeout' )
            elif arg == '--mx':
                ai= params.set_int( ai, argv, 'posx' )
            elif arg == '--my':
                ai= params.set_int( ai, argv, 'posy' )
            elif arg == '--on':
                ai= params.set_str( ai, argv, 'pad_button' )
                func= 'f_on'
            elif arg == '--off':
                ai= params.set_str( ai, argv, 'pad_button' )
                func= 'f_off'
            elif arg == '--lx':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_lx'
            elif arg == '--ly':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_ly'
            elif arg == '--rx':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_ry'
            elif arg == '--ry':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_ry'
            elif arg == '--tl':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_tl'
            elif arg == '--tr':
                ai= params.set_float( ai, argv, 'value' )
                func= 'f_tr'
            elif arg == '--print':
                ai= params.set_str( ai, argv, 'text' )
                last_params= params
                func= 'f_print'
            elif arg == '--key_down':
                ai= params.set_str( ai, argv, 'key' )
                func= 'f_key_down'
            elif arg == '--key_up':
                ai= params.set_str( ai, argv, 'key' )
                func= 'f_key_up'
            elif arg == '--key_char':
                ai= params.set_str( ai, argv, 'key' )
                func= 'f_key_char'
            elif arg == '--mouse_down':
                ai= params.set_str( ai, argv, 'key' )
                func= 'f_mouse_down'
            elif arg == '--mouse_up':
                ai= params.set_str( ai, argv, 'key' )
                func= 'f_mouse_up'
            elif arg == '--mouse_move':
                func= 'f_mouse_move'
            elif arg == '--mouse_setpos':
                func= 'f_mouse_setpos'
            elif arg == '--bg_logger':
                func= 'f_bg_logger'
            elif arg == '--log_echo':
                options.log_echo= True
            elif arg == '--net_echo':
                options.net_echo= True
            elif arg == '--wait_log':
                ai= params.set_str( ai, argv, 'text' )
                func= 'f_wait_log'
            elif arg == '--sleep':
                ai= params.set_float( ai, argv, 'time' )
                func= 'f_sleep'
            elif arg == '--cmd':
                ai= params.set_str( ai, argv, 'text' )
                last_params= params
                func= 'f_console_cmd'
            elif arg == '--ui_clicked':
                ai= params.set_str( ai, argv, 'widget_name' )
                func= 'f_ui_clicked'
            elif arg == '--focus_ui':
                ai= params.set_str( ai, argv, 'widget_name' )
                func= 'f_focus_ui'
            elif arg == '--focus_game':
                func= 'f_focus_game'
            elif arg == '--ui_dump':
                func= 'f_ui_dump'
            elif arg == '--level_name':
                func= 'f_level_name'
            elif arg == '--pad_reset':
                func= 'f_pad_reset'
            elif arg == '--replay':
                ai= params.set_str( ai, argv, 'replay' )
                func= 'f_replay'
            elif arg == '--get_var':
                ai= params.set_str( ai, argv, 'text' )
                func= 'f_get_var'
            elif arg == '--decode':
                ai= params.set_str( ai, argv, 'replay' )
                func= 'f_decode'
            else:
                usage()
        else:
            if last_params:
                last_params.text+= ' ' + arg
        if func:
            options.func_list.append( (func,params) )
            params= Params()
        ai+= 1

    if options.func_list != []:
        with CommandTool( options ) as tool:
            tool.f_run()
    else:
        usage()
    return  0


if __name__=='__main__':
    sys.exit( main( sys.argv ) )


