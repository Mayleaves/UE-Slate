// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenCVForUnreal.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FOpenCVForUnrealModule"

void FOpenCVForUnrealModule::StartupModule()
{
	// 这里的文本名称要是 .uplugin 文件的名称
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("SyntheticDataFactory"))->GetBaseDir();
	FString LibraryPath;

#if WITH_OPENCV
	LibraryPath = FPaths::Combine(*PluginDir, TEXT("ThirdParty/OpenCV/Libs/"));
	OpenCV_World_Handler = FPlatformProcess::GetDllHandle(*(LibraryPath + TEXT("opencv_world4120.dll")));  // 加载 OpenCV 句柄
#endif
}

void FOpenCVForUnrealModule::ShutdownModule()
{
#if WITH_OPENCV
	if (OpenCV_World_Handler)
	{
		FPlatformProcess::FreeDllHandle(OpenCV_World_Handler);  // 释放 OpenCV 句柄
		OpenCV_World_Handler = nullptr;
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOpenCVForUnrealModule, OpenCVForUnreal)