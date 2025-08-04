// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "SyntheticDataFactoryStyle.h"

/**
 * 命令绑定类，继承自 TCommands，用于定义和注册插件的 UI 命令（如菜单、按钮）。
 */
class FSyntheticDataFactoryCommands : public TCommands<FSyntheticDataFactoryCommands>
{
public:

	FSyntheticDataFactoryCommands()
		: TCommands<FSyntheticDataFactoryCommands>(TEXT("SyntheticDataFactory"), NSLOCTEXT("Contexts", "SyntheticDataFactory", "SyntheticDataFactory Plugin"), NAME_None, FSyntheticDataFactoryStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};