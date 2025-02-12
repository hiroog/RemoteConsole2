# RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import socket
import struct
import time
import re
import collections
import threading


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class Controller:
    PAD_MAP= {
            'U': 1<<0,
            'D': 1<<1,
            'L': 1<<2,
            'R': 1<<3,
            'A': 1<<4,
            'B': 1<<5,
            'X': 1<<6,
            'Y': 1<<7,
            'L1': 1<<8,
            'R1': 1<<9,
            'L2': 1<<10,
            'R2': 1<<11,
            'L3': 1<<12,
            'R3': 1<<13,
            'M0': 1<<14,
            'M1': 1<<15,
            'M2': 1<<16,
            'M3': 1<<17,
        }
    def __init__( self ):
        self.reset()

    def reset( self ):
        self.reset_button()
        self.reset_stick()
        return  self

    def reset_button( self ):
        self.button_bit= 0
        return  self

    def reset_stick( self ):
        self.stick= [0,0,0,0,0,0]
        return  self

    def on( self, button_name ):
        if button_name in self.PAD_MAP:
            self.button_bit|= self.PAD_MAP[button_name]
        return  self

    def off( self, button_name ):
        if button_name in self.PAD_MAP:
            self.button_bit&= ~self.PAD_MAP[button_name]
        return  self

    def stick_value( self, value ):
        return  int(float(value) * 32767) & 0xffff

    def lx( self, value ):
        self.stick[0]= self.stick_value( value )
        return  self

    def ly( self, value ):
        self.stick[1]= self.stick_value( value )
        return  self

    def rx( self, value ):
        self.stick[2]= self.stick_value( value )
        return  self

    def ry( self, value ):
        self.stick[3]= self.stick_value( value )
        return  self

    def tl( self, value ):
        self.stick[4]= self.stick_value( value )
        return  self

    def tr( self, value ):
        self.stick[5]= self.stick_value( value )
        return  self

    def __str__( self ):
        return  ''.join( map( lambda val: val[0] if self.button_bit & val[1] else '_', self.PAD_MAP.items() ) )


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class SCC_Error( Exception ):
    def __init__( self, message ):
        self.message= message
    def __str__( self ):
        return  self.message


class Event:
    CMD_CLOSE        =   2
    CMD_CONTROLLER   =   257
    CMD_KEYBOARD     =   258
    CMD_MOUSE        =   259
    CMD_RETURN_LOG   =   260
    CMD_RETURN_STRING=   261
    CMD_RECV_SERVER  =   384
    CMD_CONSOLE_CMD  =   513
    CMD_PRINT_LOG    =   514
    CMD_UI_DUMP      =   515
    CMD_UI_BUTTON    =   516
    CMD_UI_FOCUS     =   517
    CMD_GET_LEVEL_NAME=  518
    CMD_GET_CONSOLE_VAR= 519

    KEY_UP      =   0
    KEY_DOWN    =   1
    KEY_CHAR    =   2
    MOUSE_UP    =   8
    MOUSE_DOWN  =   9
    MOUSE_DCLICK=   10
    MOUSE_WHEEL =   11
    MOUSE_MOVE  =   12
    MOUSE_SETPOS=   13

    UI_BUTTON_CLICKED  =   0
    UI_BUTTON_PRESSED  =   1
    UI_BUTTON_RELEASED =   2
    UI_BUTTON_HOVERED  =   3
    UI_BUTTON_UNHOVERED=   4

    UI_FOCUS_GAMEONLY =   0
    UI_FOCUS_UIONLY   =   1
    UI_FOCUS_GAMEANDUI=   2
    UI_FOCUS_WINDOW   =   3
    UI_FOCUS_BTOF     =   4
    UI_FOCUS_BTOF2    =   5

    UI_DUMP_ALL     =   0
    UI_DUMP_BUTTON  =   1
    UI_DUMP_DEBUG   =   2


class ConnectionSocket:

    def __init__( self, sock, addr ):
        self.addr= addr
        self.sock= sock
        self.net_echo= False
        self.result_key= 0

    @staticmethod
    def createFromHost( host, port, ip_version= 4 ):
        family,addr= ConnectionSocket.getAddr( host, port, ip_version )
        sock= socket.socket( family, socket.SOCK_STREAM )
        return  ConnectionSocket( sock, addr )

    @staticmethod
    def getAddr( host, port, ip_version= 4 ):
        if ip_version == 4:
            filter_family= socket.AF_INET
        else:
            filter_family= socket.AF_INET6
        for family,type,proto,ca,addr in socket.getaddrinfo( host, port, proto=socket.IPPROTO_TCP, family=filter_family ):
            return  family,addr
        return  None,None

    @staticmethod
    def createAndConnect( host, port, ip_version= 4, retry_count= 0, timeout_sec= 2.0 ):
        connection= None
        while True:
            connection= ConnectionSocket.createFromHost( host, port, ip_version )
            connection.sock.settimeout( timeout_sec )
            try:
                connection.connect()
            except socket.timeout as e:
                if connection is not None:
                    connection.close()
                    connection= None
                if retry_count > 0:
                    retry_count-= 1
                    continue
                raise e
            connection.sock.settimeout( None )
            break
        return  connection

    def __enter__( self ):
        return  self

    def __exit__( self, exception_type, exception_value, traceback ):
        self.close()
        return  None

    def set_echo( self, echo ):
        self.net_echo= echo

    def connect( self ):
        self.sock.connect( self.addr )

    def sendBinary( self, binary ):
        data_size= len(binary)
        sent_size= 0
        while sent_size < data_size:
            size= self.sock.send( binary[sent_size:] )
            if size == 0:
                raise   SCC_Error( 'socket closed' )
            sent_size+= size

    def recvBinary( self ):
        header_bin= self.sock.recv( 16 )
        if len(header_bin) != 16:
            raise   SCC_Error( 'socket closed' )
        magic,command,param0,data_size,param1= struct.unpack( '<IHHII', header_bin )
        if self.net_echo:
            print( 'recv cmd=%d size=%d param0=%d param1=%d' % (command,data_size,param0,param1), flush=True )
        buffer= b''
        if magic != 0x70198fb3:
            print( 'Header Error' )
            return  buffer
        recv_size= 0
        while recv_size < data_size:
            left_size= data_size - recv_size
            data= self.sock.recv( left_size )
            if data == 0:
                raise   SCC_Error( 'socket closed' )
            recv_size+= len(data)
            buffer+= data
        return  command,param0,param1,buffer

    def sendCommand( self, command, binary_data, param0= 0, param1= 0 ):
        binary= b''
        if binary_data:
            binary= binary_data
        if self.net_echo:
            print( 'send cmd=%d size=%d (%d,%d)' % (command,len(binary),param0,param1), flush=True )
        header_binary= struct.pack( '<IHHII', 0x70198fb3, command, param0, len(binary), param1 )
        self.sendBinary( header_binary + binary )

    def alloc_key( self ):
        result_key= self.result_key
        self.result_key= (result_key + 1) & 0xffffffff
        print( 'alloc key=%d' % result_key )
        return  result_key

    def sendTextCommand( self, command, text_data, param0= 0, param1= 0 ):
        binary= None
        if text_data:
            binary= text_data.encode( 'utf-8' )
        self.sendCommand( command, binary, param0, param1 )

    def sendController( self, controller ):
        button_binary= struct.pack( '<6HI', controller.stick[0], controller.stick[1], controller.stick[2], controller.stick[3], controller.stick[4], controller.stick[5], controller.button_bit )
        self.sendCommand( Event.CMD_CONTROLLER, button_binary )

    def sendKeyboard( self, key_code, char_code, action ):
        self.sendCommand( Event.CMD_KEYBOARD, None, action, (char_code<<16)|key_code );

    def sendMouse( self, button, action, cursor_x, cursor_y ):
        self.sendCommand( Event.CMD_MOUSE, None, (button<<8)|action, (cursor_y<<16)|(cursor_x&0xffff) );

    def close( self ):
        if self.sock:
            self.sock.close()
            self.sock= None

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class LogFile:
    def __init__( self, options ):
        self.options= options
        self.is_color= False
        self.log_file= None
        self.log_lock= threading.Lock()
        if self.options.color:
            import os
            os.system( 'color' )
            self.is_color= True
        self.open( self.options.log_file_name )

    def write_file( self, *msg ):
        if self.log_file:
            for text in msg:
                self.log_file.write( text )
            self.log_file.write( '\n' )
            self.log_file.flush()

    def print( self, *msg ):
        with self.log_lock:
            print( *msg, flush=True )
            self.write_file( *msg )

    def print_script( self, *msg ):
        with self.log_lock:
            if self.is_color:
                print( '\x1b[44m', end='' )
                print( *msg, end='' )
                print( '\x1b[0m', flush=True )
            else:
                print( '----------------', *msg, flush=True )
            self.write_file( '---------------- ', *msg )

    def open( self, file_name ):
        if file_name:
            self.close_file()
            self.log_file= open( file_name, 'w', encoding='utf-8' )

    def close_file( self ):
        if self.log_file:
            self.log_file.close()
            self.log_file= None

    def close( self ):
        self.close_file()


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class Feature:
    def __init__( self, logger, key ):
        self.logger= logger
        self.key= key
        self.result= None

    def wait( self ):
        if not self.result:
            self.result= self.logger.recv_result( self.key )

    def get( self ):
        self.wait()
        return  self.result[0]

    def get_param0( self ):
        self.wait()
        return  self.result[1]


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class BackgroundLogger(threading.Thread):

    def __init__( self, options, logfile ):
        super().__init__()
        self.options= options
        self.logfile= logfile
        self.sock= None
        self.log_queue= collections.deque()
        self.log_lock= threading.Lock()
        self.log_event= threading.Event()
        self.result_lock= threading.Lock()
        self.result_event= threading.Event()
        self.log_index= 0
        self.base_index= 0
        self.result= {}
        self.loop_flag= True

    def start_server( self ):
        if not self.is_alive():
            self.loop_flag= True
            self.start()

    def stop_server( self ):
        self.loop_flag= False
        self.log_event.set()
        self.result_event.set()
        if self.is_alive():
            self.sock.close()
            self.join()

    def print( self, *msg ):
        self.logfile.print_script( *msg )

    #--------------------------------------------------------------------------

    def clear_log( self ):
        with self.log_lock:
            self.log_queue.clear()

    def push_line( self, line ):
        self.log_queue.append( line )
        while len(self.log_queue) >= self.options.log_limit:
            self.log_queue.popleft()

    #--------------------------------------------------------------------------

    def command_exec( self, command, param0, param1, binary ):
        if command == Event.CMD_RETURN_LOG:
            text= binary.decode( 'utf-8', 'ignore' )
            with self.log_lock:
                self.push_line( (self.log_index,text) )
                self.log_index+= 1
                self.log_event.set()
            if self.options.log_echo:
                self.logfile.print( text )
        elif command == Event.CMD_RETURN_STRING:
            text= binary.decode( 'utf-8', 'ignore' )
            if self.options.net_echo:
                self.print( 'Return String "%s" %d %d' % (text, param0, param1) )
            with self.result_lock:
                self.result[param1]= text,param0
                self.result_event.set()

    #--------------------------------------------------------------------------

    def run( self ):
        while self.loop_flag:
            self.print( 'logger: waiting' )
            try:
                with ConnectionSocket.createAndConnect( self.options.host, self.options.port, self.options.ipv ) as self.sock:
                    self.print( 'logger: connected' )
                    self.sock.set_echo( self.options.net_echo )
                    try:
                        self.sock.sendCommand( Event.CMD_RECV_SERVER, None )
                    except SCC_Error as e:
                        break
                    while self.loop_flag:
                        try:
                            command,param0,param1,binary= self.sock.recvBinary()
                            self.command_exec( command, param0, param1, binary )
                        except SCC_Error as e:
                            self.print( 'logger:', e )
                            break
                        except ConnectionResetError as e:
                            self.print( 'logger: Connection Reset' )
                            break
                        except ConnectionAbortedError as e:
                            self.print( 'logger: Connection Aborted' )
                            break
            except socket.timeout as e:
                self.print( 'logger: connection timeout' )
                continue
        self.print( 'logger: stop server' )

    #--------------------------------------------------------------------------

    def recv_result( self, result_key ):
        while self.loop_flag:
            with self.result_lock:
                if result_key in self.result:
                    self.result_event.clear()
                    return  self.result.pop( result_key )
            if self.result_event.wait( self.options.timeout ):
                continue
            break
        return  None

    #--------------------------------------------------------------------------

    def get_index( self ):
        with self.log_lock:
            return  self.base_index

    def set_index( self, index ):
        with self.log_lock:
            self.base_index= index

    def find_log( self, pattern ):
        with self.log_lock:
            for index,line in self.log_queue:
                if index>= self.base_index:
                    pat= pattern.search( line )
                    if pat:
                        self.log_event.clear()
                        self.base_index= index+1
                        return  pat,index
        return  None,self.base_index

    def wait_log( self, pattern ):
        while self.loop_flag:
            pat,_= self.find_log( pattern )
            if pat:
                return  pat
            if self.log_event.wait( self.options.timeout ):
                time.sleep( 0.01 )
                continue
            break
        return  None


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class ConsoleAPI:

    VKEY_MAP= {
            'BACK'   : 0x08,    # BackSpace
            'TAB'    : 0x09,    # TAB
            'RETURN' : 0x0d,    # Enter
            'SHIFT'  : 0x10,
            'CONTROL': 0x11,
            'MENU'   : 0x12,    # ALT
            'ESCAPE' : 0x1b,
            'SPACE'  : 0x20,
            'PRIOR'  : 0x21,    # PageUp
            'NEXT'   : 0x22,    # PageDown
            'END'    : 0x23,
            'HOME'   : 0x24,
            'LEFT'   : 0x25,
            'UP'     : 0x26,
            'RIGHT'  : 0x27,
            'DOWN'   : 0x28,
            'DELETE' : 0x2e,
            'F1'     : 0x70,
            'F2'     : 0x71,
            'F3'     : 0x72,
            'F4'     : 0x73,
            'F5'     : 0x74,
            'F6'     : 0x75,
            'F7'     : 0x76,
            'F8'     : 0x77,
            'F9'     : 0x78,
            'F10'    : 0x79,
            'F11'    : 0x7a,
            'F12'    : 0x7b,
        }
    MOUSE_MAP= {
            'L' :   0,
            'M' :   1,
            'R' :   2,
            'T1':   3,
            'T2':   4,
        }

    def __init__( self, options ):
        self.options= options
        self.sock= None
        self.controller= Controller()
        self.logger= None
        self.logfile= LogFile( options )

    def __enter__( self ):
        return  self

    def __exit__( self, *arg ):
        self.close()
        return  None

    def close( self ):
        if self.logger:
            self.logger.stop_server()
            self.logger= None
        if self.sock:
            self.sock.close()
            self.sock= None
        if self.logfile:
            self.logfile.close()
            self.logfile= None

    def print( self, *msg ):
        self.logfile.print_script( *msg )

    def connect( self ):
        self.print( 'waiting' )
        self.sock= ConnectionSocket.createAndConnect( self.options.host, self.options.port, self.options.ipv, self.options.timeout/2 )
        self.sock.set_echo( self.options.net_echo )
        self.print( 'connected' )

    #--------------------------------------------------------------------------

    def sleep( self, time_sec ):
        self.print( 'cmd:sleep %.2f sec' % time_sec )
        time.sleep( time_sec )

    def start_logger( self ):
        if not self.logger:
            self.print( 'start logger' )
            self.logger= BackgroundLogger( self.options, self.logfile )
            self.logger.start_server()

    def stop_logger( self ):
        if self.logger:
            self.print( 'stop logger' )
            self.logger.stop_server()
            self.logger= None

    #--------------------------------------------------------------------------

    def send_close( self ):
        self.sock.sendCommand( Event.CMD_CLOSE, None )

    def send_console_command( self, command ):
        self.print( 'cmd:console command "%s"' % command )
        self.sock.sendTextCommand( Event.CMD_CONSOLE_CMD, command )

    def send_print_string( self, text ):
        self.print( 'cmd:print string "%s"' % text )
        self.sock.sendTextCommand( Event.CMD_PRINT_LOG, text )

    def wait_log( self, text ):
        self.print( 'cmd:wait log "%s"' % text )
        log_pattern= re.compile( text )
        pat= self.logger.wait_log( log_pattern )
        if pat:
            self.print( '  Found Log: %s' % pat.group(0) )
            return  pat.group(0)
        return  None

    #--------------------------------------------------------------------------

    def send_controller( self, controller ):
        self.print( 'cmd:send controller', str(controller) )
        self.sock.sendController( controller )

    def click_controller( self, button_name, controller= None, duration=0.1 ):
        if controller is None:
            controller= Controller()
        controller.on( button_name )
        self.send_controller( controller )
        time.sleep( duration )
        controller.off( button_name )
        self.send_controller( controller )

    def send_keycode( self, key_code, action ):
        self.print( 'cmd:send keycode %d (%d)' % (key_code, action) )
        self.sock.sendKeyboard( key_code, key_code, action )

    def get_keycode( self, key_name ):
        if key_name in self.VKEY_MAP:
            return  self.VKEY_MAP[key_name]
        return  ord( key_name )

    def send_key( self, key_name, action ):
        self.print( 'cmd:send key %s (%d)' % (key_name, action) )
        key_code= self.get_keycode( key_name )
        self.sock.sendKeyboard( key_code, key_code, action )

    def click_key( self, key_name, duration=0.1 ):
        self.send_key( key_name, Event.KEY_DOWN )
        time.sleep( duration )
        self.send_key( key_name, Event.KEY_UP )

    def send_mouse_button( self, button, action ):
        self.print( 'cmd:send mouse %s (%d)' % (button, action) )
        self.sock.sendMouse( self.MOUSE_MAP[button], action, 0, 0 )

    def click_mouse( self, button, duration=0.1 ):
        self.send_mouse_button( button, Event.MOUSE_DOWN )
        time.sleep( duration )
        self.send_mouse_button( button, Event.MOUSE_UP )

    def send_mouse_move( self, cursor_x, cursor_y ):
        self.print( 'cmd:send mouse move (%d,%d)' % (cursor_x, cursor_y) )
        self.sock.sendMouse( 0, Event.MOUSE_MOVE, cursor_x, cursor_y )

    def send_mouse_setpos( self, cursor_x, cursor_y ):
        self.print( 'cmd:send mouse setpos (%d,%d)' % (cursor_x, cursor_y) )
        self.sock.sendMouse( 0, Event.MOUSE_SETPOS, cursor_x, cursor_y )

    #--------------------------------------------------------------------------

    def send_ui_button( self, widget_name, action ):
        self.print( 'cmd:ui button %s (%d)' % (widget_name, action) )
        self.sock.sendTextCommand( Event.CMD_UI_BUTTON, widget_name, action, 0 );

    def send_ui_dump( self, dump_mode ):
        self.print( 'cmd:ui dump', dump_mode )
        self.sock.sendCommand( Event.CMD_UI_DUMP, None, dump_mode, 0 );

    #--------------------------------------------------------------------------

    def set_focus( self, widget_name= None, force= False ):
        if widget_name is not None:
            self.print( 'cmd:focus ui-only %s' % widget_name )
            self.sock.sendCommand( Event.CMD_UI_FOCUS, None, Event.UI_FOCUS_WINDOW, 0 );
            if force:
                self.sock.sendCommand( Event.CMD_UI_FOCUS, None, Event.UI_FOCUS_BTOF2, 0 );
            self.sock.sendTextCommand( Event.CMD_UI_FOCUS, widget_name, Event.UI_FOCUS_UIONLY, 0 );
        else:
            self.print( 'cmd:focus game-only' )
            self.sock.sendCommand( Event.CMD_UI_FOCUS, None, Event.UI_FOCUS_WINDOW, 0 );
            if force:
                self.sock.sendCommand( Event.CMD_UI_FOCUS, None, Event.UI_FOCUS_BTOF2, 0 );
            self.sock.sendCommand( Event.CMD_UI_FOCUS, None, Event.UI_FOCUS_GAMEONLY, 0 );

    #--------------------------------------------------------------------------

    def send_request_api( self, command, param0 ):
        key= self.sock.alloc_key()
        self.sock.sendCommand( command, None, param0, key );
        return  Feature( self.logger, key )

    def send_request_api_text( self, command, param0, text ):
        key= self.sock.alloc_key()
        self.sock.sendTextCommand( command, text, param0, key );
        return  Feature( self.logger, key )

    def get_level_name( self ):
        self.print( 'cmd:get level name' )
        return  self.send_request_api( Event.CMD_GET_LEVEL_NAME, 0 )

    def get_console_var( self, var_name ):
        self.print( 'cmd:get console var "%s"' % var_name )
        return  self.send_request_api_text( Event.CMD_GET_CONSOLE_VAR, 0, var_name )

    #--------------------------------------------------------------------------

    def replay_wait( self, clock, base_sys_clock ):
        while True:
            sys_clock= time.perf_counter()-base_sys_clock
            if sys_clock >= clock:
                break
            dclock= clock - sys_clock
            if dclock >= 0.012:
                time.sleep( 0.01 )

    def replay( self, log_file ):
        self.print( 'cmd:replay %s' % log_file )
        with open( log_file, 'r' ) as fi:
            base_replay_clock= -1.0;
            base_sys_clock= time.perf_counter()
            for line in fi:
                line= line.strip()
                if line == '' or line[0] == '#':
                    continue
                params= line.split()
                if params[0] == 'C':
                    controller= Controller()
                    clock= float(params[1])
                    controller.button_bit= int(params[2],16)
                    controller.stick[0]= int(params[3]) & 0xffff
                    controller.stick[1]= int(params[4]) & 0xffff
                    controller.stick[2]= int(params[5]) & 0xffff
                    controller.stick[3]= int(params[6]) & 0xffff
                    controller.stick[4]= int(params[7]) & 0xffff
                    controller.stick[5]= int(params[8]) & 0xffff
                    if base_replay_clock < 0.0:
                        base_replay_clock= clock
                        base_sys_clock= time.perf_counter()
                    self.replay_wait( clock - base_replay_clock, base_sys_clock )
                    self.sock.sendController( controller )
                elif params[0] == 'K':
                    clock= float(params[1])
                    key_code= int(params[2],16)
                    action= key_code >> 16
                    key_code&= 0xffff
                    if base_replay_clock < 0.0:
                        base_replay_clock= clock
                        base_sys_clock= time.perf_counter()
                    self.replay_wait( clock - base_replay_clock, base_sys_clock )
                    self.sock.sendKeyboard( key_code, key_code, action )

    def decode( self, log_file ):
        self.print( 'cmd:decode %s' % log_file )
        prev_clock= -1
        with open( log_file, 'r' ) as fi:
            for line in fi:
                line= line.strip()
                if line == '' or line[0] == '#':
                    continue
                params= line.split()
                if params[0] == 'C':
                    controller= Controller()
                    clock= float(params[1])
                    controller.button_bit= int(params[2],16)
                    controller.stick[0]= int(params[3]) & 0xffff
                    controller.stick[1]= int(params[4]) & 0xffff
                    controller.stick[2]= int(params[5]) & 0xffff
                    controller.stick[3]= int(params[6]) & 0xffff
                    controller.stick[4]= int(params[7]) & 0xffff
                    controller.stick[5]= int(params[8]) & 0xffff
                    if prev_clock >= 0:
                        diff_clock= clock - prev_clock
                        print( 'C %.5f  %.7f' % (clock, diff_clock) )
                    prev_clock= clock
                elif params[0] == 'K':
                    clock= float(params[1])
                    key_code= int(params[2],16)
                    action= key_code >> 16
                    key_code&= 0xffff
                    if prev_clock >= 0:
                        diff_clock= clock - prev_clock
                        print( 'K %.5f  %.7f' % (clock, diff_clock) )
                    prev_clock= clock


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

class OptionBase:
    def __init__( self ):
        pass

    def set_str( self, ai, argv, name ):
        acount= len(argv)
        if ai+1 < acount:
            ai+= 1
            setattr( self, name, argv[ai] )
        return  ai

    def set_int( self, ai, argv, name ):
        acount= len(argv)
        if ai+1 < acount:
            ai+= 1
            setattr( self, name, int(argv[ai]) )
        return  ai

    def set_float( self, ai, argv, name ):
        acount= len(argv)
        if ai+1 < acount:
            ai+= 1
            setattr( self, name, float(argv[ai]) )
        return  ai

    def apply( self, args ):
        for key in args:
            getattr( self,key )
            setattr( self, key, args[key] )


class Options( OptionBase ):
    def __init__( self, **args ):
        super().__init__()
        self.host= '127.0.0.1'  # '::1'
        self.port= 10101        # 10102
        self.ipv= 4             # 6
        self.func_list= []
        self.timeout= 20 * 60
        self.net_echo= False
        self.log_echo= True
        self.log_limit= 6000
        self.color= True
        self.log_file_name= None
        self.apply( args )


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------


