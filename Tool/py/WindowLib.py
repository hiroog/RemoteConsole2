# 2018/06/10 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

import  os
import  tkinter as tk
import  tkinter.filedialog, tkinter.messagebox
import  json


def is_readonly( file_name ):
    if os.path.exists( file_name ):
        st= os.stat( file_name )
        if (st.st_mode & 0o202) == 0:
            return  True
    return  False


class CheckAction:
    def __init__( self, title, param= '', preset= False ):
        self.title= title
        self.param= param
        self.ch= tk.IntVar()
        if preset:
            self.setValue( True )

    def isChecked( self ):
        return  self.ch.get() != 0

    def setValue( self, value ):
        self.ch.set( value )


# SelectOptions( 'save-name', { 'sort-name' : CheckAction( 'disp-name', 'opt-param', False ), }, [ 'sort-name' ] )
class CheckOptions:
    def __init__( self, name, option_list, order_list ):
        self.name= name
        self.option_list= option_list
        self.order_list= order_list

    def add_win( self, parent_win, title ):
        frame_win= tk.Frame( parent_win )
        tk.Label( frame_win, text=title, bg='white' ).pack( anchor= tk.W )
        for cname in self.order_list:
            action= self.option_list[cname]
            tk.Checkbutton( frame_win, text= action.title, variable=action.ch ).pack( anchor=tk.W )
        return  frame_win

    def list( self ):
        #return  self.option_list
        return  self.order_list

    def get( self, name ):
        return  self.option_list[name]

    def isChecked( self, name ):
        return  self.option_list[name].isChecked()

    def load( self, load_data ):
        if self.name in load_data:
            obj= load_data[self.name]
            for cname in obj:
                if cname in self.option_list:
                    self.option_list[cname].setValue( obj[cname] )

    def save( self, save_data ):
        obj= {}
        for cname in self.option_list:
            action= self.option_list[cname]
            obj[cname]= action.isChecked()
        save_data[self.name]= obj


class SelectOptions:
    def __init__( self, name, enum_list, index= 0 ):
        self.name= name
        self.enum_list= enum_list
        self.var= tk.StringVar()
        self.set( enum_list[index] )

    def add_win( self, parent_win, title ):
        return  tk.OptionMenu( parent_win, self.var, *self.enum_list )

    def set_index( self, index ):
        self.var.set( self.enum_list[index] )

    def get( self ):
        return  self.var.get()

    def set( self, data ):
        self.var.set( data )

    def load( self, load_data ):
        if self.name in load_data:
            self.set( load_data[self.name] )

    def save( self, save_data ):
        save_data[ self.name ]= self.get()


class CheckOption:
    def __init__( self, name ):
        self.name= name
        self.var= tk.IntVar()

    def add_win( self, parent_win, title ):
        frame_win= tk.Frame( parent_win )
        tk.Checkbutton( frame_win, text= title, variable=self.var ).pack( anchor=tk.W )
        return  frame_win

    def set( self, value ):
        self.var.set( value )

    def get( self ):
        return  self.var.get()

    def isChecked( self ):
        return  self.var.get() != 0

    def load( self, load_data ):
        if self.name in load_data:
            self.set( load_data[self.name] )

    def save( self, save_data ):
        save_data[ self.name ]= self.get()


class TextOption:
    def __init__( self, name ):
        self.name= name
        self.var= tk.StringVar()

    def add_win( self, parent_win, title ):
        frame_win= tk.Frame( parent_win )
        tk.Label( frame_win, text=title ).pack( side= tk.LEFT )
        tk.Entry( frame_win, textvariable=self.var ).pack( fill=tk.BOTH )
        return  frame_win

    def set( self, text ):
        self.var.set( text )

    def get( self ):
        return  self.var.get()

    def load( self, load_data ):
        if self.name in load_data:
            self.set( load_data[self.name] )

    def save( self, save_data ):
        save_data[ self.name ]= self.get()


class FileOption:
    def __init__( self, name, mode= 'OPEN' ):
        self.name= name
        self.mode= mode
        self.var= tk.StringVar()

    def add_win( self, parent_win, title ):
        frame_win= tk.Frame( parent_win )
        tk.Label( frame_win, text=title ).pack( side= tk.LEFT )
        tk.Button( frame_win, text= '...', command=lambda: self.select_file() ).pack( side=tk.RIGHT )
        tk.Entry( frame_win, textvariable=self.var ).pack( fill=tk.BOTH )
        return  frame_win

    def select_file( self ):
        file= None
        if self.mode == 'OPEN':
            file= tk.filedialog.askopenfilename()
        elif self.mode == 'SAVE':
            file= tk.filedialog.asksavefilename()
        elif self.mode == 'DIR':
            file= tk.filedialog.askdirectory()
        if file is not None and file != '':
            self.set( file )

    def set( self, text ):
        self.var.set( text )

    def get( self ):
        return  self.var.get()

    def load( self, load_data ):
        if self.name in load_data:
            self.set( load_data[self.name] )

    def save( self, save_data ):
        save_data[ self.name ]= self.get()


def SaveWindowOption( file_name, save_list ):
    if is_readonly( file_name ):
        print( 'Caution!! ' + file_name + ' is READ ONLY' )
    else:
        obj= {}
        for save_item in save_list:
            save_item.save( obj )
        with open( file_name, 'w' ) as fo:
            fo.write( json.dumps( obj, sort_keys=True, indent=4 ) )


def LoadWindowOption( file_name, save_list ):
    if os.path.exists( file_name ):
        print( 'Load=' + file_name )
        with open( file_name ) as fi:
            obj= json.loads( fi.read() )
            for save_item in save_list:
                save_item.load( obj )


