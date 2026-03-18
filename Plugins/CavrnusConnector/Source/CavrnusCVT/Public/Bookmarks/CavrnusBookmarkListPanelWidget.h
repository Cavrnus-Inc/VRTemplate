// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/CavrnusBasePanelWidget.h"
#include "CavrnusConnector/Public/UI/Components/Lists/CavrnusListContainerWidget.h"
#include "CavrnusConnector/Public/UI/Components/Lists/CavrnusUIListHandler.h"
#include "CavrnusConnector/Public/UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "CavrnusConnector/Public/UI/Components/Text/CavrnusUITextBlock.h"
#include "Bookmarks/CavrnusBookmarkData.h"
#include "Bookmarks/CavrnusBookmarkListItemWidget.h"
#include "CavrnusBookmarkListPanelWidget.generated.h"

class UCavrnusBookmarkManager;

/**
 * @brief Main panel widget for displaying and managing bookmarks.
 * 
 * This widget contains a header, Add/Delete buttons, and a list of bookmarks.
 * It handles bookmark creation, deletion, and selection.
 */
UCLASS(Abstract)
class CAVRNUSCVT_API UCavrnusBookmarkListPanelWidget : public UCavrnusBasePanelWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    /** Header text block displaying "Bookmarks" */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusUITextBlock> HeaderTextBlock = nullptr;

    /** Add button for creating new bookmarks */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusUITextButton> AddButton = nullptr;

    /** Delete button for entering delete mode */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusUITextButton> DeleteButton = nullptr;

    /** List container for displaying bookmarks */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusListContainerWidget> BookmarkListContainer = nullptr;

    /** Blueprint class for bookmark list item widget */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|Bookmark")
    TSubclassOf<UCavrnusBookmarkListItemWidget> BookmarkListItemBlueprint = nullptr;

private:
    /** Reference to the bookmark manager service */
    UPROPERTY()
    TObjectPtr<UCavrnusBookmarkManager> BookmarkManager = nullptr;

    /** List handler for managing bookmark list */
    TUniquePtr<TCavrnusUIListHandler<FCavrnusBookmarkData>> ListHandler;

    /** Whether delete mode is currently active */
    bool bIsDeleteMode = false;

    /** Delegate handles for button clicks */
    FDelegateHandle AddButtonHandle = FDelegateHandle();
    FDelegateHandle DeleteButtonHandle = FDelegateHandle();

    /**
     * Handle Add button click - shows input dialog to create new bookmark
     */
    void OnAddButtonClicked();

    /**
     * Handle Delete button click - toggles delete mode
     */
    void OnDeleteButtonClicked();

    /**
     * Handle bookmark selection - either selects or deletes based on delete mode
     * @param BookmarkData The bookmark that was selected
     */
    void OnBookmarkSelected(const FCavrnusBookmarkData& BookmarkData);

    /**
     * Initialize the bookmark list using TCavrnusUIListHandler
     */
    void InitializeBookmarkList();

    /**
     * Update the visual state of the delete button to indicate delete mode
     */
    void UpdateDeleteButtonState();
};

