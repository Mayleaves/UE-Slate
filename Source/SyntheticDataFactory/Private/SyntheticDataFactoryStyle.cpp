// Copyright Epic Games, Inc. All Rights Reserved.
/**
 * 负责管理插件的样式，包括样式的初始化、关闭、图标设置和纹理重新加载。
 */

#include "SyntheticDataFactoryStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSyntheticDataFactoryStyle::StyleInstance = nullptr;

void FSyntheticDataFactoryStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSyntheticDataFactoryStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSyntheticDataFactoryStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SyntheticDataFactoryStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FSyntheticDataFactoryStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("SyntheticDataFactoryStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SyntheticDataFactory")->GetBaseDir() / TEXT("Resources"));
	// Assigns an icon to the button.
	Style->Set("SyntheticDataFactory.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("AIsdg"), Icon20x20));

	return Style;
}

void FSyntheticDataFactoryStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSyntheticDataFactoryStyle::Get()
{
	return *StyleInstance;
}
