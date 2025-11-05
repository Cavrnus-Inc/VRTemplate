// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"

void UCavrnusScopedMessages::Initialize(UCavrnusWidgetBlueprintLookup* InLookup, ICavrnusWidgetDisplayer* InDisplayer)
{
	ToastMessages = NewObject<UCavrnusToastMessageUISystem>(this);
	ToastMessages->Initialize(InLookup, InDisplayer);
}