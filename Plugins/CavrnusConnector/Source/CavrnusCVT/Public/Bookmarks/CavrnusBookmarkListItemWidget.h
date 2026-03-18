// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Lists/CavrnusBaseListItemWidget.h"
#include "Bookmarks/CavrnusBookmarkData.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusBookmarkListItemWidget.generated.h"

/**
 * @brief Widget for displaying a single bookmark entry in the bookmark list.
 * 
 * This widget displays bookmark information including name and optional timestamp.
 * It extends UCavrnusBaseListItemWidget to integrate with the list container system.
 */
UCLASS(Abstract)
class CAVRNUSCVT_API UCavrnusBookmarkListItemWidget : public UCavrnusBaseListItemWidget
{
    GENERATED_BODY()

public:
    /**
     * Setup the widget with bookmark data.
     * @param InBookmarkData The bookmark data to display
     */
    void Setup(const FCavrnusBookmarkData& InBookmarkData);

protected:
    /** Bookmark name text block */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusUITextBlock> BookmarkNameTextBlock = nullptr;

    /** Timestamp text block (optional) */
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UCavrnusUITextBlock> TimestampTextBlock = nullptr;

    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual void NativeDestruct() override;

private:
    /** Stored bookmark data */
    UPROPERTY()
    FCavrnusBookmarkData BookmarkData = FCavrnusBookmarkData();

    /** Delegate handle for button click */
    FDelegateHandle ButtonDelegate = FDelegateHandle();
};

