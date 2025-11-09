#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomComputeShadersModule : public IModuleInterface {
public:
	virtual void StartupModule() override;
};