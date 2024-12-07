# 2020/08/01 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import os
import sys
import socket
import struct
import tkinter as tk

sys.path.append( os.path.dirname( sys.argv[0] ) )
import WindowLib as wlib
import RemoteConsole2API


class SendCommandGUI:

    APP_NAME= 'SendCommandGUI.py'
    VERSION= 'v2.20'
    CONFIG_FILENAME= 'send_command_option.txt'

    def __init__( self, config_path ):
        self.win= tk.Tk()
        self.win.title( self.APP_NAME + ' ' + self.VERSION )

        self.session= None
        self.prev_host= None
        self.controller= RemoteConsole2API.Controller()

        self.host= wlib.TextOption( 'host' )
        self.host.set( 'localhost' )
        self.port= wlib.TextOption( 'port' )
        self.port.set( '10101' )
        self.ipv6= wlib.CheckOption( 'ipv6' )
        self.ipv6.set( 0 )
        self.command= wlib.TextOption( 'command' )
        self.command.set( 'stat fps' )

        self.save_list= [
                self.host,
                self.port,
                self.ipv6,
                self.command,
            ]
        self.config_file= os.path.join( config_path, self.CONFIG_FILENAME )

    def save_options( self ):
        wlib.SaveWindowOption( self.config_file, self.save_list );

    def load_options( self ):
        wlib.LoadWindowOption( self.config_file, self.save_list );

    def init_window( self ):

        send_win= tk.LabelFrame( self.win, text='Console Command', padx=14, pady=14 )
        self.command.add_win( send_win, 'Cmd:' ).pack( fill=tk.BOTH )
        tk.Button( send_win, text='Send  ([Enter])', command=lambda: self.cmd_send() ).pack( fill=tk.BOTH )
        tk.Button( send_win, text='Clear ([Ctrl]+[U])', command=lambda: self.key_clear(None) ).pack( fill=tk.BOTH )
        send_win.pack( side=tk.TOP, fill=tk.BOTH )
        self.win.bind( '<Return>', self.key_enter )
        self.win.bind( '<Control-Key-u>', self.key_clear )

        controller_win= tk.LabelFrame( self.win, text='Controller', padx=14, pady=14 )
        button_win= tk.Frame( controller_win )
        button_list= [
                ('↑', 'U'),
                ('↓', 'D'),
                ('←', 'L'),
                ('→', 'R'),
                ('A',  'A'),
                ('B',  'B'),
                ('X',  'X'),
                ('Y',  'Y'),
                ('L1', 'L1'),
                ('R1', 'R1'),
                ('L2', 'L2'),
                ('R2', 'R2'),
                ('L3', 'L3'),
                ('R3', 'R3'),
                ('M0', 'M0'),
                ('M1', 'M1'),
                ('M2', 'M2'),
                ('M3', 'M3'),
            ]
        for bt_label,bt_name in button_list:
            b0= tk.Button( button_win, text=bt_label, command=lambda name=bt_name: self.cmd_button( name, False ) )
            b0.pack( side=tk.LEFT, fill=tk.BOTH )
            b0.bind( '<ButtonPress>', lambda event,name=bt_name:self.cmd_button( name, True ) )
        button_win.pack( side=tk.TOP, fill=tk.BOTH )
        tk.Button( controller_win, text='Dump UI (Button)', command=lambda: self.cmd_dump_ui( RemoteConsole2API.Event.UI_DUMP_BUTTON ) ).pack( fill=tk.BOTH )
        tk.Button( controller_win, text='Dump UI (All)', command=lambda: self.cmd_dump_ui( RemoteConsole2API.Event.UI_DUMP_ALL ) ).pack( fill=tk.BOTH )
        tk.Button( controller_win, text='Focus UI', command=lambda: self.cmd_focus( 'UI' ) ).pack( fill=tk.BOTH )
        tk.Button( controller_win, text='Focus Game', command=lambda: self.cmd_focus( None ) ).pack( fill=tk.BOTH )
        controller_win.pack( side=tk.TOP, fill=tk.BOTH )

        sub_win= tk.LabelFrame( self.win, text='Sub Command', padx=14, pady=14 )
        tk.Button( sub_win, text='Open Console Log', command=lambda: self.cmd_console_log() ).pack( fill=tk.BOTH )
        tk.Button( sub_win, text='Remote Console Client', command=lambda: self.cmd_remote_console_client() ).pack( fill=tk.BOTH )
        sub_win.pack( side=tk.TOP, fill=tk.BOTH )

        main_win= tk.LabelFrame( self.win, text='Host Settings', padx=14, pady=14 )
        self.host.add_win( main_win, 'Host:' ).pack( fill=tk.BOTH )
        self.port.add_win( main_win, 'Port:' ).pack( fill=tk.BOTH )
        self.ipv6.add_win( main_win, 'IPv6' ).pack( fill=tk.BOTH )
        main_win.pack( side=tk.TOP, fill=tk.BOTH )

        tk.Button( self.win, text='Window Close', command=lambda: self.cmd_close() ).pack( fill=tk.BOTH )

    def main_loop( self ):
        self.win.mainloop()
        self.save_options()
        self.close_session()

    #--------------------------------------------------------------------------

    def get_host( self ):
        port= int(self.port.get())
        host= self.host.get()
        ipv= 6 if self.ipv6.isChecked() else 4
        return  host,port,ipv

    def open_session( self ):
        if self.prev_host != self.get_host():
            self.close_session()
        if self.session is None:
            options= RemoteConsole2API.Options()
            cur_host= self.get_host()
            host,port,ipv= cur_host
            options.host= host
            options.port= port
            options.ipv= ipv
            try:
                self.session= RemoteConsole2API.ConsoleAPI( options )
                self.session.connect()
                self.prev_host= cur_host
            except RemoteConsole2API.SCC_Error as e:
                print( 'SendCommand ERROR: Connection timeout host="%s" port=%d' % (host, port) )
                self.session= None
            except socket.gaierror as e:
                print( 'SendCommand ERROR: Unknown hostname "%s"' % host )
                self.session= None

    def close_session( self ):
        if self.session:
            self.session.close()
            self.session= None

    #--------------------------------------------------------------------------

    def cmd_button( self, button_name, press_flag ):
        if press_flag:
            self.controller.on( button_name )
        else:
            self.controller.off( button_name )
        self.open_session()
        if self.session:
            self.session.send_controller( self.controller )

    #--------------------------------------------------------------------------

    def cmd_console_log( self ):
        host,port,ipv= self.get_host()
        command= 'start "Console Log" "%s" "%s/SendCommandLine.py" -h %s -p %d -%d --bg_logger --log_echo --sleep 86400' % (sys.executable,os.path.dirname(sys.argv[0]),host,port,ipv)
        print( command )
        os.system( command )

    def cmd_remote_console_client( self ):
        script_dir= os.path.dirname(sys.argv[0])
        search_path= [
            '%s\\..\\..\\ControllerClient\\remote_client.exe' % script_dir,
            '%s\\..\\..\\ControllerClient\\win32\\x64\\Release\\remote_client2022.exe' % script_dir,
            '%s\\..\\ControllerClient\\remote_client.exe' % script_dir,
            '%s\\ControllerClient\\remote_client.exe' % script_dir,
            '%s\\remote_client.exe' % script_dir,
            'remote_client.exe',
        ]
        client_exe= None
        for client_path in search_path:
            if os.path.exists( client_path ):
                client_exe= client_path
                break
        if client_exe:
            host,port,ipv= self.get_host()
            command= 'start "" "%s" -h%s -p%d -%d' % (client_exe,host,port,ipv)
            print( command )
            os.system( command )

    #--------------------------------------------------------------------------

    def key_enter( self, event ):
        self.cmd_send()

    def key_clear( self, event ):
        self.command.set( '' )

    def cmd_send( self ):
        self.open_session()
        command= self.command.get()
        if command == '':
            return
        if self.session:
            self.session.send_console_command( command )

    #--------------------------------------------------------------------------

    def cmd_dump_ui( self, dump_mode ):
        self.open_session()
        if self.session:
            self.session.send_ui_dump( dump_mode )

    def cmd_focus( self, focus_mode ):
        self.open_session()
        if self.session:
            self.session.set_focus( focus_mode )

    #--------------------------------------------------------------------------

    def cmd_close( self ):
        self.save_options()
        self.close_session()
        sys.exit( 0 )


#------------------------------------------------------------------------------

def main( argv ):
    acount= len(argv)
    config_path= '.'
    ai= 1
    while ai < acount:
        arg= argv[ai]
        if arg[0] == '-':
            if arg == '-c':
                if ai+1 < acount:
                    ai+= 1
                    config_path= argv[ai]
        ai+= 1
    win= SendCommandGUI( config_path )
    win.load_options()
    win.init_window()
    win.main_loop()
    return  0


if __name__=='__main__':
    sys.exit( main( sys.argv ) )


