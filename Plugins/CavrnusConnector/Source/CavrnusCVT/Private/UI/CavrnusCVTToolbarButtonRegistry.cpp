// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusCVTToolbarButtonRegistry.h"
#include "CavrnusConnector/Public/UI/Systems/Panels/Types/Toolbar/CavrnusToolbarPanelWidget.h"
#include "CavrnusConnector/Public/UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "CavrnusConnector/Public/Core/Subsystems/CavrnusSubsystem.h"
#include "CavrnusConnector/Public/AssetManager/CavrnusDataAssetManager.h"
#include "CavrnusConnector/Public/Core/Contexts/CavrnusRuntimeContext.h"
#include "DataAssets/CavrnusCVTToolbarButtonConfigAsset.h"
#include "CavrnusCVT.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

static TWeakObjectPtr<UCavrnusCVTToolbarButtonRegistry> GRegistryInstance;
static TFunction<void(UCavrnusCVTToolbarButtonRegistry*)> GFactoryRegistrationCallback;

void UCavrnusCVTToolbarButtonRegistry::SetFactoryRegistrationCallback(TFunction<void(UCavrnusCVTToolbarButtonRegistry*)> Callback)
{
	GFactoryRegistrationCallback = Callback;
}

void UCavrnusCVTToolbarButtonRegistry::Initialize()
{
    Super::Initialize();

	// Bind to the toolbar construction delegate
	ToolbarConstructedHandle = UCavrnusToolbarPanelWidget::OnToolbarConstructed.AddUObject(
		this, 
		&UCavrnusCVTToolbarButtonRegistry::OnToolbarConstructed
	);
	
	// Load configuration asset
	LoadConfigAsset();
	
	// Re-register button factories - this is needed because the registry gets recreated on each PIE session
	// and the factories map gets cleared in Dispose()
	if (GFactoryRegistrationCallback)
	{
		GFactoryRegistrationCallback(this);
	}
}

void UCavrnusCVTToolbarButtonRegistry::Dispose()
{
	if (ToolbarConstructedHandle.IsValid())
	{
		UCavrnusToolbarPanelWidget::OnToolbarConstructed.Remove(ToolbarConstructedHandle);
		ToolbarConstructedHandle.Reset();
	}
	
	ButtonFactories.Empty();
	DefaultInsertIndices.Empty();
	ConfigAsset = nullptr;
	GRegistryInstance.Reset();

    Super::Dispose();

}

void UCavrnusCVTToolbarButtonRegistry::RegisterButtonFactory(const FString& ButtonName, FToolbarButtonFactory Factory, int32 DefaultInsertIndex)
{
	ButtonFactories.Add(ButtonName, Factory);
	DefaultInsertIndices.Add(ButtonName, DefaultInsertIndex);
	UE_LOG(LogCavrnusCVT, Log, TEXT("Registered toolbar button factory: %s"), *ButtonName);
}

void UCavrnusCVTToolbarButtonRegistry::SetConfigAsset(UCavrnusCVTToolbarButtonConfigAsset* InConfigAsset)
{
	ConfigAsset = InConfigAsset;
}

void UCavrnusCVTToolbarButtonRegistry::LoadConfigAsset()
{
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (!Subsystem || !Subsystem->RuntimeContext)
	{
		UE_LOG(LogCavrnusCVT, Warning, TEXT("Failed to load toolbar button config: Subsystem not available"));
		return;
	}

	UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
	if (!DataAssetManager)
	{
		UE_LOG(LogCavrnusCVT, Warning, TEXT("Failed to load toolbar button config: DataAssetManager not available"));
		return;
	}

	// Try to get the config asset from the data asset manager
	ConfigAsset = DataAssetManager->GetAsset<UCavrnusCVTToolbarButtonConfigAsset>();
	
	if (!ConfigAsset)
	{
		// Create a default config programmatically if none is found
		// This allows the system to work out of the box
		UE_LOG(LogCavrnusCVT, Log, TEXT("Toolbar button config asset not found. Creating default config with bookmark enabled."));
		
		UWorld* World = GetWorld();
		if (World)
		{
			ConfigAsset = NewObject<UCavrnusCVTToolbarButtonConfigAsset>(World);
			
			// Add default bookmark button configuration
			FCavrnusToolbarButtonConfig BookmarkConfig;
			BookmarkConfig.ButtonName = TEXT("Bookmark");
			BookmarkConfig.InsertIndex = 0;
			BookmarkConfig.bEnabled = true;
			ConfigAsset->ButtonConfigs.Add(BookmarkConfig);
			
			UE_LOG(LogCavrnusCVT, Log, TEXT("Created default toolbar button config with bookmark enabled"));
		}
		else
		{
			UE_LOG(LogCavrnusCVT, Warning, TEXT("Cannot create default config: World not available"));
		}
	}
	else
	{
		UE_LOG(LogCavrnusCVT, Log, TEXT("Loaded toolbar button config asset"));
	}
}

void UCavrnusCVTToolbarButtonRegistry::OnToolbarConstructed(UCavrnusToolbarPanelWidget* ToolbarWidget)
{
	if (!ToolbarWidget)
		return;

	// If no config asset, don't create any buttons
	if (!ConfigAsset)
	{
		UE_LOG(LogCavrnusCVT, Verbose, TEXT("No config asset available, skipping toolbar button creation"));
		return;
	}

	// Get enabled button names in order
	TArray<FString> EnabledButtonNames = ConfigAsset->GetEnabledButtonNames();

	// Create buttons in the order specified by config
	for (const FString& ButtonName : EnabledButtonNames)
	{
		// Find the factory for this button
		FToolbarButtonFactory* FactoryPtr = ButtonFactories.Find(ButtonName);
		if (!FactoryPtr)
		{
			UE_LOG(LogCavrnusCVT, Warning, TEXT("No factory registered for button: %s"), *ButtonName);
			continue;
		}

		// Create the button using the factory
		FToolbarButtonFactory& Factory = *FactoryPtr;
		UCavrnusUIToggleButton* Button = Factory(ToolbarWidget);
		
		if (!Button)
		{
			UE_LOG(LogCavrnusCVT, Verbose, TEXT("Factory for button %s returned nullptr, skipping"), *ButtonName);
			continue;
		}

		// Get insert index from config, or use default
		int32 InsertIndex = ConfigAsset->GetInsertIndexForButton(ButtonName);
		if (InsertIndex == INDEX_NONE)
		{
			// Fall back to default insert index if config doesn't specify
			InsertIndex = DefaultInsertIndices.FindRef(ButtonName);
		}

		// Add button to toolbar
		ToolbarWidget->AddToolbarButton(Button, InsertIndex);
		UE_LOG(LogCavrnusCVT, Log, TEXT("Added toolbar button: %s at index %d"), *ButtonName, InsertIndex);
	}
}
