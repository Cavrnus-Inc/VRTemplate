// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Bookmarks/CavrnusBookmarkMode.h"
#include "Bookmarks/CavrnusBookmarkListPanelWidget.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"
#include "CavrnusConnector/Public/UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "CavrnusConnector/Public/Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnector/Public/AssetManager/CavrnusDataAssetManager.h"
#include "CavrnusConnector/Public/AssetManager/DataAssets/CavrnusIconsDataAsset.h"
#include "CavrnusConnector/Public/Core/Contexts/CavrnusRuntimeContext.h"
#include "CavrnusConnector/Public/Modes/CavrnusModeManager.h"
#include "CavrnusConnector/Public/UI/Helpers/CavrnusWidgetFactory.h"
#include "CavrnusConnector/Public/UI/CavrnusUI.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/CavrnusPanelLocation.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/CavrnusPanelSystem.h"
#include "CavrnusCVT.h"
#include "Engine/World.h"

void UCavrnusBookmarkMode::EnterMode(UWorld* World, const int32 Priority)
{
    Super::EnterMode(World, Priority);
    
    // Create and show bookmark panel
    if (UCavrnusUISystems* UI = UCavrnusUI::Get(World))
    {
        BookmarkPanel = UI->Panels()->Create<UCavrnusBookmarkListPanelWidget>(
            FCavrnusPanelOptions::SetLocation(EPanelLocation::RightMiddle)
        );
    }
    
    UE_LOG(LogCavrnusCVT, Log, TEXT("Bookmark Mode Entered"));
}

void UCavrnusBookmarkMode::ExitMode()
{
    Super::ExitMode();
    
    // Close bookmark panel
    UCavrnusUISystems* UI = UCavrnusUI::Get();
    if (BookmarkPanel && UI)
    {
        UI->Panels()->Close(BookmarkPanel);
        BookmarkPanel = nullptr;
    }
    
    UE_LOG(LogCavrnusCVT, Log, TEXT("Bookmark Mode Exited"));
}

UCavrnusUIToggleButton* UCavrnusBookmarkMode::CreateToolbarButton(UCavrnusToolbarPanelWidget* ToolbarWidget)
{
    if (!ToolbarWidget)
        return nullptr;

    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
        return nullptr;

    UCavrnusModeManager* ModeManager = Subsystem->RuntimeContext->Get<UCavrnusModeManager>();
    if (!ModeManager)
        return nullptr;

    UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
    if (!DataAssetManager)
        return nullptr;

    UCavrnusIconsDataAsset* Icons = DataAssetManager->GetAsset<UCavrnusIconsDataAsset>();
    if (!Icons)
        return nullptr;

    // Create the button
    UCavrnusUIToggleButton* BookmarkButton = FCavrnusWidgetFactory::CreateUserWidget(
        ToolbarWidget->GetToolToggleButtonBlueprint(), 
        ToolbarWidget->GetWorld()
    );
    
    if (!BookmarkButton)
        return nullptr;

    // Set up bookmark mode state tracking
    auto InBookmarkMode = ModeManager->CurrentActiveMode.Translating(
        [](UClass* const& Mode) { return Mode == UCavrnusBookmarkMode::StaticClass(); });

    BookmarkButton->InitToggleSetting(InBookmarkMode);

    UTexture2D* BookmarkIcon = Icons->GetIcon("bookmark");
    if (BookmarkIcon)
    {
        BookmarkButton->SetIcon(BookmarkIcon);
    }
    else
    {
        UE_LOG(LogCavrnusCVT, Warning, TEXT("Bookmark icon 'bookmark' not found in IconsDataAsset"));
    }

    BookmarkButton->SetCanToggleFunc([ModeManager] { 
        return !ModeManager->IsCurrentModeOfType<UCavrnusBookmarkMode>(); 
    });

    BookmarkButton->OnToggleChanged.AddLambda([ModeManager, ToolbarWidget](const bool bVal)
    {
        if (bVal)
        {
            ModeManager->PushTransientMode<UCavrnusBookmarkMode>(ToolbarWidget->GetWorld());
        }
        else
        {
            ModeManager->PopTransientMode();
        }
    });

    return BookmarkButton;
}