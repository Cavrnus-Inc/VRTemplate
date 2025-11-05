// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Screens/CavrnusScreenSystem.h"

#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"

void UCavrnusScreenSystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	LookupAsset = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
	
	ScreenStack.Empty();
}

void UCavrnusScreenSystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (Cast<UCavrnusBaseScreenWidget>(WidgetToClose))
	{
		// Fully remove this screen
		ScreenStack.Pop();
		Displayer->RemoveWidget(WidgetToClose->GetId());

		// If there was a prev screen, show it!
		if (!ScreenStack.IsEmpty())
		{
			if (auto* Prev = ScreenStack.Top())
				Prev->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	if (ScreenStack.IsEmpty())
	{
		UCavrnusUI::Get()->Arbiter()->SetVisibilityMode(EUIVisibilityMode::Normal);
	}
}

void UCavrnusScreenSystem::CloseAll()
{
	if (ScreenStack.IsEmpty())
		return;

	while (ScreenStack.Num() > 0)
	{
		if (const auto* Screen = ScreenStack.Pop())
			Displayer->RemoveWidget(Screen->GetId());
	}

	UCavrnusUI::Get()->Arbiter()->SetVisibilityMode(EUIVisibilityMode::Normal);
}

void UCavrnusScreenSystem::Teardown()
{
	CloseAll();
	
	Displayer = nullptr;
}

UCavrnusBaseScreenWidget* UCavrnusScreenSystem::CreateInternal(
	const UClass* Type,
	const FCavrnusScreenOptions& Options,
	const FUIContextData& Data)
{
	const auto FoundBlueprint = LookupAsset->GetScreenBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;

	if (!ScreenStack.IsEmpty())
	{
		if (auto* CurrScreen = ScreenStack.Top())
			CurrScreen->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (auto* ScreenWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBaseScreenWidget>(FoundBlueprint, GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		ScreenWidget->SetId(Id);
		ScreenWidget->SetContextData(Data);
		
		ScreenStack.Push(ScreenWidget);
		
		Displayer->DisplayScreenWidget(ScreenWidget, Options,[this](UCavrnusBaseScreenWidget* PopupToClose)
		{
			Close(PopupToClose);
		});

		// Screens take precedence!
		UCavrnusUI::Get()->Arbiter()->SetVisibilityMode(EUIVisibilityMode::ScreenExclusive);
		
		return ScreenWidget;
	}
	
	return nullptr;
}
