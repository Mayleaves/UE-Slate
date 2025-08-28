// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InteractButtons : ModuleRules
{
	public InteractButtons(ReadOnlyTargetRules Target) : base(Target)
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
				"LevelSequenceEditor",
				"MovieRenderPipelineCore",
				"CommonTypes",
				"LevelSequenceUtils",
				// ... add other public dependencies that you statically link with here ...
			]
		);


		PrivateDependencyModuleNames.AddRange(
			[
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"LevelSequence",
				"MovieRenderPipelineEditor",
				"EditorStyle",
				"PropertyEditor", 
				"TraceServices",
				"Sequencer", 
				"EditorScriptingUtilities",
				"CinematicCamera",
				"DesktopPlatform",
				"DesktopWidgets",
				"EditorWidgets",
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