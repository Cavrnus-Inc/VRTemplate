// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/LoginMenus/CavrnusMemberLoginWidget.h"

#include "CavrnusFunctionLibrary.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/DataState.h"
#include "UI/CavrnusUI.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageEnum.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusMemberLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto HasValidFields = [this]() -> bool {
		return PasswordInputField->GetHasValidInput()
			&& EmailInputField->GetHasValidInput();
	};

	if (EmailInputField)
	{
		EmailInputField
			->SetLabel("Email*")
			->SetHintText("Enter email")
			->SetToKeyboardFocusedCustom();
		
		EmailInputField
			->SetValidationType(ECavrnusInputFieldValidationType::Late)
			->SetValidationPredicate([](const FString& Text)
			{
				return !Text.IsEmpty() && Text.Contains("@");
			});

		EmailInputField->OnValidatedTextUpdated.AddWeakLambda(this, [this, HasValidFields](const FText& Text, const bool IsValidInput)
			{
				if (LoginButton)
					LoginButton->SetEnabledState(HasValidFields());
			});
		EmailInputField->OnValidatedTextCommitted.AddWeakLambda(this, [this, HasValidFields] (const FText& Text, const bool IsValidInput)
			{
				if (!IsValidInput)
				{
					if (Text.IsEmpty())
						EmailInputField->SetValidationMessage("Field must not be empty!");
					if (!Text.ToString().Contains("@"))
						EmailInputField->SetValidationMessage("Please enter valid email! Missing '@'.");
				}

				if (LoginButton)
					LoginButton->SetEnabledState(HasValidFields());
			});
	}

	if (PasswordInputField)
	{
		PasswordInputField
			->SetLabel("Password*")
			->SetHintText("Enter password")
			->SetIsPasswordField(true);
		
		PasswordInputField
			->SetValidationType(ECavrnusInputFieldValidationType::Early)
			->SetValidationPredicate([](const FString& Text)
			{
				return !Text.IsEmpty();
			});

		PasswordInputField->OnValidatedTextUpdated.AddWeakLambda(this, [this, HasValidFields](const FText&, bool)
		{
			if (LoginButton)
				LoginButton->SetEnabledState(HasValidFields());
		});
		PasswordInputField->OnValidatedTextCommitted.AddWeakLambda(this, [this, HasValidFields] (const FText&, const bool IsValidInput)
			{
				if (!IsValidInput)
					PasswordInputField->SetValidationMessage("Password must not be empty!");

				if (LoginButton)
					LoginButton->SetEnabledState(HasValidFields());
			});
	}

	if (LoginButton)
	{
		LoginButton->SetEnabledState(false);
		LoginButtonDelegateHandle = LoginButton->OnButtonClicked.AddWeakLambda(this, [this]
		{
			TryLogin();
		});
	}
}

void UCavrnusMemberLoginWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (LoginButton)
	{
		LoginButton->OnButtonClicked.Remove(LoginButtonDelegateHandle);
		LoginButtonDelegateHandle.Reset();
	}
}

void UCavrnusMemberLoginWidget::TryLogin()
{
	const FString Server = Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer;
	UCavrnusFunctionLibrary::AuthenticateWithPassword(
		Server, EmailInputField->GetText(),
		PasswordInputField->GetText(),
		[this](const FCavrnusAuthentication&)
		{
			UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(this);
		}, [](const FString& Error)
		{
			UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
				->SetPrimaryText("Member login failed")
				->SetSecondaryText(Error)
				->SetType(ECavrnusInfoToastMessageEnum::Error);
		});
}