// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Loaders/CavrnusUILoaderSystem.h"

#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Loaders/CavrnusBaseUILoaderWidget.h"
#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"
#include "UI/Systems/Scrims/Types/CavrnusUIScrim.h"

void UCavrnusUILoaderSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	Lookup = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
}

void UCavrnusUILoaderSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (WidgetToClose == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusDialogSystem::Close] WidgetToClose is null!"))
		return;
	}

	if (auto* FoundDialog = Cast<UCavrnusBaseUILoaderWidget>(WidgetToClose))
	{
		Displayer->RemoveWidget(FoundDialog->GetId());
		
		Loaders.Remove(FoundDialog);
		LoaderMap.Remove(FoundDialog->GetId());
	}
}

void UCavrnusUILoaderSystem::CloseAll()
{
	if (Loaders.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusUILoaderSystem::CloseAll] There are currently no loaders open!"))
		return;
	}

	Displayer->RemoveAll();
	
	Loaders.Empty();
	LoaderMap.Empty();
}

void UCavrnusUILoaderSystem::Teardown()
{
	Displayer->RemoveAll();
}

UCavrnusBaseUILoaderWidget* UCavrnusUILoaderSystem::CreateInternal(const UClass* Type, const FCavrnusUILoaderOptions& Options)
{
	CloseAll(); // enforcing one open dialog at a time!
	
	const auto FoundBlueprint = Lookup->GetLoaderBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (UCavrnusBaseUILoaderWidget* LoaderWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBaseUILoaderWidget>(FoundBlueprint, GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		LoaderWidget->SetId(Id);

		const auto Scrim = UCavrnusUI::Get(this)->Scrims()->Create<UCavrnusUIScrim>();
		Scrim->HookOnClose([this, LoaderWidget] { Close(LoaderWidget); });
		
		LoaderWidget->HookOnClose([this, LoaderWidget, Scrim]
		{
			Close(LoaderWidget);
			if (Scrim)
				Scrim->CloseScrim();
		});

		LoaderMap.Add(Id, LoaderWidget);
		Displayer->DisplayLoaderWidget(LoaderWidget, Options, [this](UCavrnusBaseUILoaderWidget* WidgetToClose)
		{
			if (WidgetToClose)
				Close(WidgetToClose);
		});
		
		return LoaderWidget;
	}

	return nullptr;
}
