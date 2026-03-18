// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnector/Public/Modes/CavrnusModeBase.h"
#include "CavrnusBookmarkMode.generated.h"

class UCavrnusToolbarPanelWidget;
class UCavrnusUIToggleButton;
class UCavrnusBookmarkListPanelWidget;

UCLASS()
class CAVRNUSCVT_API UCavrnusBookmarkMode : public UCavrnusModeBase
{
    GENERATED_BODY()

public:
    virtual void EnterMode(UWorld* World, const int32 Priority = 0) override;
    virtual void ExitMode() override;

    /**
     * Creates a toolbar button for the bookmark mode.
     * @param ToolbarWidget The toolbar widget that will contain the button
     * @return The created button, or nullptr if button should not be created
     */
    static UCavrnusUIToggleButton* CreateToolbarButton(UCavrnusToolbarPanelWidget* ToolbarWidget);

    /**
     * Gets the unique identifier for the bookmark button.
     * @return Button name string
     */
    static FString GetButtonName() { return TEXT("Bookmark"); }

    /**
     * Gets the default insert index for the bookmark button.
     * @return Insert index (0 for first position)
     */
    static int32 GetDefaultInsertIndex() { return 0; }

protected:
    virtual FString GetInputMappingContextName() override { return TEXT("bookmarkContext"); }
    virtual void BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data) override {}

private:
    /** Reference to the bookmark panel widget */
    UPROPERTY()
    TObjectPtr<UCavrnusBookmarkListPanelWidget> BookmarkPanel = nullptr;
};