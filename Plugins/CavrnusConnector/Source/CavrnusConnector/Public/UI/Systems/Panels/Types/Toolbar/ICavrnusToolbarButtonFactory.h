// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ICavrnusToolbarButtonFactory.generated.h"

class UCavrnusToolbarPanelWidget;
class UCavrnusUIToggleButton;

/**
 * Interface for mode/tool classes to create their own toolbar buttons.
 * Each mode class should implement this interface to provide button creation logic.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UCavrnusToolbarButtonFactory : public UInterface
{
	GENERATED_BODY()
};

class CAVRNUSCONNECTOR_API ICavrnusToolbarButtonFactory
{
	GENERATED_BODY()

public:
	/**
	 * Creates a toolbar button for this mode/tool.
	 * @param ToolbarWidget The toolbar widget that will contain the button
	 * @return The created button, or nullptr if button should not be created
	 */
	virtual UCavrnusUIToggleButton* CreateToolbarButton(UCavrnusToolbarPanelWidget* ToolbarWidget) = 0;

	/**
	 * Gets the unique identifier for this button (used for configuration lookup).
	 * @return Button name/ID string
	 */
	virtual FString GetButtonName() const = 0;

	/**
	 * Gets the default insert index for this button.
	 * @return Insert index, or INDEX_NONE to append at end
	 */
	virtual int32 GetDefaultInsertIndex() const = 0;
};

