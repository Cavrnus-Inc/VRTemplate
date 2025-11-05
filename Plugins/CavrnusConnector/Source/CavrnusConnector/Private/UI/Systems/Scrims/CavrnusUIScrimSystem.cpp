// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"

#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Scrims/CavrnusBaseScrimWidget.h"

class UCavrnusUI;

void UCavrnusUIScrimSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	Lookup = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
}

void UCavrnusUIScrimSystem::CloseWidgetScrim(const FGuid& WidgetId)
{
	if (const auto* FoundScrimPtr = WidgetToScrimsMap.Find(WidgetId))
	{
		if (const auto FoundScrimPtrPtr = ScrimsMap.Find(*FoundScrimPtr))
			Close(*FoundScrimPtrPtr);

		WidgetToScrimsMap.Remove(*FoundScrimPtr);
		ScrimsMap.Remove(*FoundScrimPtr);
	}
}

void UCavrnusUIScrimSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (WidgetToClose == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusUIScrimSystem::Close] WidgetToClose is null!"))
		return;
	}

	if (auto* FoundScrim = Cast<UCavrnusBaseScrimWidget>(WidgetToClose))
	{
		Displayer->RemoveWidget(FoundScrim->GetId());
		
		Scrims.Remove(FoundScrim);
		ScrimsMap.Remove(FoundScrim->GetId());
	}
}

void UCavrnusUIScrimSystem::CloseAll()
{
	if (Scrims.IsEmpty())
	{
		// UE_LOG(LogTemp, Error, TEXT("[UCavrnusUILoaderSystem::CloseAll] There are currently no Scrims open!"))
		return;
	}

	Displayer->RemoveAll();
	
	Scrims.Empty();
	ScrimsMap.Empty();
}

void UCavrnusUIScrimSystem::Teardown()
{
	Displayer->RemoveAll();
}

UCavrnusBaseScrimWidget* UCavrnusUIScrimSystem::CreateInternal(const UClass* Type, const FCavrnusScrimOptions& Options)
{
	CloseAll();
	
	const auto FoundBlueprint = Lookup->GetScrimBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (UCavrnusBaseScrimWidget* ScrimWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBaseScrimWidget>(FoundBlueprint, GetWorld()))
	{
		ScrimWidget->InitializeScrim(Options);
		
		const auto Id = FGuid::NewGuid();
		ScrimWidget->SetId(Id);
		ScrimWidget->HookOnClose([this, ScrimWidget] { Close(ScrimWidget); });
		
		ScrimsMap.Add(Id, ScrimWidget);
		Displayer->DisplayScrimWidget(ScrimWidget, Options, [this](UCavrnusBaseScrimWidget* WidgetToClose)
		{
			if (WidgetToClose)
				Close(WidgetToClose);
		});
		
		return ScrimWidget;
	}

	return nullptr;
}

UCavrnusBaseScrimWidget* UCavrnusUIScrimSystem::AssignToWidgetInternal(
	const UClass* Type,
	const FGuid WidgetId,
	const FCavrnusScrimOptions& Options)
{
	const auto Scrim = CreateInternal(Type, Options);
	if (Scrim)
	{
		Scrim->HookOnClose([this, WidgetId]
		{
			CloseWidgetScrim(WidgetId);
			Displayer->RemoveWidget(WidgetId);
		});

		WidgetToScrimsMap.Add(WidgetId, Scrim->GetId());
	}

	return Scrim;
}
