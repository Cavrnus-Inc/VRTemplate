// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"

#include "Core/Subsystems/CavrnusSubsystem.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusIconsDataAsset.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Modes/CavrnusExploreMode.h"
#include "Modes/CavrnusModeManager.h"
#include "Modes/CavrnusSceneCaptureMode.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "Components/PanelWidget.h"  // Add this include
#include "Runtime/Launch/Resources/Version.h"  // For ENGINE_MAJOR_VERSION

// Initialize static delegate
FOnToolbarConstructed UCavrnusToolbarPanelWidget::OnToolbarConstructed;

void UCavrnusToolbarPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ModeManager = UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusModeManager>();

    const auto Icons = UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusIconsDataAsset>();

    auto InExploreMode = ModeManager->CurrentActiveMode.Translating(
        [](UClass* const& Mode) { return Mode == UCavrnusExploreMode::StaticClass(); });

    auto InCapMode = ModeManager->CurrentActiveMode.Translating(
        [](UClass* const& Mode) { return Mode == UCavrnusSceneCaptureMode::StaticClass(); });

    ExploreButton = FCavrnusWidgetFactory::CreateUserWidget(ToolToggleButtonBlueprint, GetWorld());
    ExploreButton->InitToggleSetting(InExploreMode);
    ExploreButton->SetIcon(Icons->GetIcon("explore"));
    ExploreButton->SetCanToggleFunc([this] { return !ModeManager->IsCurrentModeOfType<UCavrnusExploreMode>(); });
    ExploreButton->OnToggleChanged.AddLambda([this](const bool bVal)
        {
            if (bVal)
                ModeManager->SetExplicitMode<UCavrnusExploreMode>(GetWorld());
        });

    SceneCaptureButton = FCavrnusWidgetFactory::CreateUserWidget(ToolToggleButtonBlueprint, GetWorld());
    SceneCaptureButton->InitToggleSetting(InCapMode);
    SceneCaptureButton->SetIcon(Icons->GetIcon("camera"));
    SceneCaptureButton->SetCanToggleFunc([this] { return !ModeManager->IsCurrentModeOfType<UCavrnusSceneCaptureMode>(); });
    SceneCaptureButton->OnToggleChanged.AddLambda([this](const bool bVal)
        {
            if (bVal)
                ModeManager->PushTransientMode<UCavrnusSceneCaptureMode>(GetWorld());
        });

    if (Container)
    {
        Container->AddChildToVerticalBox(ExploreButton);
        Container->AddChildToVerticalBox(SceneCaptureButton);
    }

    // Broadcast to allow modules to extend this toolbar
    OnToolbarConstructed.Broadcast(this);
}

void UCavrnusToolbarPanelWidget::AddToolbarButton(UCavrnusUIToggleButton* Button, int32 InsertIndex)
{
    if (!Button || !Container)
        return;

    if (InsertIndex == INDEX_NONE || InsertIndex >= Container->GetChildrenCount())
    {
        Container->AddChildToVerticalBox(Button);
    }
    else
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
        // UE 5.6+: Use InsertChildAt from UPanelWidget base class
        if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Container))
        {
            PanelWidget->InsertChildAt(InsertIndex, Button);
        }
        else
        {
            // Fallback if cast fails
            Container->AddChildToVerticalBox(Button);
        }
#else
        // UE 5.0-5.5: Rebuild children list to insert at specific index
        TArray<UWidget*> ExistingChildren;
        for (int32 i = 0; i < Container->GetChildrenCount(); ++i)
        {
            if (UWidget* Child = Container->GetChildAt(i))
            {
                ExistingChildren.Add(Child);
            }
        }

        Container->ClearChildren();

        for (int32 i = 0; i < ExistingChildren.Num(); ++i)
        {
            if (i == InsertIndex)
            {
                Container->AddChildToVerticalBox(Button);
            }
            Container->AddChildToVerticalBox(ExistingChildren[i]);
        }

        if (InsertIndex == ExistingChildren.Num())
        {
            Container->AddChildToVerticalBox(Button);
        }
#endif
    }
}

void UCavrnusToolbarPanelWidget::NativeDestruct()
{
    Super::NativeDestruct();

    ModeDelegate.Reset();
}