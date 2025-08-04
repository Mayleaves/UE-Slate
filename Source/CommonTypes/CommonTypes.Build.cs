using UnrealBuildTool;

public class CommonTypes : ModuleRules
{
	public CommonTypes(ReadOnlyTargetRules Target) : base(Target)
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
				"CoreUObject",
				"Engine",
				// ... add other public dependencies that you statically link with here ...
			]
		);


		PrivateDependencyModuleNames.AddRange(
			[
				"Projects",
				"InputCore", // 输入系统：键鼠
				"EditorFramework",
				"UnrealEd", // 编辑器核心功能
				"ToolMenus",
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