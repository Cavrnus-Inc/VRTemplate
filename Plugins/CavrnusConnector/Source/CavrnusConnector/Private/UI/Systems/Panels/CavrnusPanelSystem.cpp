// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Panels/CavrnusPanelSystem.h"

#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"

void UCavrnusPanelSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer, UCavrnusUIArbiter* Arbiter)
{
	LookupAsset = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;

	Arbiter->CurrentUIVisMode->Bind(this, [this](const EUIVisibilityMode& Mode)
	{
		switch (Mode)
		{
		case EUIVisibilityMode::Normal:
			SetVisibility(true);
			break;
		case EUIVisibilityMode::ScreenExclusive:
			SetVisibility(false);
			break;
		case EUIVisibilityMode::NoUI:
			SetVisibility(false);
			break;
		}
	});
}

void UCavrnusPanelSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (const auto* Panel = Cast<UCavrnusBasePanelWidget>(WidgetToClose))
	{
		ActivePanels.Remove(Panel->GetId());
		Displayer->RemoveWidget(WidgetToClose->GetId());
	}
}

void UCavrnusPanelSystem::CloseAll()
{
	// Collect all panels first to avoid modifying the map during iteration
	TArray<UCavrnusBasePanelWidget*> PanelsToClose;
	ActivePanels.GenerateValueArray(PanelsToClose);
	
	// Close each panel
	for (UCavrnusBasePanelWidget* Panel : PanelsToClose)
	{
		if (IsValid(Panel))
		{
			Close(Panel);
		}
	}

	ActivePanels.Empty();
}

void UCavrnusPanelSystem::Dispose()
{
	CloseAll();
	DisplayerObj = nullptr;
}

UCavrnusBasePanelWidget* UCavrnusPanelSystem::CreateInternal(
	const UClass* Type,
	const FCavrnusPanelOptions& Options,
	const FUIContextData& Data)
{
	const auto FoundBlueprint = LookupAsset->GetPanelBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (auto* PanelInstance = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBasePanelWidget>(FoundBlueprint, GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		PanelInstance->SetId(Id);
		PanelInstance->SetContextData(Data);
		
		ActivePanels.Add(Id, PanelInstance);

		Displayer->DisplayPanelWidget(PanelInstance, Options,[this](UCavrnusBasePanelWidget* PanelToClose)
		{
			Close(PanelToClose);
		});
		
		return PanelInstance;
	}
	
	return nullptr;
}

void UCavrnusPanelSystem::SetVisibility(const bool bCond)
{
	for (const TPair<FGuid, UCavrnusBasePanelWidget*> Panel : ActivePanels)
		Panel.Value->SetVisibility(bCond ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
