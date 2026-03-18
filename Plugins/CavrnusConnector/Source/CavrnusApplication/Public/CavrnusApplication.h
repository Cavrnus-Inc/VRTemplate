// Copyright (c) 2025 Cavrnus. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusApplication, Log, All);

class FCavrnusApplication final : public IModuleInterface
{
public:
	FCavrnusApplication();
	virtual ~FCavrnusApplication() override;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:

};