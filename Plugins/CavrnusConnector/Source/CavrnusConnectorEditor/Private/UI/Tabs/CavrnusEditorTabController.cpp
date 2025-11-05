// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Tabs/CavrnusEditorTabController.h"

#include "EditorUtilityWidgetBlueprint.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Framework/Docking/LayoutExtender.h"
#include "UI/Widgets/CavrnusMainEditorPanelWidget.h"
#include "Utilities/CavrnusEditorHelpers.h"

FOnMainEditorWidgetSpawned UCavrnusEditorTabController::OnMainEditorWidgetSpawned;

FName UCavrnusEditorTabController::TabId = "CavrnusPanelWidgetID";
void UCavrnusEditorTabController::Initialize()
{
	if (FGlobalTabmanager::Get()->HasTabSpawner(TabId))
		return;

	FGlobalTabmanager::Get()
	->RegisterNomadTabSpawner(
		TabId,
		FOnSpawnTab::CreateUObject(this, &UCavrnusEditorTabController::HandleSpawnTab))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
	.SetDisplayName(FText::FromString(TEXT("Cavrnus")))
	.SetTooltipText(FText::FromString(TEXT("Cavrnus")));
	
	FLevelEditorModule* pLevelEditorModule =
	FModuleManager::GetModulePtr<FLevelEditorModule>(
		FName(TEXT("LevelEditor")));
	
	pLevelEditorModule->OnRegisterLayoutExtensions().AddLambda([](FLayoutExtender& Extender) {
		Extender.ExtendLayout(
			FTabId("PlacementBrowser"),
			ELayoutExtensionPosition::After,
			FTabManager::FTab(TabId, ETabState::OpenedTab));
	});
}

void UCavrnusEditorTabController::Teardown()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("CavrnusPanelWidgetID");
}

TSharedRef<SDockTab> UCavrnusEditorTabController::HandleSpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	const FString BlueprintPath = TEXT("/CavrnusConnector/UI/EditorMenus/CavrnusMainEditorPanel/EUW_Cavrnus_MainEditorPanelWidget");

	UEditorUtilityWidgetBlueprint* WidgetBP = LoadObject<UEditorUtilityWidgetBlueprint>(
		nullptr,  TEXT("/CavrnusConnector/UI/EditorMenus/CavrnusMainEditorPanel/EUW_Cavrnus_MainEditorPanelWidget")
	);

	if (!WidgetBP)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load EditorUtilityWidgetBlueprint!"));
		return SNew(SDockTab);
	}

	return WidgetBP->SpawnEditorUITab(TabSpawnArgs);
}

void UCavrnusEditorTabController::ShowCavrnusTab()
{
	if (!FGlobalTabmanager::Get()->HasTabSpawner(TabId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cavrnus tab not registered. Call Initialize() first."));
		return;
	}

	FGlobalTabmanager::Get()->TryInvokeTab(TabId);
}
