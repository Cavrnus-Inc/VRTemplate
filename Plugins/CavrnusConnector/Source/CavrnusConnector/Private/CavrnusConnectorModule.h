// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * @brief Declares a log category for the Cavrnus Connector module.
 * Used for logging messages related to the Cavrnus Spatial Connector plugin.
 */
DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusConnector, Log, All);

/**
 * @class FCavrnusConnectorModule
 * @brief Manages the lifecycle of the Cavrnus Spatial Connector module.
 *
 * This class is responsible for initializing and shutting down the Cavrnus Spatial Connector plugin.
 */
class CAVRNUSCONNECTOR_API FCavrnusConnectorModule : public IModuleInterface
{
private:

public:

	// Called when module is loaded in memory
	FCavrnusConnectorModule();
	~FCavrnusConnectorModule() override;

	/** IModuleInterface implementation */
	// Called when module is ready to start
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

/**
 * @brief Defines a static log category for the Cavrnus Relay module.
 *
 * This log category is used for logging messages specifically related to the relay functionality within the Cavrnus Spatial Connector plugin.
 */
DEFINE_LOG_CATEGORY_STATIC(CavrnusRelayModule, Log, All);
