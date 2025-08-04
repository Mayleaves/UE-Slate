// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyntheticDataFactory : ModuleRules
{
	public SyntheticDataFactory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"MovieRenderPipelineCore", // 渲染管线
				"LevelSequence", // 基础Sequencer模块
				"LevelSequenceEditor", // 编辑器相关功能（如需）
				"MovieScene", // FFrameRate依赖
				"MovieSceneTools", // 可能的额外依赖
				"CoreUObject",
				"Engine",
				"InputCore",
				"Sequencer",
				"UnrealEd",
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AssetSelector", // todo 测试用，可删
				"InteractButtons",
				"UnrealEd", // 编辑器相关模块
				"Sequencer" // 直接依赖Sequencer模块
				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}