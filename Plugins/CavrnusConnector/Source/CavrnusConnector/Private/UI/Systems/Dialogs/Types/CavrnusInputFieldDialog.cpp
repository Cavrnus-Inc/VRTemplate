// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Dialogs/Types/CavrnusInputFieldDialog.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::SetTitleText(const FString& InText)
{
	if (TitleText)
		TitleText->SetText(FText::FromString(InText));

	SetTitle(InText);
	
	return this;
}

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::SetBodyText(const FString& InText)
{
	if (BodyText)
		BodyText->SetText(FText::FromString(InText));

	return this;
}

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::SetInputFieldHintText(const FString& InText)
{
	if (InputField)
		InputField->SetHintText(InText);
	
	return this;
}

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::SetConfirmButton(const FString& InText, const TFunction<void(const FString& SubmittedText)>& OnClicked)
{
	if (ConfirmButton)
	{
		ConfirmButton->SetButtonText(FText::FromString(InText));
		ConfirmButtonHandle = ConfirmButton->OnButtonClicked.AddWeakLambda(this, [OnClicked, this]
		{
			CloseDialog();
			
			OnClicked(InputField->GetText());
		});
	}

	return this;
}

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::SetDismissButton(const FString& InText, const TFunction<void()>& OnClicked)
{
	if (DismissButton)
	{
		DismissButton->SetButtonText(FText::FromString(InText));
		DismissButtonHandle = DismissButton->OnButtonClicked.AddWeakLambda(this, [OnClicked, this]
		{
			CloseDialog();

			if (OnClicked)
				OnClicked();
		});
	}
	
	return this;
}

UCavrnusInputFieldDialog* UCavrnusInputFieldDialog::OnTextSubmit(const TFunction<void(const FString& Text)>& OnSubmit)
{
	OnSubmitDelegate = OnSubmit;
	return this;
}

void UCavrnusInputFieldDialog::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
		ConfirmButton->SetEnabledState(false);

	if (InputField)
	{
		InputField->SetValidationType(ECavrnusInputFieldValidationType::Early);
		InputFieldHandle = InputField->OnValidatedTextUpdated.AddWeakLambda(this, [this](const FText& Text, bool IsValid)
		{
			if (ConfirmButton)
				ConfirmButton->SetEnabledState(IsValid);
		});
		InputField->SetValidationPredicate([](const FString& Text)
		{
			return !Text.IsEmpty();
		});
	}
}

void UCavrnusInputFieldDialog::NativeDestruct()
{
	Super::NativeDestruct();

	OnSubmitDelegate.Reset();

	if (InputField)
	{
		InputField->OnValidatedTextUpdated.Remove(InputFieldHandle);
		InputFieldHandle.Reset();
	}

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
