// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Dialogs/Types/CavrnusConfirmationDialog.h"

#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

UCavrnusConfirmationDialog* UCavrnusConfirmationDialog::SetTitleText(const FString& InText)
{
	if (TitleText)
		BodyText->SetText(FText::FromString(InText));

	SetTitle(InText);
	
	return this;
}

UCavrnusConfirmationDialog* UCavrnusConfirmationDialog::SetBodyText(const FString& InText)
{
	if (BodyText)
		BodyText->SetText(FText::FromString(InText));

	return this;
}

UCavrnusConfirmationDialog* UCavrnusConfirmationDialog::SetConfirmButton(const FString& InText, const TFunction<void()>& OnClicked)
{
	if (ConfirmButton)
	{
		ConfirmButton->SetButtonText(FText::FromString(InText));
		ConfirmButtonHandle = ConfirmButton->OnButtonClicked.AddWeakLambda(this, [OnClicked, this]
		{
			CloseDialog();
			OnClicked();
		});
	}

	return this;
}

UCavrnusConfirmationDialog* UCavrnusConfirmationDialog::SetDismissButton(const FString& InText, const TFunction<void()>& OnClicked)
{
	if (DismissButton)
	{
		DismissButton->SetButtonText(FText::FromString(InText));
		DismissButtonHandle = DismissButton->OnButtonClicked.AddWeakLambda(this, [OnClicked, this]
		{
			CloseDialog();
			OnClicked();
		});
	}
	
	return this;
}

void UCavrnusConfirmationDialog::NativeDestruct()
{
	Super::NativeDestruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnButtonClicked.Remove(ConfirmButtonHandle);
		ConfirmButtonHandle.Reset();
	}
	
	if (DismissButton)
	{
		DismissButton->OnButtonClicked.Remove(DismissButtonHandle);
		DismissButtonHandle.Reset();
	}
}
