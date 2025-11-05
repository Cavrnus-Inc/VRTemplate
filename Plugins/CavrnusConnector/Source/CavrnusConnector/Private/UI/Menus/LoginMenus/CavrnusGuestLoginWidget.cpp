// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/LoginMenus/CavrnusGuestLoginWidget.h"
#include "CavrnusFunctionLibrary.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/DataState.h"
#include "UI/CavrnusUI.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusGuestLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GuestUsernameInput)
	{
		GuestUsernameInput
			->SetLabel("Name")
			->SetHintText("Enter name")
			->SetToKeyboardFocusedCustom();
		
		GuestUsernameInput
			->SetValidationType(ECavrnusInputFieldValidationType::Early)
			->SetValidationPredicate([](const FString& Text) { return !Text.IsEmpty(); });

		GuestUsernameInput->OnValidatedTextUpdated.AddWeakLambda(this, [this](const FText&, const bool IsValidInput)
			{
				if (LoginButton)
					LoginButton->SetEnabledState(IsValidInput);
			});
		GuestUsernameInput->OnValidatedTextCommitted.AddWeakLambda(this, [this] (const FText&, const bool IsValidInput)
			{
				if (!IsValidInput)
					GuestUsernameInput->SetValidationMessage("Field must not be empty!");

				if (LoginButton)
					LoginButton->SetEnabledState(IsValidInput);
			});
	}

	if (LoginButton)
	{
		LoginButton->SetEnabledState(false);
		LoginButtonDelegateHandle = LoginButton->OnButtonClicked.AddWeakLambda(this, [this]
		{
			TryJoin();
		});
	}
}

void UCavrnusGuestLoginWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (LoginButton)
	{
		LoginButton->OnButtonClicked.Remove(LoginButtonDelegateHandle);
		LoginButtonDelegateHandle.Reset();
	}
}

void UCavrnusGuestLoginWidget::TryJoin()
{
	const FString Server = Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState()->CurrentServer;
	UCavrnusFunctionLibrary::AuthenticateAsGuest(Server, GuestUsernameInput->GetText(),
	                                             [this](const FCavrnusAuthentication&)
	                                             {
	                                             	RemoveFromParent();
	                                             },
	                                             [](const FString& Error)
	                                             {
	                                             	UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
														 ->SetPrimaryText("Member login failed")
														 ->SetSecondaryText(Error)
														 ->SetType(ECavrnusInfoToastMessageEnum::Error);
	                                             });

	UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(this);
}