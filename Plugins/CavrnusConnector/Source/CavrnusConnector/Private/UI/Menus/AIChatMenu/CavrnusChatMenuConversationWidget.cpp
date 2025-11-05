// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/ChatMenus/CavrnusChatMenuConversationWidget.h"

#include "Components/VerticalBox.h"
#include "UI/Components/ScrollBox/CavrnusUIScrollBox.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuEntryWidget.h"

void UCavrnusChatMenuConversationWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (BaseMenu.IsValid())
	{
		BaseMenu->OnMessageAdded.Remove(OnChatAddedHandle);
		OnChatAddedHandle.Reset();
	}

	ChatMessages.Empty();
}

void UCavrnusChatMenuConversationWidget::Setup(UCavrnusChatMenuBase* InBaseMenu, const TSubclassOf<UCavrnusChatMenuEntryWidget> InEntryBlueprint)
{
	ChatMessages.Empty();
	EntryBlueprint = InEntryBlueprint;

	OnChatAddedHandle = InBaseMenu->OnMessageAdded.AddWeakLambda(this, [this](const FCavrnusChatMessage& Cm)
	{
		// reset scroll window to show latest, or show a button
		AddChat(Cm);
	});
}

void UCavrnusChatMenuConversationWidget::AddChat(const FCavrnusChatMessage& InChatMessage)
{
	if (VerticalBoxContainer)
	{
		if (EntryBlueprint)
		{
			auto* ChatWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusChatMenuEntryWidget>(EntryBlueprint, GetWorld());
			ChatWidget->Setup(InChatMessage);
			
			VerticalBoxContainer->AddChild(ChatWidget);
			ChatMessages.Add(ChatWidget);

			if (ScrollBox)
				ScrollBox->ScrollToEnd();
		}
	}
}