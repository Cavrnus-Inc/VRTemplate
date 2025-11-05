// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusEditorUIManager.h"

#include "UI/Styles/CavrnusUIThemeAsset.h"
#include "UI/Styles/CavrnusUIThemeManager.h"
#include "UI/Toolbar/CavrnusToolbarEntriesController.h"

void UCavrnusEditorUIManager::Initialize()
{
	TabController = NewObject<UCavrnusEditorTabController>();
	TabController->Initialize();

	ToolbarController = NewObject<UCavrnusToolbarEntriesController>(GetTransientPackage());
	ToolbarController->Initialize();

	const auto ThemeAssetPath = TEXT("/CavrnusConnector/UI/Styles/DA_Cavrnus_ThemeAsset.DA_Cavrnus_ThemeAsset");
	if (UCavrnusUIThemeAsset* ThemeAsset = LoadObject<UCavrnusUIThemeAsset>(nullptr, ThemeAssetPath))
		UCavrnusUIThemeManager::SetEditorPreviewTheme(ThemeAsset);
	else
		UE_LOG(LogTemp, Warning, TEXT("Failed to load default theme asset for editor preview!"));
}

void UCavrnusEditorUIManager::Teardown()
{
	if (TabController)
		TabController->Teardown();
	if (ToolbarController)
		ToolbarController->Teardown();
	
	TabController = nullptr;
	ToolbarController = nullptr;
}
