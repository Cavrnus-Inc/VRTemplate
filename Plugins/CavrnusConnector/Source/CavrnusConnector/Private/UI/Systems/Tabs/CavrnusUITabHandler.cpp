// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Tabs/CavrnusUITabHandler.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

UCavrnusUITabHandler* UCavrnusUITabHandler::Register(const FString& InId, UCavrnusUIButton* InButton, UUserWidget* InWidget)
{
	DelegateHandles.Add(InButton, InButton->OnButtonClicked.AddLambda([this, InId]
	{
		SetActive(InId);
	}));

	if (InWidget)
		InWidget->SetVisibility(ESlateVisibility::Collapsed);

	WidgetMap.Add(InId, InWidget);
	ButtonMap.Add(InId, InButton);

	return this;
}

UCavrnusUITabHandler* UCavrnusUITabHandler::Register(const FString& InId, UCavrnusUIButton* InButton, const TSubclassOf<UUserWidget> InBlueprint)
{
	if (auto* Widget = FCavrnusWidgetFactory::CreateUserWidget(InBlueprint, GetWorld()))
		Register(InId, InButton, Widget);

	return this;
}

UCavrnusUITabHandler* UCavrnusUITabHandler::SetAllowToggleOff(bool bAllow)
{
	bAllowToggleOff = bAllow;
	return this;
}

UCavrnusUITabHandler* UCavrnusUITabHandler::SetActive(const FString& InId)
{
	if (!CurrentActiveId.IsEmpty())
	{
		if (CurrentActiveId == InId)
		{
			if (!bAllowToggleOff)
				return this;

			if (const TWeakObjectPtr<UWidget>* FoundPtr = WidgetMap.Find(CurrentActiveId))
				FoundPtr->Get()->SetVisibility(ESlateVisibility::Collapsed);

			if (const TWeakObjectPtr<UCavrnusUIButton>* BtnPtr = ButtonMap.Find(CurrentActiveId))
				if (BtnPtr->IsValid())
					BtnPtr->Get()->SetSelected(false);

			CurrentActiveId = "";
			return this;
		}

		if (const TWeakObjectPtr<UWidget>* FoundPtr = WidgetMap.Find(CurrentActiveId))
			FoundPtr->Get()->SetVisibility(ESlateVisibility::Collapsed);

		if (const TWeakObjectPtr<UCavrnusUIButton>* BtnPtr = ButtonMap.Find(CurrentActiveId))
			if (BtnPtr->IsValid())
				BtnPtr->Get()->SetSelected(false);
	}

	if (const TWeakObjectPtr<UWidget>* FoundPtr = WidgetMap.Find(InId))
	{
		if (FoundPtr && FoundPtr->IsValid())
		{
			FoundPtr->Get()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			CurrentActiveId = InId;
		}
	}

	if (const TWeakObjectPtr<UCavrnusUIButton>* BtnPtr = ButtonMap.Find(InId))
		if (BtnPtr->IsValid())
			BtnPtr->Get()->SetSelected(true);

	return this;
}

void UCavrnusUITabHandler::Teardown()
{
	for (const TPair<TWeakObjectPtr<UCavrnusUIButton>, FDelegateHandle>& Pair : DelegateHandles)
	{
		TWeakObjectPtr<UCavrnusUIButton> ButtonPtr = Pair.Key;
		const FDelegateHandle& Handle = Pair.Value;

		if (ButtonPtr.IsValid())
		{
			UCavrnusUIButton* Button = ButtonPtr.Get();
			Button->OnButtonClicked.Remove(Handle);
		}
	}
}
