// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusUIConfigAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/Popups/Types/CavrnusPopupSystem.h"
#include "CavrnusUISubsystem.generated.h"

class UCavrnusUIArbiter;
class UCavrnusPanelSystem;
class UCavrnusRawWidgetHost;
class UCavrnusUIScrimSystem;
class UCavrnusUILoaderSystem;
class UCavrnusUIThemeManager;
class UCavrnusDialogSystem;
class UCavrnusPopupSystem;
class UCavrnusScopedMessages;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UCavrnusUIArbiter* Arbiter() const { return ArbiterSystem; }
	UCavrnusRawWidgetHost* GenericWidgetDisplayer() const { return GenericWidgetDisplayerSystem; }
	UCavrnusUIScrimSystem* Scrims() const { return ScrimSystem; }
	UCavrnusPanelSystem* Panels() const { return PanelSystem; }
	UCavrnusPopupSystem* Popups() const { return PopupSystem; }
	UCavrnusDialogSystem* Dialogs() const { return DialogSystem; }
	UCavrnusScreenSystem* Screens() const { return ScreenSystem; }
	UCavrnusUILoaderSystem* Loaders() const { return LoaderSystem; }
	UCavrnusScopedMessages* Messages() const { return ScopedMessages; }

	UCavrnusUIThemeManager* ThemeManager() const { return CachedThemeManager; }

	static void AwaitUIInit(const TFunction<void()>& OnCallback);
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Cavrnus|UiConfigAsset")
	TSoftObjectPtr<UCavrnusUIConfigAsset> UIConfig = nullptr;
	
	static bool HasUIInit;
	static TMulticastDelegate<void()> OnUIInit;
	
	UPROPERTY()
	TObjectPtr<UCavrnusUIThemeManager> CachedThemeManager = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusRawWidgetHost> GenericWidgetDisplayerSystem = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusPanelSystem> PanelSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusUIScrimSystem> ScrimSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusPopupSystem> PopupSystem = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusUILoaderSystem> LoaderSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusDialogSystem> DialogSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusScopedMessages> ScopedMessages = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusUIArbiter> ArbiterSystem = nullptr;

	UPROPERTY()
	UCavrnusScreenSystem* ScreenSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> MenuLookup;
};