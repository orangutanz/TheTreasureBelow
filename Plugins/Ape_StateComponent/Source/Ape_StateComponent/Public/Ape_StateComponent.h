#pragma once

#include "Modules/ModuleManager.h"

class FApe_StateComponentModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};