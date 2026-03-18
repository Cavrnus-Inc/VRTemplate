// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnector/Public/Managers/CavrnusService.h"

// Forward declarations
class UCavrnusToolbarPanelWidget;
class UCavrnusUIToggleButton;
class UCavrnusCVTToolbarButtonConfigAsset;

#include "CavrnusCVTToolbarButtonRegistry.generated.h"

/**
 * Type alias for button factory functions.
 * Factory functions take a toolbar widget and return a button, or nullptr if creation should be skipped.
 */
using FToolbarButtonFactory = TFunction<UCavrnusUIToggleButton*(UCavrnusToolbarPanelWidget*)>;

/**
 * Registry for CVT toolbar buttons.
 * Coordinates button registration and creation based on configuration.
 * Registered as an optional service in the RuntimeContext.
 */
UCLASS()
class CAVRNUSCVT_API UCavrnusCVTToolbarButtonRegistry : public UCavrnusService
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;
	virtual void Dispose() override;
	
	/**
	 * Registers a button factory function.
	 * Called by module startup code to register button creation functions.
	 * @param ButtonName Unique identifier for the button (must match config)
	 * @param Factory Function that creates the button
	 * @param DefaultInsertIndex Default insert index if not specified in config
	 */
	void RegisterButtonFactory(const FString& ButtonName, FToolbarButtonFactory Factory, int32 DefaultInsertIndex = INDEX_NONE);

	/**
	 * Sets the configuration asset to use for button enablement.
	 * @param ConfigAsset The configuration data asset
	 */
	void SetConfigAsset(UCavrnusCVTToolbarButtonConfigAsset* ConfigAsset);

	/**
	 * Sets a callback function that will be called to register all button factories.
	 * This allows factories to be re-registered when the registry is reinitialized (e.g., on PIE restart).
	 * @param Callback Function that registers all button factories
	 */
	static void SetFactoryRegistrationCallback(TFunction<void(UCavrnusCVTToolbarButtonRegistry*)> Callback);

private:
	void OnToolbarConstructed(UCavrnusToolbarPanelWidget* ToolbarWidget);
	void LoadConfigAsset();
	
	// Map of button names to factory functions
	TMap<FString, FToolbarButtonFactory> ButtonFactories;
	
	// Map of button names to default insert indices
	TMap<FString, int32> DefaultInsertIndices;
	
	// Configuration asset
	UPROPERTY()
	TObjectPtr<UCavrnusCVTToolbarButtonConfigAsset> ConfigAsset;
	
	FDelegateHandle ToolbarConstructedHandle;

};