// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

#include "CavrnusConnectorModule.h"
#include "UI/CavrnusUI.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/Displayers/CavrnusWidgetDisplayer.h"
#include "UI/Systems/Scrims/CavrnusUIScrimSystem.h"
#include "UI/Systems/Scrims/Types/CavrnusUIScrim.h"

void UCavrnusRawWidgetHost::Initialize(UObject* InOuterObject, ICavrnusWidgetDisplayer* InDisplayer)
{
	OuterObject = InOuterObject;
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;
	
	VisDelegate = UCavrnusUI::Get()->Arbiter()->CurrentUIVisMode.Bind(this,[this](const EUIVisibilityMode Mode)
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

UUserWidget* UCavrnusRawWidgetHost::Show(const TSubclassOf<UUserWidget>& InBlueprint)
{
	if (OuterObject == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("[UCavrnusRawWidgetHost::Show] OuterObject is null! Cannot show widget."))
		return nullptr;
	}
	
	if (UUserWidget* Widget = FCavrnusWidgetFactory::CreateUserWidget<UUserWidget>(InBlueprint, OuterObject->GetWorld()))
	{
		const auto Id = FGuid::NewGuid();
		Displayer->DisplayRawWidget(Id, Widget);
		Widgets.Add(Widget, Id);
		
		return Widget;
	}

	return nullptr;
}

UUserWidget* UCavrnusRawWidgetHost::ShowWithScrim(const TSubclassOf<UUserWidget>& InBlueprint, const FCavrnusScrimOptions& ScrimOptions)
{
	const auto Widget = Show(InBlueprint);

	if (const auto FoundIdPtr = Widgets.Find(Widget))
		UCavrnusUI::Get(this)->Scrims()->AssignToWidget<UCavrnusUIScrim>(*FoundIdPtr, ScrimOptions);
	
	return Widget;
}

bool UCavrnusRawWidgetHost::Close(const UUserWidget* WidgetToClose)
{
	if (WidgetToClose == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCavrnusRawWidgetHost::Close] WidgetToClose is null! Cannot close"))
		return false;
	}

	if (const auto FoundGuidPtr = Widgets.Find(WidgetToClose))
	{
		if (FoundGuidPtr)
			Displayer->RemoveWidget(*FoundGuidPtr);

		UCavrnusUI::Get(this)->Scrims()->CloseWidgetScrim(*FoundGuidPtr);

		return true;
	}

	return false;
}

void UCavrnusRawWidgetHost::CloseAll()
{
	for (const TPair<UUserWidget*, FGuid>& Pair : Widgets)
	{
		const FGuid& Id = Pair.Value;
		
		UCavrnusUI::Get(this)->Scrims()->CloseWidgetScrim(Id);
		Displayer->RemoveWidget(Id);
	}
	
	Widgets.Empty();
}

void UCavrnusRawWidgetHost::SetVisibility(const bool bVis)
{
	for (const TPair<UUserWidget*, FGuid>& Pair : Widgets)
	{
		if (auto* Found = Pair.Key)
		{
			Found->SetVisibility(bVis ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

			// This is a major assumption here - ideally we keep track where a widget DID have a scrim and reassign if so. But lazy rn...
			if (bVis)
				UCavrnusUI::Get(this)->Scrims()->AssignToWidget<UCavrnusUIScrim>(Pair.Value, FCavrnusScrimOptions());
			 else
				UCavrnusUI::Get(this)->Scrims()->CloseWidgetScrim(Pair.Value);
		}
	}
}

void UCavrnusRawWidgetHost::Teardown()
{
	Widgets.Empty();
	UCavrnusUI::Get()->Arbiter()->CurrentUIVisMode.Unbind(VisDelegate);
}
