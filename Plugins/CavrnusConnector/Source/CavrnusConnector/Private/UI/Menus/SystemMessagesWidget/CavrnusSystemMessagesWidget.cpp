// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/SystemMessagesWidget/CavrnusSystemMessagesWidget.h"

#include "CavrnusFunctionLibrary.h"
#include "UI/CavrnusUI.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/ServerStatus/CavrnusServerStatusToastWidget.h"

class UCavrnusServerStatusToastWidget;

void UCavrnusSystemMessagesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([this](const FCavrnusSpaceConnection& SC)
	{
		UCavrnusFunctionLibrary::BindSpaceStatus(SC, [this](const FCavrnusConnectionStatus& CCS)
		{
			UCavrnusUI::Get(this)->Messages()->Toast()
				->CreateAutoClose<UCavrnusServerStatusToastWidget>()
				->SetStatus(CCS);
		});
	});
}
