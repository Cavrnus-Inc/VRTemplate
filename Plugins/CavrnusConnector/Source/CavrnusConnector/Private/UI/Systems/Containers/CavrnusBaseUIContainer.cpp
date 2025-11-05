// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Containers/CavrnusBaseUIContainer.h"

#include "Components/NamedSlot.h"
#include "UI/Components/Buttons/Types/CavrnusUIIconButton.h"

void UCavrnusBaseUIContainer::SetWidgetContent(UUserWidget* InContent)
{
	if (ContentSlot)
		ContentSlot->AddChild(InContent);
}

void UCavrnusBaseUIContainer::HookOnCloseButton(const TFunction<void()>& OnCloseCallback)
{
	if (CloseButton)
		CloseHandle = CloseButton->OnButtonClicked.AddWeakLambda(this, OnCloseCallback);
}

void UCavrnusBaseUIContainer::NativeDestruct()
{
	Super::NativeDestruct();

	if (CloseButton)
	{
		CloseButton->OnButtonClicked.Remove(CloseHandle);
		CloseHandle.Reset();
	}
}
