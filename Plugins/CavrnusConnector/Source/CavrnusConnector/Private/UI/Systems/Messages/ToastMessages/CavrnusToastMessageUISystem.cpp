// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"

#include "UI/Helpers/CavrnusUIScheduler.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/AssetLookup/CavrnusWidgetBlueprintLookup.h"
#include "UI/Systems/Messages/CavrnusBaseUIMessageWidget.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusBaseToastMessageWidget.h"

void UCavrnusToastMessageUISystem::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	LookupAsset = InLookup;
	
	DisplayerObj = Cast<UObject>(InDisplayer);
	Displayer = InDisplayer;

	ActiveMessages.Empty();
	ActiveMessagesMap.Empty();
}

void UCavrnusToastMessageUISystem::Close(UCavrnusBaseUserWidget* WidgetToClose)
{
	if (WidgetToClose == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusToastMessageUISystem::Close] WidgetToClose is nullptr! Cannot close."))
		return;
	}

	auto* Message = Cast<UCavrnusBaseUIMessageWidget>(WidgetToClose);
	if (Message == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCavrnusToastMessageUISystem::Close] Message cast failed! Cannot close."))
		return;
	}
	
	if (auto** FoundPtr = ActiveMessagesMap.Find(Message->GetId()))
	{
		if (auto* FoundWidget = *FoundPtr)
		{
			if (FTimerHandle* FoundTimerHandlePtr = AutoDismissTimersMap.Find(FoundWidget))
			{
				FCavrnusUIScheduler::ClearTimer(GetWorld(), *FoundTimerHandlePtr);
				AutoDismissTimersMap.Remove(FoundWidget);
			}

			Displayer->RemoveWidget(FoundWidget->GetId());
			
			ActiveMessages.Remove(FoundWidget);
			ActiveMessagesMap.Remove(Message->GetId());
		}
	}
}

void UCavrnusToastMessageUISystem::CloseAll()
{
	if (ActiveMessages.IsEmpty())
		return;
	
	for (UUserWidget* Msg : ActiveMessages)
		Close(Cast<UCavrnusBaseToastMessageWidget>(Msg));
	
	AutoDismissTimersMap.Empty();
	ActiveMessages.Empty();
	ActiveMessagesMap.Empty();
}

void UCavrnusToastMessageUISystem::Teardown()
{
}

void UCavrnusToastMessageUISystem::StartAutoDismissTimer(UCavrnusBaseToastMessageWidget* Widget, const float Duration)
{
	Widget->StartTimer(Duration);
	const FTimerHandle TimerHandle = FCavrnusUIScheduler::SetTimer(GetWorld(), Duration, [this, Widget]
	{
		if (!Widget->Hovered)
			Close(Widget);
	});
	
	AutoDismissTimersMap.Add(Widget, TimerHandle);
}

UCavrnusBaseUIMessageWidget* UCavrnusToastMessageUISystem::CreateInternal(const UClass* Type, const FCavrnusToastMessageOptions& Options)
{
	const auto FoundBlueprint = LookupAsset->GetToastMessageBlueprint(Type);
	if (FoundBlueprint == nullptr)
		return nullptr;
	
	if (UCavrnusBaseToastMessageWidget* MessageWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusBaseToastMessageWidget>(FoundBlueprint, GetWorld()))
	{
		if (Options.ShouldAutoDismiss)
			StartAutoDismissTimer(MessageWidget, Options.AutoDismissDuration);

		const auto Id = FGuid::NewGuid();
		MessageWidget->SetId(Id);
		
		ActiveMessagesMap.Add(Id, MessageWidget);
		Displayer->DisplayToastMessageWidget(MessageWidget, Options);

		MessageWidget->HookOnClose([this, MessageWidget]
		{
			Close(MessageWidget);
		});

		return MessageWidget;
	}
	
	return nullptr;
}