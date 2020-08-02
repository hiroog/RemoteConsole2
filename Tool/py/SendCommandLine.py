# 2020/07/25 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import sys
import socket
import struct


class SCC_Error( Exception ):
    def __init__( self, message ):
        self.message= message
    def __str__( self ):
        return  self.message


class ConnectionSocket:
    def __init__( self, sock, addr ):
        self.addr= addr
        self.sock= sock

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

    def createAndConnect( host, port, ip_version= 4 ):
        connection= None
        retry_count= 0
        while True:
            connection= ConnectionSocket.createFromHost( host, port, ip_version )
            connection.sock.settimeout( 2.0 )
            try:
                connection.connect()
            except socket.timeout as e:
                print( 'Ignore timeout : ' + str(e) + ' ' + host )
                if connection is not None:
                    connection.close()
                if retry_count > 0:
                    retry_count-= 1
                    continue
                raise SCC_Error( 'timeout' )
            connection.sock.settimeout( None )
            break
        return  connection

    def __enter__( self ):
        return  self

    def __exit__( self, exception_type, exception_value, traceback ):
        self.close()
        return  True

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

    def sendCommand( self, command ):
        binary= command.encode( 'utf-8' )
        header_binary= struct.pack( '<HBB', 0x7083, 2, len(binary) )
        self.sendBinary( header_binary + binary )

    def close( self ):
        if self.sock:
            self.sock.close()
            self.sock= None


def usage():
    print( 'SendCommand v1.00 Hiroyuki Ogasawara' )
    print( 'usage: SendCommand [<options>] <cmd>...' )
    print( '  -4         use ipv4' )
    print( '  -6         use ipv6' )
    print( '  -p <port>  port (default 10101)' )
    print( '  -h <host>  host (default localhost)' )
    print( 'ex. SendCommand -h 192.168.0.10 stat fps' )
    print( 'ex. SendCommand -6 stat unit' )
    sys.exit( 0 )



def main( argv ):
    acount= len(argv)
    command= None
    host= 'localhost'
    port= 10101
    ai= 1
    ip_version= 4
    while ai < acount:
        arg= argv[ai]
        if arg[0] == '-':
            if arg == '-4':
                ip_version= 4
            elif arg == '-6':
                ip_version= 6
            elif arg == '-h' or arg == '--host':
                if ai+1 < acount:
                    ai+= 1
                    host= argv[ai]
            elif arg == '-p' or arg == '--port':
                if ai+1 < acount:
                    ai+= 1
                    port= int(argv[ai])
        else:
            if command:
                command+= ' ' + arg
            else:
                command= arg
        ai+= 1
    if command:
        #print( host, port, ip_version )
        sock= ConnectionSocket.createAndConnect( host, port, ip_version )
        sock.sendCommand( command )
    else:
        usage()
    return  0


if __name__=='__main__':
    sys.exit( main( sys.argv ) )


