// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Popups/Types/CavrnusPopupSystem.h"

#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UI/Systems/Popups/CavrnusBasePopupWidget.h"

void UCavrnusPopupSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	LookupAsset = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
	
	Popups.Empty();
	PopupsMap.Empty();
}

void UCavrnusPopupSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (auto* Popup = Cast<UCavrnusBasePopupWidget>(WidgetToClose))
	{
		Popups.Remove(Popup);
		PopupsMap.Remove(Popup->GetId());
		Displayer->RemoveWidget(WidgetToClose->GetId());
	}
}

void UCavrnusPopupSystem::CloseAll()
{
	if (Popups.IsEmpty())
		return;
	
	Popups.Empty();
	PopupsMap.Empty();
}

void UCavrnusPopupSystem::Teardown()
{
	if (Displayer)
		Displayer->RemoveAll();

	Displayer = nullptr;
}

UCavrnusBasePopupWidget* UCavrnusPopupSystem::CreateInternal(const UClass* Type, const FCavrnusPopupOptions& Options, const FUIContextData& Data)
{
	CloseAll();

	const auto FoundBlueprint = LookupAsset->GetPopupBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (auto* PopupInstance = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBasePopupWidget>(FoundBlueprint, GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		PopupInstance->SetId(Id);
		PopupInstance->SetContextData(Data);
		
		Popups.Add(PopupInstance);
		PopupsMap.Add(Id, PopupInstance);

		Displayer->DisplayPopupWidget(PopupInstance, Options,[this](UCavrnusBasePopupWidget* PopupToClose)
		{
			Close(PopupToClose);
		});
		
		return PopupInstance;
	}
	
	return nullptr;
}
