# 2020/08/01 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import os
import sys
import socket
import struct
import tkinter as tk
import WindowLib as wlib
import RemoteConsole2API


class SendCommandGUI:

    APP_NAME= 'SendCommandGUI.py'
    VERSION= 'v2.00'
    CONFIG_FILENAME= 'send_command_option.txt'

    def __init__( self, config_path ):
        self.win= tk.Tk()
        self.win.title( self.APP_NAME + ' ' + self.VERSION )

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

        send_win= tk.LabelFrame( self.win, text='Command', padx=14, pady=14 )
        self.command.add_win( send_win, 'Cmd:' ).pack( fill=tk.BOTH )
        tk.Button( send_win, text='Send  ([Enter])', command=lambda: self.send() ).pack( fill=tk.BOTH )
        tk.Button( send_win, text='Clear ([Ctrl]+[U])', command=lambda: self.key_clear(None) ).pack( fill=tk.BOTH )
        send_win.pack( side=tk.TOP, fill=tk.BOTH )
        self.win.bind( '<Return>', self.key_enter )
        self.win.bind( '<Control-Key-u>', self.key_clear )

        main_win= tk.LabelFrame( self.win, text='Host Settings', padx=14, pady=14 )
        self.host.add_win( main_win, 'Host:' ).pack( fill=tk.BOTH )
        self.port.add_win( main_win, 'Port:' ).pack( fill=tk.BOTH )
        self.ipv6.add_win( main_win, 'IPv6' ).pack( fill=tk.BOTH )
        main_win.pack( side=tk.TOP, fill=tk.BOTH )

        tk.Button( self.win, text='Window Close', command=lambda: self.close() ).pack( fill=tk.BOTH )

    def main_loop( self ):
        self.win.mainloop()
        self.save_options()

    def key_enter( self, event ):
        self.send()

    def key_clear( self, event ):
        self.command.set( '' )

    def send( self ):
        command= self.command.get()
        if command == '':
            return
        host= self.host.get()
        port= int(self.port.get())
        ipv= 4
        if self.ipv6.isChecked():
            ipv= 6
        try:
            with RemoteConsole2API.ConnectionSocket.createAndConnect( host, port, ipv ) as sock:
                sock.sendTextCommand( RemoteConsole2API.ConnectionSocket.CMD_CONSOLE_CMD, command )
        except RemoteConsole2API.SCC_Error as e:
            print( 'SendCommand ERROR: Connection timeout host="%s" port=%d' % (host, port) )
        except socket.gaierror as e:
            print( 'SendCommand ERROR: Unknown hostname "%s"' % host )

    def close( self ):
        self.save_options()
        sys.exit( 0 )


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


