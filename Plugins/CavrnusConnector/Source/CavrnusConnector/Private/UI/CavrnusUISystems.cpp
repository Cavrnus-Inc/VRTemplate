// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusUISystems.h"

#include "CavrnusConnectorModule.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Modes/CavrnusModeManager.h"
#include "UI/CavrnusUIConfigAsset.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Styles/CavrnusUIThemeManager.h"
#include "UI/Systems/Dialogs/CavrnusDialogSystem.h"
#include "UI/Systems/Displayers/CavrnusDesktopCanvasWidgetDisplayer.h"
#include "UI/Systems/Loaders/CavrnusUILoaderSystem.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Panels/CavrnusPanelSystem.h"
#include "UI/Systems/Popups/Types/CavrnusPopupSystem.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"
#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"

class UCavrnusConfirmationDialog;
class UCavrnusServerStatusToastWidget;
class UCavrnusUIConfigAsset;

TSharedPtr<TSetting<bool>> UCavrnusUISystems::UIIsReadySetting = MakeShared<TSetting<bool>>();
void UCavrnusUISystems::Dispose()
{
	Super::Dispose();
	UIIsReadySetting->Set(false);
}

void UCavrnusUISystems::Initialize()
{
	Super::Initialize();
	// Load UIConfig from DataAssetManager instead of hardcoded path
	UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
	if (Subsystem && Subsystem->RuntimeContext)
	{
		UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
		if (DataAssetManager)
		{
			UCavrnusUIConfigAsset* LoadedConfig = DataAssetManager->GetAsset<UCavrnusUIConfigAsset>();
			if (LoadedConfig)
			{
				UIConfig = LoadedConfig;
			}
		}
	}

	// Fallback to hardcoded path if DataAssetManager doesn't have it (for backwards compatibility during transition)
	if (!UIConfig.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Falling back to hardcoded path for CavrnusUIConfig Asset"));
		UIConfig = FCavrnusWidgetFactory::LoadAssetFromPath<UCavrnusUIConfigAsset>("/Game/Cavrnus/DataAssets/CavrnusUIConfigDataAsset.CavrnusUIConfigDataAsset");
	}

	if (!UIConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load CavrnusUIConfig Asset"));
		return;
	}
				
	MenuLookup = UIConfig->MenuLookupObject.LoadSynchronous();
	if (!MenuLookup)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to load MenuLookupObject Asset"));
		return;
	}

	if (UIConfig == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusUISubsystem::Initialize] Failed to load MenuLookupObject Asset"));
		return;
	}

	if (GetWorld() == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusUISubsystem::Initialize] World is null!"));
		return;
	}

	// Viewport-dependent systems need the game viewport to exist.
	// During OnPreWorldInitialization the viewport hasn't been created yet,
	// so defer until it is available.
	SetupViewportDependentSystems();
}

void UCavrnusUISystems::SetupViewportDependentSystems()
{
	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[UISystems] Game viewport not yet available -- deferring UI initialization"));
		DeferredInitHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateWeakLambda(this, [this](float) -> bool
			{
				if (GEngine && GEngine->GameViewport)
				{
					SetupViewportDependentSystems();
					return false;
				}
				return true;
			}), 0.0f);
		return;
	}

	UE_LOG(LogCavrnusConnector, Log, TEXT("[UISystems] Game viewport available -- completing UI initialization"));

	ArbiterSystem = NewObject<UCavrnusUIArbiter>(GetWorld());
	ArbiterSystem->Initialize();
	AlsoDispose(ArbiterSystem.Get());

	CachedThemeManager = NewObject<UCavrnusUIThemeManager>(GetWorld());
	CachedThemeManager->Initialize();
	AlsoDispose(CachedThemeManager.Get());

	auto* CanvasDisplayer = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusDesktopCanvasWidgetDisplayer>(UIConfig->CanvasDisplayer, GetWorld());
	CanvasDisplayer->Setup(MenuLookup);

	GenericWidgetDisplayerSystem = NewObject<UCavrnusRawWidgetHost>(GetWorld());
	GenericWidgetDisplayerSystem->Initialize(GetWorld(), CanvasDisplayer, ArbiterSystem);
	AlsoDispose(GenericWidgetDisplayerSystem.Get());

	ScrimSystem = NewObject<UCavrnusUIScrimSystem>(GetWorld());
	ScrimSystem->Initialize(MenuLookup, CanvasDisplayer);
	AlsoDispose(ScrimSystem.Get());

	PanelSystem = NewObject<UCavrnusPanelSystem>(GetWorld());
	PanelSystem->Initialize(MenuLookup, CanvasDisplayer, ArbiterSystem);
	AlsoDispose(PanelSystem.Get());

	PopupSystem = NewObject<UCavrnusPopupSystem>(GetWorld());
	PopupSystem->Initialize(MenuLookup, CanvasDisplayer);
	AlsoDispose(PopupSystem.Get());

	// Scoped messages object is to hold eventual other types of Message systems
	ScopedMessages = NewObject<UCavrnusScopedMessages>(GetWorld());
	ScopedMessages->Initialize(MenuLookup, CanvasDisplayer);
	AlsoDispose(ScopedMessages.Get());

	LoaderSystem = NewObject<UCavrnusUILoaderSystem>(GetWorld());
	LoaderSystem->Initialize(MenuLookup, CanvasDisplayer);
	AlsoDispose(LoaderSystem.Get());

	DialogSystem = NewObject<UCavrnusDialogSystem>(GetWorld());
	DialogSystem->Initialize(MenuLookup, CanvasDisplayer);
	AlsoDispose(DialogSystem.Get());

	ScreenSystem = NewObject<UCavrnusScreenSystem>(GetWorld());
	ScreenSystem->Initialize(MenuLookup, CanvasDisplayer, ArbiterSystem);
	AlsoDispose(ScreenSystem.Get());

	UIIsReadySetting->Set(true);
}
