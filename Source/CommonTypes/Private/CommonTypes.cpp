#include "CommonTypes.h"

#define LOCTEXT_NAMESPACE "CommonTypesNamespace"

void FCommonTypesModule::StartupModule()
{
} // 模块初始化
void FCommonTypesModule::ShutdownModule()
{
} // 模块卸载

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCommonTypesModule, CommonTypes)
