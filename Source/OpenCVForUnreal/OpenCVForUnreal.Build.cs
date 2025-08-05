using System.IO;
using UnrealBuildTool;

public class OpenCVForUnreal : ModuleRules
{
	public OpenCVForUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 添加必要的依赖模块
		PublicIncludePaths.AddRange(
			[
				// ... add public include paths required here ...
			]
		);
		
		PrivateIncludePaths.AddRange(
			[
				// ... add private include paths required here ...
			]
		);
		
		PublicDependencyModuleNames.AddRange(
			[
				"Core" // 底层核心功能
				// ... add public dependencies that you statically link with here ...
			]
		);
		
		PrivateDependencyModuleNames.AddRange(
			[
				"Projects",  // IPluginManager 接口
				"InputCore", // 输入系统：键鼠
				"EditorFramework",
				"UnrealEd", // 编辑器核心功能
				"ToolMenus",
				"CoreUObject", // 反射
				"Engine", // 运行时核心功能
				"Slate",  // UI 框架
				"SlateCore" // Slate 的基础样式和渲染逻辑
				// ... add private dependencies that you statically link with here ...	
			]
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			[
				// ... add any modules that your module loads dynamically here ...
			]
		);
		
		#region Load OpenCV
		// 链接 OpenCV 库
		{
			var IncPath = Path.Combine(ModuleDirectory, "../../ThirdParty/OpenCV/includes");
			var LibPath = Path.Combine(ModuleDirectory, "../../ThirdParty/OpenCV/Libs");
			const string LibName = "opencv_world4120";  // 版本号 4.12.0

			PublicSystemIncludePaths.Add(IncPath);
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, LibName + ".lib"));  // 静态库
			const string DllName = LibName + ".dll";  // 动态库
			PublicDelayLoadDLLs.Add(DllName);

			var PlatformDir = Target.Platform.ToString();  // Windows64 系统为 "Win64"
			var BinaryPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries", PlatformDir));
			RuntimeDependencies.Add(Path.Combine(BinaryPath, DllName)); 

			PublicDefinitions.Add("WITH_OPENCV=1");
		}

		#endregion
	}
}