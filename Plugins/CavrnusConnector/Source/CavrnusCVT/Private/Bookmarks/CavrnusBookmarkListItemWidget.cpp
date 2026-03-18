// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Bookmarks/CavrnusBookmarkListItemWidget.h"
#include "CavrnusConnector/Public/UI/Components/Lists/CavrnusListViewDataObject.h"
#include "CavrnusConnector/Public/UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusCVT.h"

void UCavrnusBookmarkListItemWidget::Setup(const FCavrnusBookmarkData& InBookmarkData)
{
    BookmarkData = InBookmarkData;

    // Set bookmark name
    if (BookmarkNameTextBlock)
    {
        BookmarkNameTextBlock->SetText(FText::FromString(BookmarkData.Name));
    }

    // Set timestamp if text block exists
    if (TimestampTextBlock)
    {
        if (BookmarkData.CreatedTimestamp.GetTicks() > 0)
        {
            const FString DateString = BookmarkData.CreatedTimestamp.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
            TimestampTextBlock->SetText(FText::FromString(DateString));
        }
        else
        {
            TimestampTextBlock->SetText(FText::GetEmpty());
        }
    }

    // Bind button click to trigger list item selection
    if (Button)
    {
        if (ButtonDelegate.IsValid())
        {
            Button->OnButtonClicked.Remove(ButtonDelegate);
        }
        
        ButtonDelegate = Button->OnButtonClicked.AddWeakLambda(this, [this]()
        {
            ListViewSetSelectedItem();
        });
    }
}

void UCavrnusBookmarkListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    Super::NativeOnListItemObjectSet(ListItemObject);

    // The list handler will call Setup() via OnWidgetCreated callback
    // This method is called when the item is bound to the data object
}

void UCavrnusBookmarkListItemWidget::NativeDestruct()
{
    // Clean up button delegate binding
    if (Button && ButtonDelegate.IsValid())
    {
        Button->OnButtonClicked.Remove(ButtonDelegate);
        ButtonDelegate.Reset();
    }

    Super::NativeDestruct();
}

