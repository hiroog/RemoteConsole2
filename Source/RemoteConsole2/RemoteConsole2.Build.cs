// RemoteConsole2 2020/07/25 Hiroyuki Ogasawara
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
					"UMG",
				});
			PrivateDependencyModuleNames.AddRange( new string[] {
					"ApplicationCore",
					"Networking",
					"Sockets",
					"InputCore",
					"InputDevice",
					"SlateCore",
					"Slate",
				});
		}
	}
}


