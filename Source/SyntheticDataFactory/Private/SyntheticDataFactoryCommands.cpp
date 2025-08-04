// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyntheticDataFactoryCommands.h"

#define LOCTEXT_NAMESPACE "FSyntheticDataFactoryModule"

// 负责定义插件的命令。
void FSyntheticDataFactoryCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Synthetic Data Factory", "Use levelsequences to generate images or point cloud synthetic datasets", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
