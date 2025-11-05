// Copyright (c) 2025 Cavrnus. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusBlueprintModule, Log, All);

class FCavrnusBlueprintModule final : public IModuleInterface
{
public:
	FCavrnusBlueprintModule();
	virtual ~FCavrnusBlueprintModule() override;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:

};