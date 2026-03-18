// Copyright (c) 2025 Cavrnus. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "EdGraphUtilities.h"
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusBlueprintModule, Log, All);

class FCavrnusBlueprintModule final : public IModuleInterface
{
public:
	FCavrnusBlueprintModule();
	virtual ~FCavrnusBlueprintModule() override;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	/** Keep a reference so we can unregister cleanly */
#if WITH_EDITOR
	TSharedPtr<FGraphPanelPinFactory> PinFactory;
#endif
};