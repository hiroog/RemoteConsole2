// RemoteConsole2 2020/07/25
// vim:ts=4 sw=4 noet:

using	UnrealBuildTool;
using	System.IO;

namespace UnrealBuildTool.Rules
{
	public class RemoteConsole2 : ModuleRules {

		public RemoteConsole2( ReadOnlyTargetRules Target ) : base( Target )
		{
			PublicDependencyModuleNames.AddRange( new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"InputDevice",
					"Networking",
					"Sockets",
				});
			if( File.Exists( Path.Combine( ModuleDirectory, "../ThirdParty/ualibnet/ualibnet.Build.cs" ) ) ){
				PublicDefinitions.AddRange( new string[] {
						"USE_UALIBNET=1",
					});
				PublicDependencyModuleNames.AddRange( new string[] {
						"ualibnet",
					});
			}else{
				PublicDefinitions.AddRange( new string[] {
						"USE_UALIBNET=0",
					});
			}
		}
	}
}

