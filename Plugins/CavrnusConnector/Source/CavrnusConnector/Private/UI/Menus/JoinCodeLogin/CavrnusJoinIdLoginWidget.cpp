// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/JoinCodeLogin/CavrnusJoinIdLoginWidget.h"
#include "CavrnusFunctionLibrary.h"
#include "UI/CavrnusUI.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/RawWidgetHost/CavrnusRawWidgetHost.h"

void UCavrnusJoinIdLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (JoinCodeInputField)
	{
		JoinCodeInputField->SetHintText("Enter Join ID");
		JoinCodeInputField->SetToKeyboardFocusedCustom();
		
		JoinCodeInputField
			->SetValidationType(ECavrnusInputFieldValidationType::Early)
			->SetValidationPredicate([](const FString& Text) { return !Text.IsEmpty(); });

		JoinCodeInputField->OnValidatedTextUpdated.AddWeakLambda(this, [this] (const FText&, const bool IsValidInput)
		{
			if (TryJoinSpaceButton)
				TryJoinSpaceButton->SetEnabledState(IsValidInput);
		});
		
		JoinCodeInputField->OnValidatedTextCommitted.AddWeakLambda(this, [this] (const FText&, const bool IsValidInput)
			{
				if (!IsValidInput)
					JoinCodeInputField->SetValidationMessage("Join Id must not be empty!");
				
				if (TryJoinSpaceButton)
					TryJoinSpaceButton->SetEnabledState(IsValidInput);
			});
	}
	
	if (TryJoinSpaceButton)
	{
		TryJoinSpaceButton->SetEnabledState(false);
		TryJoinSpaceButton->OnButtonClicked.AddWeakLambda(this, [this]
		{
			if (JoinCodeInputField)
				TryJoinSpace(JoinCodeInputField->GetText());
		});
	}
}

void UCavrnusJoinIdLoginWidget::TryJoinSpace(const FString& JoinId)
{
	SetVisibility(ESlateVisibility::Hidden);
	
	UCavrnusFunctionLibrary::JoinSpace(JoinId.TrimStartAndEnd(),
	                                   [this](const FCavrnusSpaceConnection& Sc)
	                                   {
	                                   		UCavrnusUI::Get()->GenericWidgetDisplayer()->Close(this);
	                                   }, [this](const FString& Err)
	                                   {
		                                   SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		                                   TryJoinSpaceButton->SetIsEnabled(false);

	                                   	UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>()
											->SetPrimaryText("Failed to Join Space")
											->SetSecondaryText(FString::Printf(TEXT("%s"), *Err))
											->SetType(ECavrnusInfoToastMessageEnum::Warning);
	                                   });
}
