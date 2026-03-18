// Copyright (c) 2025 Cavrnus. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusCVT, Log, All);

class FCavrnusCVT final : public IModuleInterface
{
public:
	FCavrnusCVT();
	virtual ~FCavrnusCVT() override;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	/**
	 * Attempts to register toolbar button factories.
	 * Returns true if successful, false if RuntimeContext is not ready yet.
	 */
	bool TryRegisterToolbarButtonFactories();
	
	void MountCustomContentFolder();
	FTSTicker::FDelegateHandle TickerHandle;
};