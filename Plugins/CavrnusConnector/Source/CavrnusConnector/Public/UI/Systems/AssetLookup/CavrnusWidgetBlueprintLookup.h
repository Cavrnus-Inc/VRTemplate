// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/Containers/CavrnusBaseUIContainer.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusBaseToastMessageWidget.h"
#include "UI/Systems/Panels/CavrnusBasePanelWidget.h"
#include "CavrnusWidgetBlueprintLookup.generated.h"

class UCavrnusBaseScreenWidget;
class UCavrnusBaseScrimWidget;
class UCavrnusBaseUILoaderWidget;
class UCavrnusBaseUIContainer;
class UCavrnusBaseDialogWidget;
class UCavrnusBasePopupWidget;
class FCavrnusWidgetFactory;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusWidgetBlueprintLookup : public UDataAsset
{
	GENERATED_BODY()
public:
	TSubclassOf<UCavrnusBasePanelWidget> GetPanelBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBaseScreenWidget> GetScreenBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBasePopupWidget> GetPopupBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBaseDialogWidget> GetDialogBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBaseUILoaderWidget> GetLoaderBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBaseScrimWidget> GetScrimBlueprint(const UClass* ClassType);
	TSubclassOf<UCavrnusBaseToastMessageWidget> GetToastMessageBlueprint(const UClass* ClassType);

	TSubclassOf<UCavrnusBaseUIContainer> GetContainerBlueprint(const UClass* ClassType);

protected:
	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseUIContainer>> ContainerBlueprints;

	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBasePanelWidget>> PanelBlueprints;

	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseScreenWidget>> ScreenBlueprints;
	
	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBasePopupWidget>> PopupBlueprints;
	
	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseDialogWidget>> DialogBlueprints;
	
	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseToastMessageWidget>> ToastMessageBlueprints;

	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseUILoaderWidget>> LoaderBlueprints;
	
	UPROPERTY(EditAnywhere, Category="Cavrnus|WidgetLookup")
	TArray<TSubclassOf<UCavrnusBaseScrimWidget>> ScrimBlueprints;
};