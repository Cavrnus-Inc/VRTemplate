// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"

#include "UI/Menus/ChatMenus/CavrnusChatMenuConversationWidget.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuTextFieldWidget.h"

void UCavrnusChatMenuBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (TextFieldWidget)
		TextFieldWidget->Setup(this);

	if (ConversationWidget)
		ConversationWidget->Setup(this, ChatEntryBlueprint);
}

void UCavrnusChatMenuBase::NativeDestruct()
{
	Super::NativeDestruct();
	
	OnMessageAdded.RemoveAll(this);
}

UCavrnusChatMenuBase* UCavrnusChatMenuBase::Setup()
{
	return this;
}

UCavrnusChatMenuBase* UCavrnusChatMenuBase::SetHintText(const FText& InHintText)
{
	if (TextFieldWidget)
		TextFieldWidget->SetHintText(InHintText);

	return this;
}

void UCavrnusChatMenuBase::AddMessage(const FCavrnusChatMessage& Message)
{
	if (OnMessageAdded.IsBound())
		OnMessageAdded.Broadcast(Message);
}