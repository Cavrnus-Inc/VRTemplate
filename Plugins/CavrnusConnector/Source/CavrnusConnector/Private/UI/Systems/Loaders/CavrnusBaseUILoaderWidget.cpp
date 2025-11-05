// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Loaders/CavrnusBaseUILoaderWidget.h"

void UCavrnusBaseUILoaderWidget::CloseLoader() const
{
	OnCloseDelegate.Broadcast();
}

void UCavrnusBaseUILoaderWidget::HookOnClose(const TFunction<void()>& OnCloseCallback)
{
	OnCloseDelegate.AddWeakLambda(this, OnCloseCallback);
}
