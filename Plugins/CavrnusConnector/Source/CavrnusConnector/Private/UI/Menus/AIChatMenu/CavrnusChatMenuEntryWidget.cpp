// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/ChatMenus/CavrnusChatMenuEntryWidget.h"

#include "UI/Components/Borders/CavrnusUIBorder.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"

void UCavrnusChatMenuEntryWidget::Setup(const FCavrnusChatMessage& InChatMessage)
{
	if (MessageTextBlock)
		MessageTextBlock->SetText(FText::FromString(InChatMessage.Message));

	if (DateTextBlock)
		DateTextBlock->SetText(FText::FromString(InChatMessage.Timestamp.ToString()));

	if (InChatMessage.IsLocalUser)
	{
		if (MessageBackground)
			MessageBackground->SetBrushColor(LocalUserColor);
	} else
	{
		if (MessageBackground)
			MessageBackground->SetBrushColor(RemoteUserColor);
	}
}
