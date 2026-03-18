// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusUI.h"
#include "CoreMinimal.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "UI/CavrnusUISystems.h"
#include "UI/Systems/Panels/CavrnusPanelSystem.h"
#include "UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"

UCavrnusUISystems* UCavrnusUI::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        if (GEngine && GEngine->GameViewport)
        {
            if (UGameViewportClient* ViewportClient = GEngine->GameViewport.Get())
                WorldContextObject = ViewportClient->GetWorld();
        }
        
        if (!WorldContextObject)
        {
            UE_LOG(LogTemp, Verbose, TEXT("CavrnusUI::Get: GameViewport not ready yet, returning nullptr."));
            return nullptr;
        }
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid UWorld found!"));
        return nullptr;
    }

    return UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusUISystems>();
}

UCavrnusToolbarPanelWidget* UCavrnusUI::CreateToolbarPanel(UObject* WorldContextObject, EPanelLocation Location)
{
    UCavrnusUISystems* UISystems = Get(WorldContextObject);
    if (!UISystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateToolbarPanel: Failed to get UISystems"));
        return nullptr;
    }

    UCavrnusPanelSystem* PanelSystem = UISystems->Panels();
    if (!PanelSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateToolbarPanel: PanelSystem is not available"));
        return nullptr;
    }

    // Check if UI system is ready
    if (UCavrnusUISystems::UIIsReadySetting && !UCavrnusUISystems::UIIsReadySetting->Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateToolbarPanel: UI system is not ready yet"));
        return nullptr;
    }

    // Create the toolbar panel with the specified location
    FCavrnusPanelOptions Options = FCavrnusPanelOptions::SetLocation(Location);
    UCavrnusToolbarPanelWidget* ToolbarPanel = PanelSystem->Create<UCavrnusToolbarPanelWidget>(Options);
    
    if (!ToolbarPanel)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateToolbarPanel: Failed to create toolbar panel. Make sure WBP_Cavrnus_ToolbarPanelWidget is registered in the WidgetBlueprintLookup data asset."));
    }

    return ToolbarPanel;
}
