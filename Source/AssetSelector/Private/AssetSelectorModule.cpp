// AssetSelector/Private/AssetSelectorModule.cpp
#include "AssetSelectorModule.h"

#define LOCTEXT_NAMESPACE "FAssetSelectorModule"

void FAssetSelectorModule::StartupModule() {}
void FAssetSelectorModule::ShutdownModule() {}


#undef LOCTEXT_NAMESPACE

/*
 * 解决 Plugin 'SyntheticDataFactory' failed to load because module 'AssetSelector' could not be initialized successfully after it was loaded.
 * 原因 只创建了 Slate 控件，没有创建模块。还需要导出宏
 */
IMPLEMENT_MODULE(FAssetSelectorModule, AssetSelector); // 注册模块
