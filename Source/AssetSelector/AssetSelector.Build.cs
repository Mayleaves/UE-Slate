// AssetSelector.Build.cs

using UnrealBuildTool;

public class AssetSelector : ModuleRules
{
	public AssetSelector(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDefinitions.Add("ASSETSELECTOR_EXPORTS"); // 关键：定义导出宏。一般不用手动添加
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 添加必要的依赖模块
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add private include paths required here ...
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", // 底层核心功能
				// ... add public dependencies that you statically link with here ...
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore", // 输入系统：键鼠
				"EditorFramework",
				"UnrealEd", // 编辑器核心功能
				"ToolMenus",
				"CoreUObject", // 反射
				"Engine", // 运行时核心功能
				"Slate",  // UI 框架
				"SlateCore", // Slate 的基础样式和渲染逻辑
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