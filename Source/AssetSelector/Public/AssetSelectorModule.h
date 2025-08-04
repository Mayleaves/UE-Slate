// AssetSelector/Public/AssetSelectorModule.h
// 模块类（FAssetSelectorModule）

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetSelectorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};