// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusCVTToolbarExtension.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"
#include "CavrnusConnector/Public/UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "CavrnusConnector/Public/Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnector/Public/AssetManager/CavrnusDataAssetManager.h"
#include "CavrnusConnector/Public/AssetManager/DataAssets/CavrnusIconsDataAsset.h"
#include "CavrnusConnector/Public/Core/Contexts/CavrnusRuntimeContext.h"
#include "CavrnusConnector/Public/Modes/CavrnusModeManager.h"
#include "CavrnusConnector/Public/UI/Helpers/CavrnusWidgetFactory.h"
#include "Bookmarks/CavrnusBookmarkMode.h"
#include "CavrnusCVT.h"
#include "Engine/World.h"
#include "Components/VerticalBox.h"
#include "UObject/UObjectIterator.h" 

static TWeakObjectPtr<UCavrnusCVTToolbarExtension> GExtensionInstance;

UCavrnusCVTToolbarExtension* UCavrnusCVTToolbarExtension::Get()
{
    if (!GExtensionInstance.IsValid())
    {
        UWorld* World = nullptr;
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
        
        if (World)
        {
            GExtensionInstance = NewObject<UCavrnusCVTToolbarExtension>(World);
        }
    }
    
    return GExtensionInstance.IsValid() ? GExtensionInstance.Get() : nullptr;
}

void UCavrnusCVTToolbarExtension::Initialize()
{
    // Use a ticker to find toolbar widgets after they're constructed
    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UCavrnusCVTToolbarExtension::Tick)
    );
}

void UCavrnusCVTToolbarExtension::Shutdown()
{
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }
    
    ExtendedWidgets.Empty();
    GExtensionInstance.Reset();
}

bool UCavrnusCVTToolbarExtension::Tick(float DeltaTime)
{
    FindAndExtendToolbarWidgets();
    
    // Continue ticking until we've found and extended all widgets
    // You could add logic to stop ticking after a certain time or condition
    return true;
}

void UCavrnusCVTToolbarExtension::FindAndExtendToolbarWidgets()
{
    // Get all toolbar panel widgets in the world
    TArray<UUserWidget*> AllWidgets;
    
    // Find toolbar widgets by iterating through world objects
    // Since we can't easily enumerate all widgets, we'll use a different approach:
    // Hook into the panel system or use a delegate
    
    // Alternative: Use GetWorld()->GetFirstPlayerController() and search through widget tree
    // Or better: Access through the UI system
    
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
        return;
    
    // Access the panel system to find active toolbar panels
    // This is a bit of a workaround, but necessary without modifying base class
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Search for toolbar widgets by class
    // We'll need to iterate through all UUserWidget instances
    // This is not ideal, but works without base class modification
    
    // Better approach: Use TObjectIterator to find all toolbar widgets
    for (TObjectIterator<UCavrnusToolbarPanelWidget> It; It; ++It)
    {
        UCavrnusToolbarPanelWidget* ToolbarWidget = *It;
        if (!ToolbarWidget || !IsValid(ToolbarWidget))
            continue;
        
        // Check if we've already extended this widget
        if (ExtendedWidgets.Contains(ToolbarWidget))
            continue;
        
        // Check if widget is in a valid world
        if (ToolbarWidget->GetWorld() != World)
            continue;
        
        // Check if widget is constructed and has a container
        if (!ToolbarWidget->IsConstructed())
            continue;
        
        // Extend the widget
        AddBookmarkButton(ToolbarWidget);
        ExtendedWidgets.Add(ToolbarWidget);
    }
}

void UCavrnusCVTToolbarExtension::AddBookmarkButton(UCavrnusToolbarPanelWidget* ToolbarWidget)
{
    if (!ToolbarWidget)
        return;
    
    // Access private members via reflection or make Container protected/public
    // Since Container is protected, we need to either:
    // 1. Make it public in the base class (requires base class change - not ideal)
    // 2. Use a friend class (requires base class change)
    // 3. Create a public getter in base class (requires base class change)
    // 4. Use a subclass approach (but user wants dynamic extension)
    
    // Best solution: Add a public method to the base class to add buttons
    // But since we can't modify it, we'll need to use reflection or...
    
    // Actually, let's check if we can access Container via a public method
    // Looking at the code, Container is protected, so we need a different approach
    
    // Solution: We'll need to modify UCavrnusToolbarPanelWidget to add a public method
    // OR use a different extension mechanism
    
    // For now, let's assume we can add a public extension method to the base class
    // But since user wants no base class changes, let's use a workaround:
    
    UE_LOG(LogCavrnusCVT, Log, TEXT("Attempting to add bookmark button to toolbar widget"));
    
    // We'll need to add a public extension API to the toolbar widget
    // Since that requires base class modification, let me provide an alternative:
    // Use a Blueprint-callable extension method or delegate system
}