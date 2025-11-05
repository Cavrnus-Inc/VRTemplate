// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusFunctionLibrary.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "UI/CavrnusUI.h"
#include "UI/Menus/ServerSelectionMenu/CavrnusServerSelectWidget.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusServerSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ServerTextBox)
	{
		ServerTextBox
			->SetLabel("Server Domain")
			->SetHintText("Enter server domain (yourDomain.cavrn.us)")
			->SetToKeyboardFocusedCustom();
		
		ServerTextBox
			->SetValidationType(ECavrnusInputFieldValidationType::Early)
			->SetValidationPredicate([](const FString& Text) { return !Text.IsEmpty(); });
		
		ServerTextBox->OnValidatedTextUpdated.AddWeakLambda(this, [this](const FText&, const bool IsValidInput)
			{
				if (ProceedButton)
					ProceedButton->SetEnabledState(IsValidInput);
			});
		ServerTextBox->OnValidatedTextCommitted.AddWeakLambda(this, [this] (const FText&, const bool IsValidInput)
			{
				if (!IsValidInput)
					ServerTextBox->SetValidationMessage("Server domain must not be empty!");

				if (ProceedButton)
					ProceedButton->SetEnabledState(IsValidInput);
			});
	}

	ProceedButton->SetEnabledState(false);
	ProceedButtonDelegate = ProceedButton->OnButtonClicked.AddWeakLambda(this, [this]
	{
		ProceedButton->SetEnabledState(false);
		ServerTextBox->SetIsEnabled(false);
		
		auto Server = ServerTextBox->GetText();
		UCavrnusLoginManager::ResolveServer(Server);
		
		UCavrnusFunctionLibrary::CheckServerStatus(Server, [this, Server](const FCavrnusServerStatus& Status)
		{
			if (Status.Live)
			{
				UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(this);
				UCavrnusFunctionLibrary::SetServer(Server);
			}
			else
			{
				ServerTextBox->SetIsEnabled(true);
				ServerTextBox->SetValidationMessage("Failed to reach domain! Ensure your domain is correct.");

				UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
					->SetPrimaryText("Invalid Server Domain")
					->SetSecondaryText(FString::Printf(TEXT("%s"), *Status.FailReason))
					->SetType(ECavrnusInfoToastMessageEnum::Error);
			}
		});
	});
}

void UCavrnusServerSelectionWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (ProceedButton)
	{
		ProceedButton->OnButtonClicked.Remove(ProceedButtonDelegate);
		ProceedButtonDelegate.Reset();
	}
}