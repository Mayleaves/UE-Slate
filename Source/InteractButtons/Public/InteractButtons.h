#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FInteractButtonsModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;  // 在模块加载时调用
	virtual void ShutdownModule() override;  // 在模块卸载时调用
};