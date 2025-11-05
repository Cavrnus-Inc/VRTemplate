// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"

#include "CavrnusSubsystem.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusIconsDataAsset.h"
#include "Modes/CavrnusExploreMode.h"
#include "Modes/CavrnusModeManager.h"
#include "Modes/CavrnusSceneCaptureMode.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

void UCavrnusToolbarPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ModeManager = UCavrnusSubsystem::Get()->Services->Get<UCavrnusModeManager>();
	
	const auto Icons = UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusIconsDataAsset>();

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
}

void UCavrnusToolbarPanelWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	ModeDelegate.Reset();
}
