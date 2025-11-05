// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"

#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"
#include "UI/Systems/Loaders/CavrnusBaseUILoaderWidget.h"
#include "UI/Systems/Popups/CavrnusBasePopupWidget.h"
#include "UI/Systems/Screens/CavrnusBaseScreenWidget.h"
#include "UI/Systems/Scrims/CavrnusBaseScrimWidget.h"

class UCavrnusBaseToastMessageWidget;

TSubclassOf<UCavrnusBaseDialogWidget> UCavrnusWidgetBlueprintLookup::GetDialogBlueprint(const UClass* ClassType)
{
	for (const auto Widget : DialogBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetDialogBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBaseUILoaderWidget> UCavrnusWidgetBlueprintLookup::GetLoaderBlueprint(const UClass* ClassType)
{
	for (const auto Widget : LoaderBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget.Get();
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetLoaderBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBaseScrimWidget> UCavrnusWidgetBlueprintLookup::GetScrimBlueprint(const UClass* ClassType)
{
	for (const auto Widget : ScrimBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget.Get();
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetScrimBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBaseToastMessageWidget> UCavrnusWidgetBlueprintLookup::GetToastMessageBlueprint(const UClass* ClassType)
{
	for (const auto Widget : ToastMessageBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetToastMessageBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBaseUIContainer> UCavrnusWidgetBlueprintLookup::GetContainerBlueprint(const UClass* ClassType)
{
	for (const auto Widget : ContainerBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetContainerBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBasePanelWidget> UCavrnusWidgetBlueprintLookup::GetPanelBlueprint(const UClass* ClassType)
{
	for (const auto Widget : PanelBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetPanelBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBaseScreenWidget> UCavrnusWidgetBlueprintLookup::GetScreenBlueprint(const UClass* ClassType)
{
	for (const auto Widget : ScreenBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget;
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetScreenBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}

TSubclassOf<UCavrnusBasePopupWidget> UCavrnusWidgetBlueprintLookup::GetPopupBlueprint(const UClass* ClassType)
{
	for (const auto Widget : PopupBlueprints)
	{
		if (Widget->IsChildOf(ClassType))
			return Widget.Get();
	}
	
	UE_LOG(LogTemp, Error, TEXT("[UCavrnusWidgetBlueprintLookup::GetPopupBlueprint] ClassType did not provide any results in lookup!"))
	return nullptr;
}
