// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusConnectorEditorModule.h"
#include "CavrnusConnector/Public/CavrnusLog.h"

#include "ToolMenus.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Utilities/CavrnusEditorHelpers.h"
#include "Utilities/CavrnusContentBrowserExtender.h"
#include "PropertyEditorModule.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Utilities/SActorOptimizationEditorPanel.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Settings/ProjectPackagingSettings.h"

#define LOCTEXT_NAMESPACE "CavrnusConnectorEditor"

IMPLEMENT_MODULE(FCavrnusConnectorEditorModule, CavrnusConnectorEditor)
DEFINE_LOG_CATEGORY(LogCavrnusConnectorEditor);

static const FName ActorOptimizationTabName("ActorOptimizationTab");

FCavrnusConnectorEditorModule::FCavrnusConnectorEditorModule() {}
FCavrnusConnectorEditorModule::~FCavrnusConnectorEditorModule() {}

void FCavrnusConnectorEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusConnectorEditor module active."));

	EditorUI = TStrongObjectPtr(NewObject<UCavrnusEditorUIManager>(GetTransientPackage()));
	EditorUI->Initialize();

    RegisterTabs();

	FCavrnusContentBrowserExtender::Register();

	// Ensure Cavrnus DataAssets are included in cooked builds (in-memory only, no SaveConfig)
	if (UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>())
	{
		const FDirectoryPath CavrnusDataAssetsDir { TEXT("/Game/Cavrnus/DataAssets") };
		const bool bAlreadyPresent = PackagingSettings->DirectoriesToAlwaysCook.ContainsByPredicate(
			[&](const FDirectoryPath& Dir) { return Dir.Path == CavrnusDataAssetsDir.Path; });
		if (!bAlreadyPresent)
		{
			PackagingSettings->DirectoriesToAlwaysCook.Add(CavrnusDataAssetsDir);
		}
	}
}

void FCavrnusConnectorEditorModule::ShutdownModule()
{
	FCavrnusContentBrowserExtender::Unregister();

    UnregisterTabs();

	if (EditorUI.IsValid())		EditorUI->Teardown();
	if (EditorTools.IsValid())	EditorTools->Teardown();

	EditorUI.Reset();
	EditorTools.Reset();

	IModuleInterface::ShutdownModule();
}

void FCavrnusConnectorEditorModule::RegisterTabs()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ActorOptimizationTabName,
		FOnSpawnTab::CreateRaw(this, &FCavrnusConnectorEditorModule::SpawnActorOptimizationTab))
		.SetDisplayName(FText::FromString(TEXT("Actor Optimization")))
		.SetMenuType(ETabSpawnerMenuType::Enabled);
}

void FCavrnusConnectorEditorModule::UnregisterTabs()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ActorOptimizationTabName);
}

TSharedRef<SDockTab> FCavrnusConnectorEditorModule::SpawnActorOptimizationTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SActorOptimizationEditorPanel)
        ];
}
#undef LOCTEXT_NAMESPACE
