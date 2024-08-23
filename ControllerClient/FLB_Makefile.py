# flatlib 2014 Hiroyuki Ogasawara
# vim:ts=4 sw=4 et:

genv.FLATLIB= tool.getEnv( 'FLATLIB5' )
tool.execScript( os.path.join( genv.FLATLIB, 'build/DefaultSettings.py' ) )

#------------------------------------------------------------------------------

src_list_base= [
        'src/linux_main.cpp',
        'src/NetworkClient.cpp',
    ]
src_list= src_list_base + [
    ]


TargetName= 'remote_controller_client'

env= tool.createTargetEnvironment()
env.addLibraries( [ 'flatCore', 'flatInput' ] )
env.addCCFlags( ['-DRC2_USE_WINDOW=0' ] )

if env.getHostPlatform() == 'Windows':
    env.addLibraries( [ 'ws2_32', 'advapi32', 'dinput8', 'dxguid', 'xinput', 'ole32', 'oleaut32' ] )
    env.setEnv( 'MSLIB', 'MT' )

#------------------------------------------------------------------------------
def lib_func( env ):
    env.addLibPaths( [ env.getOutputPath( os.path.join( env.tool.global_env.FLATLIB, 'lib' ) ) ] )

def exe_func( env ):
    global TargetName
    if env.getConfig() == 'Release':
        return  env.getExeName( TargetName )
    return  env.getExeName( TargetName + '_' + env.getTargetArch() + '_' + env.getConfig() )

env.refresh()
tool.addExeTasks( env, 'build', TargetName, src_list, [ 'Debug', 'Release', 'Retail' ], [ env.getHostArch() ], lib_func= lib_func, exe_func= exe_func )
tool.addExeTasks( env, 'single', TargetName, src_list, [ 'Debug' ], [ 'x64' ], lib_func= lib_func )


#------------------------------------------------------------------------------
