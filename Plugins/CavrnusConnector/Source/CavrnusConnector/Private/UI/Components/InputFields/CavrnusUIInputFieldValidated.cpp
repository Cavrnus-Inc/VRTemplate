// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"

#include "UI/Components/EditableTextBox/CavrnusUIEditableTextBox.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

void UCavrnusUIInputFieldValidated::SetValidationMessage(const FString& InMsg)
{
	if (ValidationMessageTextBlock)
		ValidationMessageTextBlock->SetText(FText::FromString(InMsg));
}

UCavrnusUIInputFieldValidated* UCavrnusUIInputFieldValidated::SetValidationType(
	const ECavrnusInputFieldValidationType& InType)
{
	ValidationType = InType;
	return this;
}

UCavrnusUIInputFieldValidated* UCavrnusUIInputFieldValidated::SetValidationPredicate(const TFunction<bool(const FString&)>& InPredicate)
{
	ValidationPredicate = InPredicate;
	return this;
}

void UCavrnusUIInputFieldValidated::ClearValidationMessage()
{
	SetValidationMessage("");
}

bool UCavrnusUIInputFieldValidated::GetHasValidInput()
{
	return HasValidInput;
}

void UCavrnusUIInputFieldValidated::NativeConstruct()
{
	Super::NativeConstruct();

	if (ValidationMessageTextBlock)
	{
		ValidationMessageTextBlock->SetVisibility(ESlateVisibility::Hidden);
		if (InputFieldStyleData)
			ValidationMessageTextBlock->SetColorAndOpacity(InputFieldStyleData->ValidationErrorColor);
	}
}

bool UCavrnusUIInputFieldValidated::ShouldBeValidated()
{
	return HasFieldBeenInteractedWith || !EditableTextBox->GetText().IsEmpty();
}

void UCavrnusUIInputFieldValidated::HandleTextUpdated(const FText& InText)
{
	Super::HandleTextUpdated(InText);
	
	if (ShouldBeValidated())
	{
		if (ValidationType == ECavrnusInputFieldValidationType::Early)
			ValidateInput(InText.ToString());
	}

	if (OnValidatedTextUpdated.IsBound())
		OnValidatedTextUpdated.Broadcast(InText, HasValidInput);

	HasFieldBeenInteractedWith = true;
}

void UCavrnusUIInputFieldValidated::HandleTextCommitted(const FText& InText, ETextCommit::Type InCommitMethod)
{
	if (ShouldBeValidated())
	{
		ValidateInput(InText.ToString());

		if (HasValidInput)
			ClearValidationMessage();
		
		if (OnValidatedTextCommitted.IsBound())
			OnValidatedTextCommitted.Broadcast(InText, HasValidInput);
	}
}

void UCavrnusUIInputFieldValidated::ValidateInput(const FString& InText)
{
	if (ShouldBeValidated())
	{
		HasValidInput = (ValidationPredicate != nullptr) ? ValidationPredicate(InText) : true;

		if (ValidationMessageTextBlock)
			ValidationMessageTextBlock->SetVisibility(HasValidInput ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		
		ApplyCurrentStateStyle();
	}
}

FInputFieldStyleStateData UCavrnusUIInputFieldValidated::GetCurrentStateData()
{
	if (InputFieldStyleData == nullptr)
		return FInputFieldStyleStateData();

	if (ShouldBeValidated())
		return HasValidInput ? InputFieldStyleData->NormalState : InputFieldStyleData->ValidationErrorState;

	return InputFieldStyleData->NormalState;
}
