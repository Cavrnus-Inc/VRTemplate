// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Toolbar/CavrnusToolbarEntriesController.h"

#include "UI/Tabs/CavrnusEditorTabController.h"
#include "Utilities/CavrnusEditorHelpers.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "CavrnusConnectorEditor"

void UCavrnusToolbarEntriesController::Initialize()
{
	const TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateUObject(this, &UCavrnusToolbarEntriesController::AddParentToolbarEntry)
	);

	FCavrnusEditorHelpers::GetLevelEditorModule().GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void UCavrnusToolbarEntriesController::Teardown()
{
}

void UCavrnusToolbarEntriesController::AddParentToolbarEntry(FMenuBarBuilder& MenuBuilder)
{
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("MenuLocKey", "Cavrnus"),
		LOCTEXT("MenuTooltipKey", "Opens menu for CavrnusConnector plugin"),
		FNewMenuDelegate::CreateUObject(this, &UCavrnusToolbarEntriesController::AddCavrnusSubEntries),
		FName(TEXT("Cavrnus")),
		FName(TEXT("Cavrnus Help Menu")));
}

void UCavrnusToolbarEntriesController::AddCavrnusSubEntries(FMenuBuilder& MenuBuilder)
{
	// Add other sub entries here!!
	
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Welcome")),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]
		{
			UCavrnusEditorTabController::ShowCavrnusTab();
		})),
		NAME_None,
		EUserInterfaceActionType::Button
	);

	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Run Tests")),
		FText::FromString(TEXT("Run all Cavrnus automation tests and log results")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]
		{
			GEngine->Exec(nullptr, TEXT("Automation RunTests Cavrnus"));
		})),
		NAME_None,
		EUserInterfaceActionType::Button
	);
}
#undef LOCTEXT_NAMESPACE