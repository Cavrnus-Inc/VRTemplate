// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"

void UCavrnusBaseDialogWidget::CloseDialog()
{
	OnCloseDelegate.Broadcast();
}

void UCavrnusBaseDialogWidget::HookOnClose(const TFunction<void()>& OnCloseCallback)
{
	OnCloseDelegate.AddLambda(OnCloseCallback);
}

void UCavrnusBaseDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
		ButtonHandle = CloseButton->OnButtonClicked.AddWeakLambda(this, [this] { CloseDialog(); });
}

void UCavrnusBaseDialogWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (CloseButton)
	{
		CloseButton->OnButtonClicked.Remove(ButtonHandle);
		ButtonHandle.Reset();
	}

	if (OnCloseDelegate.IsBound())
		OnCloseDelegate.Clear();
}
