// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "CavrnusCVTToolbarButtonConfigAsset.generated.h"

/**
 * Configuration entry for a single toolbar button.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCVT_API FCavrnusToolbarButtonConfig
{
	GENERATED_BODY()

	// Unique identifier for the button (must match the button name from the mode class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config")
	FString ButtonName;

	// Insert position in the toolbar (INDEX_NONE to append at end)
	// Lower values appear first in the toolbar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config")
	int32 InsertIndex = INDEX_NONE;

	// Whether this button is enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button Config")
	bool bEnabled = true;
};

/**
 * Data asset that defines which toolbar buttons are enabled and their ordering.
 * This allows selective enablement of buttons from the CVT module.
 */
UCLASS(BlueprintType)
class CAVRNUSCVT_API UCavrnusCVTToolbarButtonConfigAsset : public UCavrnusBaseDataAsset
{
	GENERATED_BODY()

public:
	// Array of button configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toolbar Configuration")
	TArray<FCavrnusToolbarButtonConfig> ButtonConfigs;

	/**
	 * Gets the insert index for a button by name.
	 * @param ButtonName The name of the button
	 * @return The insert index, or INDEX_NONE if not found or disabled
	 */
	int32 GetInsertIndexForButton(const FString& ButtonName) const;

	/**
	 * Checks if a button is enabled.
	 * @param ButtonName The name of the button
	 * @return True if the button is enabled, false otherwise
	 */
	bool IsButtonEnabled(const FString& ButtonName) const;

	/**
	 * Gets all enabled button names in order of their insert indices.
	 * @return Array of enabled button names, sorted by insert index
	 */
	TArray<FString> GetEnabledButtonNames() const;
};

