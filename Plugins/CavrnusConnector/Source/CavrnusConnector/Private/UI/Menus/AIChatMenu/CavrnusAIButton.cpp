// // Copyright (c) 2025 Cavrnus. All rights reserved.


#include "UI/Menus/ChatMenus/CavrnusAIButton.h"

#include "Components/Button.h"
#include "UI/Menus/ChatMenus/CavrnusChatMenuBase.h"

class UCavrnusChatMenuBase;

void UCavrnusAIButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Button)
		Button->OnClicked.AddDynamic(this, &UCavrnusAIButton::ButtonClicked);
}

void UCavrnusAIButton::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (Button)
		Button->OnClicked.RemoveDynamic(this, &UCavrnusAIButton::ButtonClicked);
}

void UCavrnusAIButton::ButtonClicked()
{
	// if (UCavrnusChatMenuBase* AiChatMenu = UCavrnusUI::Get(this)->MenuLookup()->GetWidgetInstance<UCavrnusChatMenuBase>("ai-chat", GetWorld()))
	// {
	// 	AiChatMenu->Setup();
	// 	
	// 	FCavrnusPopupOptions Options = FCavrnusPopupOptions();
	// 	Options.PopupTitle = FText::FromString("AI Insights");
	// 	UCavrnusUI::Get(this)->Popups()->Create(AiChatMenu, Options);
	// }
}