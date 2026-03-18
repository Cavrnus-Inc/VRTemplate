// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Bookmarks/CavrnusBookmarkListPanelWidget.h"
#include "Bookmarks/CavrnusBookmarkManager.h"
#include "CavrnusConnector/Public/Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnector/Public/Core/Contexts/CavrnusRuntimeContext.h"
#include "CavrnusConnector/Public/UI/CavrnusUI.h"
#include "UI/Systems/Dialogs/Types/CavrnusInputFieldDialog.h"
#include "UI/Systems/Dialogs/CavrnusDialogSystem.h"
#include "CavrnusConnector/Public/UI/Systems/CavrnusBaseUISystem.h"
#include "CavrnusCVT.h"

void UCavrnusBookmarkListPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get bookmark manager from service locator
    if (UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get())
    {
        if (Subsystem->RuntimeContext)
        {
            BookmarkManager = Subsystem->RuntimeContext->Get<UCavrnusBookmarkManager>();
        }
    }

    if (!BookmarkManager)
    {
        UE_LOG(LogCavrnusCVT, Error, TEXT("BookmarkManager not found! Cannot initialize bookmark panel."));
        return;
    }

    // Set header text
    if (HeaderTextBlock)
    {
        HeaderTextBlock->SetText(FText::FromString(TEXT("Bookmarks")));
    }

    // Setup Add button
    if (AddButton)
    {
        AddButton->SetButtonText(FText::FromString(TEXT("Add")));
        AddButtonHandle = AddButton->OnButtonClicked.AddWeakLambda(this, [this]()
        {
            OnAddButtonClicked();
        });
    }

    // Setup Delete button
    if (DeleteButton)
    {
        DeleteButton->SetButtonText(FText::FromString(TEXT("Delete")));
        DeleteButtonHandle = DeleteButton->OnButtonClicked.AddWeakLambda(this, [this]()
        {
            OnDeleteButtonClicked();
        });
    }

    // Initialize bookmark list
    InitializeBookmarkList();

    // Note: Bookmark selection callback should be registered externally by systems that need to respond to bookmark selection
    // The panel widget handles the UI interaction, but the actual callback execution is managed by the BookmarkManager

    // Populate list with existing bookmarks
    if (ListHandler)
    {
        const TArray<FCavrnusBookmarkData> ExistingBookmarks = BookmarkManager->GetAllBookmarks();
        ListHandler->AddItems(ExistingBookmarks);
    }
}

void UCavrnusBookmarkListPanelWidget::NativeDestruct()
{
    // Clean up button delegates
    if (AddButton && AddButtonHandle.IsValid())
    {
        AddButton->OnButtonClicked.Remove(AddButtonHandle);
        AddButtonHandle.Reset();
    }

    if (DeleteButton && DeleteButtonHandle.IsValid())
    {
        DeleteButton->OnButtonClicked.Remove(DeleteButtonHandle);
        DeleteButtonHandle.Reset();
    }

    // Teardown list handler
    if (ListHandler)
    {
        ListHandler->Teardown();
        ListHandler.Reset();
    }

    BookmarkManager = nullptr;

    Super::NativeDestruct();
}

void UCavrnusBookmarkListPanelWidget::OnAddButtonClicked()
{
    if (!BookmarkManager)
    {
        UE_LOG(LogCavrnusCVT, Warning, TEXT("BookmarkManager is null, cannot create bookmark"));
        return;
    }

    // Show input dialog to get bookmark name
    if (UCavrnusUISystems* UI = UCavrnusUI::Get(this))
    {
        UI->Dialogs()->Create<UCavrnusInputFieldDialog>()
            ->SetTitleText(TEXT("Create Bookmark"))
            ->SetBodyText(TEXT("Enter a name for the bookmark"))
            ->SetInputFieldHintText(TEXT("Bookmark Name"))
            ->SetDismissButton(TEXT("Cancel"))
            ->SetConfirmButton(TEXT("Create"), [this](const FString& SubmittedText)
            {
                if (!SubmittedText.IsEmpty() && BookmarkManager)
                {
                    // Create bookmark (manager will call metadata collector callback)
                    const FCavrnusBookmarkData NewBookmark = BookmarkManager->CreateBookmark(SubmittedText);
                    
                    // Add to list handler
                    if (ListHandler)
                    {
                        ListHandler->AddItem(NewBookmark);
                    }
                }
            });
    }
}

void UCavrnusBookmarkListPanelWidget::OnDeleteButtonClicked()
{
    // Toggle delete mode
    bIsDeleteMode = !bIsDeleteMode;
    UpdateDeleteButtonState();
}

void UCavrnusBookmarkListPanelWidget::OnBookmarkSelected(const FCavrnusBookmarkData& BookmarkData)
{
    if (!BookmarkManager)
    {
        return;
    }

    if (bIsDeleteMode)
    {
        // Delete the bookmark
        if (BookmarkManager->DeleteBookmark(BookmarkData.BookmarkId))
        {
            // Remove from list handler
            if (ListHandler)
            {
                ListHandler->RemoveItem(BookmarkData);
            }

            // Exit delete mode
            bIsDeleteMode = false;
            UpdateDeleteButtonState();
        }
    }
    else
    {
        // Trigger the manager's bookmark selected callback if registered
        if (BookmarkManager)
        {
            BookmarkManager->TriggerBookmarkSelected(BookmarkData);
        }
    }
}

void UCavrnusBookmarkListPanelWidget::InitializeBookmarkList()
{
    if (!BookmarkListContainer || !BookmarkListItemBlueprint)
    {
        UE_LOG(LogCavrnusCVT, Warning, TEXT("BookmarkListContainer or BookmarkListItemBlueprint is null, cannot initialize list"));
        return;
    }

    // Create list handler
    ListHandler = TCavrnusUIListHandler<FCavrnusBookmarkData>::Initialize(
        BookmarkListContainer,
        BookmarkListItemBlueprint,
        [](const FCavrnusBookmarkData& Data) { return Data.BookmarkId; }, // Key function
        [](const FCavrnusBookmarkData& A, const FCavrnusBookmarkData& B) // Sort predicate (newest first)
        {
            return A.CreatedTimestamp > B.CreatedTimestamp;
        }
    );

    // Setup widget creation callback
    ListHandler->OnWidgetCreated<UCavrnusBookmarkListItemWidget>([](UCavrnusBookmarkListItemWidget* Widget, const FCavrnusBookmarkData& Data)
    {
        Widget->Setup(Data);
    });

    // Setup item selection callback
    ListHandler->OnItemSelected([this](const FCavrnusBookmarkData& SelectedData)
    {
        OnBookmarkSelected(SelectedData);
    });
}

void UCavrnusBookmarkListPanelWidget::UpdateDeleteButtonState()
{
    if (!DeleteButton)
    {
        return;
    }

    // Update button text to indicate delete mode
    if (bIsDeleteMode)
    {
        DeleteButton->SetButtonText(FText::FromString(TEXT("Delete (Active)")));
        // Optionally change button style/color here to indicate active state
    }
    else
    {
        DeleteButton->SetButtonText(FText::FromString(TEXT("Delete")));
    }
}

