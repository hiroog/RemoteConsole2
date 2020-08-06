# vim:ts=4 sw=4 et:

sys.path.append( os.path.join( tool.getEnv( 'UALIB' ), '../FileListLib' ) )


def func_nop( task ):
    pass

tool.addScriptTask( genv, 'build', func_nop )


def func_copyto3( task ):
    import FileTools
    cmd= [ '', '-i',  '.fl_copyto3', '--verbose', '--copy', '../re3/RemoteConsole2' ]
    FileTools.main( cmd )

tool.addScriptTask( genv, 'copyto3', func_copyto3 )


def func_list( task ):
    import FileTools
    cmd= [ '', '-i',  '.fl_copyto3', '--list' ]
    FileTools.main( cmd )

tool.addScriptTask( genv, 'list', func_list )

