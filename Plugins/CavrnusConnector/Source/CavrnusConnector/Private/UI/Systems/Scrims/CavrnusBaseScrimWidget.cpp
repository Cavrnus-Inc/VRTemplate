// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Scrims/CavrnusBaseScrimWidget.h"

void UCavrnusBaseScrimWidget::InitializeScrim(const FCavrnusScrimOptions& InOptions)
{
	Options = InOptions;
}

void UCavrnusBaseScrimWidget::CloseScrim()
{
	OnCloseDelegate.Broadcast();
}

void UCavrnusBaseScrimWidget::HookOnClose(const TFunction<void()>& OnCloseCallback)
{
	OnCloseDelegate.AddWeakLambda(this, OnCloseCallback);
}

FReply UCavrnusBaseScrimWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (Options.bCloseOnClick)
		CloseScrim();
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
