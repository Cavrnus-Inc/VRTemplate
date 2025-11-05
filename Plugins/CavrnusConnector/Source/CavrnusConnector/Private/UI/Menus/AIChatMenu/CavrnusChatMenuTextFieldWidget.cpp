// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/ChatMenus/CavrnusChatMenuTextFieldWidget.h"

#include "UI/Components/EditableTextBox/CavrnusUIEditableTextBox.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"

void UCavrnusChatMenuTextFieldWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (BaseMenu.IsValid())
	{
		BaseMenu->OnMessageAdded.Remove(OnChatAddedHandle);
		OnChatAddedHandle.Reset();
	}
}

void UCavrnusChatMenuTextFieldWidget::Setup(UCavrnusChatMenuBase* InBaseMenu)
{
	BaseMenu = InBaseMenu;

	OnChatAddedHandle = InBaseMenu->OnMessageAdded.AddWeakLambda(this, [](const FCavrnusChatMessage&)
	{
		// reset scroll window to show latest, or show a button
	});
}

void UCavrnusChatMenuTextFieldWidget::SetHintText(const FText& InHintText)
{
	if (TextBox)
		TextBox->SetHintText(InHintText);
}

void UCavrnusChatMenuTextFieldWidget::CreateNewMessage()
{
	FCavrnusChatMessage ChatMessage = FCavrnusChatMessage();
	ChatMessage.Message = TextBox->GetText().ToString();
	ChatMessage.IsLocalUser = true;

	BaseMenu->AddMessage(ChatMessage);
}
