// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Dialogs/CavrnusDialogSystem.h"

#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"
#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"
#include "UI/Systems/Scrims/Types/CavrnusUIScrim.h"

void UCavrnusDialogSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	Lookup = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
}

void UCavrnusDialogSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (WidgetToClose == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusDialogSystem::Close] WidgetToClose is null!"))
		return;
	}

	if (auto* FoundDialog = Cast<UCavrnusBaseDialogWidget>(WidgetToClose))
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusDialogSystem::Close] CLOSING DIALOG!"))
		
		UCavrnusUI::Get(this)->Scrims()->CloseWidgetScrim(WidgetToClose->GetId());
		
		Displayer->RemoveWidget(FoundDialog->GetId());
		
		Dialogs.Remove(FoundDialog);
		DialogMap.Remove(FoundDialog->GetId());
	}
}

void UCavrnusDialogSystem::CloseAll()
{
	if (Dialogs.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusDialogSystem::CloseAll] There are currently no dialogs open!"))
		return;
	}

	for (UCavrnusBaseDialogWidget* Dialog : Dialogs)
		UCavrnusUI::Get(this)->Scrims()->Close(Dialog);

	Displayer->RemoveAll();
	
	Dialogs.Empty();
	DialogMap.Empty();
}

void UCavrnusDialogSystem::Teardown()
{
	Displayer->RemoveAll();
}

UCavrnusBaseDialogWidget* UCavrnusDialogSystem::CreateInternal(const UClass* Type, const FCavrnusDialogOptions& Options)
{
	CloseAll(); // enforcing one open dialog at a time!
	
	const auto FoundBlueprint = Lookup->GetDialogBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (UCavrnusBaseDialogWidget* DialogWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBaseDialogWidget>(FoundBlueprint, GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		DialogWidget->SetId(Id);

		DialogWidget->HookOnClose([this, DialogWidget] { Close(DialogWidget); });
		
		DialogMap.Add(Id, DialogWidget);

		FCavrnusScrimOptions ScrimOps = FCavrnusScrimOptions();
		ScrimOps.bCloseOnClick = false;
		ScrimOps.ZOrder = 99;
		
		UCavrnusUI::Get(this)->Scrims()->AssignToWidget<UCavrnusUIScrim>(Id, ScrimOps);
		
		Displayer->DisplayDialogWidget(DialogWidget, Options, [this](UCavrnusBaseDialogWidget* WidgetToClose)
		{
			if (WidgetToClose)
				Close(WidgetToClose);
		});

		return DialogWidget;
	}

	return nullptr;
}
