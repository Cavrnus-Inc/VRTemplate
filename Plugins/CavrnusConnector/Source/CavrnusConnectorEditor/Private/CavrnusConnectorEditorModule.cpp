// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusConnectorEditorModule.h"

#include "ToolMenus.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Utilities/CavrnusEditorHelpers.h"
#include "Utilities/CavrnusAssetActionsRegistry.h"
#include "Utilities/CavrnusGenericAssetActions.h"

#define LOCTEXT_NAMESPACE "CavrnusConnectorEditor"

IMPLEMENT_MODULE(FCavrnusConnectorEditorModule, CavrnusConnectorEditor)
DEFINE_LOG_CATEGORY(LogCavrnusConnectorEditor);

FCavrnusConnectorEditorModule::FCavrnusConnectorEditorModule() {}
FCavrnusConnectorEditorModule::~FCavrnusConnectorEditorModule() {}

#if WITH_EDITOR
TSharedPtr<IAssetTypeActions> FindOriginalActionsForClass(UClass* ClassType)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// UE5.0: returns a single weak ptr (the highest-priority action for this class)
	const TWeakPtr<IAssetTypeActions> Weak = AssetTools.GetAssetTypeActionsForClass(ClassType);
	return Weak.Pin(); // may be null if none registered yet
}
#endif

void FCavrnusConnectorEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	
	EditorUI = TStrongObjectPtr(NewObject<UCavrnusEditorUIManager>(GetTransientPackage()));
	EditorUI->Initialize();
	
	// Set up for the Cavrnus menu editor content menus
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// List of classes you want to override
	TArray<UClass*> TargetClasses = {
		UStaticMesh::StaticClass(),
		UBlueprint::StaticClass(),
		UMaterial::StaticClass(),
		// ... add more here
	};

#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.AddLambda([]()
		{
			FCavrnusAssetActionsRegistry::RegisterOverrides();
		});
#endif
}

void FCavrnusConnectorEditorModule::ShutdownModule()
{
	// Cleanup from the Asset Editor Remapping
#if WITH_EDITOR
	// Only try to unregister if AssetTools is still around
	FCavrnusAssetActionsRegistry::UnregisterOverrides();
	RegisteredOverrides.Empty();
#endif

	if (EditorUI.IsValid())		EditorUI->Teardown();
	if (EditorTools.IsValid())	EditorTools->Teardown();

	// Drop strong references; GC can collect afterward.
	EditorUI.Reset();
	EditorTools.Reset();

	IModuleInterface::ShutdownModule();

}

#undef LOCTEXT_NAMESPACE