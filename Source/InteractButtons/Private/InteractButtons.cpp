#include "InteractButtons.h"

#define LOCTEXT_NAMESPACE "InteractButtonsNamespace"

void FInteractButtonsModule::StartupModule(){}  // 模块初始化
void FInteractButtonsModule::ShutdownModule(){}  // 模块卸载

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInteractButtonsModule, InteractButtons);