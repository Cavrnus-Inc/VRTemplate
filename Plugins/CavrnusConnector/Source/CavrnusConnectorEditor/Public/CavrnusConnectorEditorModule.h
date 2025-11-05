// Copyright (c) 2025 Cavrnus. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Tools/CavrnusEditorToolsManager.h"
#include "UI/CavrnusEditorUIManager.h"
#include "IAssetTypeActions.h"
#include "AssetTypeActions_Base.h"
#include "Engine/World.h"
#include "Editor.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCavrnusConnectorEditor, Log, All);

class USCS_Node;
class UCavrnusValueSyncBase;

class FCavrnusConnectorEditorModule final : public IModuleInterface
{
public:
	FCavrnusConnectorEditorModule();
	virtual ~FCavrnusConnectorEditorModule() override;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:

	TStrongObjectPtr<UCavrnusEditorUIManager>         EditorUI;
	TStrongObjectPtr<UCavrnusEditorToolsManager>      EditorTools;

	TArray<TSharedPtr<IAssetTypeActions>> RegisteredOverrides;
};