using UnrealBuildTool;

public class LevelSequenceUtils : ModuleRules
{
	public LevelSequenceUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			[
				// ... add public include paths required here ...
			]
		);


		PrivateIncludePaths.AddRange(
			[
				// ... add other private include paths required here ...
			]
		);


		PublicDependencyModuleNames.AddRange(
			[
				"Core",
				"MeshDescription",
				"StaticMeshDescription",
				// ... add other public dependencies that you statically link with here ...
			]
		);


		PrivateDependencyModuleNames.AddRange(
			[
				"InputCore",
				"CoreUObject",
				"Engine",
				"CinematicCamera",
				"CommonTypes",
				"UnrealEd",
				"CinematicCamera",
				"ImageWrapper",
				"OpenCVForUnreal",
				// ... add private dependencies that you statically link with here ...	
			]
		);


		DynamicallyLoadedModuleNames.AddRange(
			[
				// ... add any modules that your module loads dynamically here ...
			]
		);
	}
}