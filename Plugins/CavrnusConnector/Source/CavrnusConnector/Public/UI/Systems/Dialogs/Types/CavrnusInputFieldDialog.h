// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "UI/Components/InputFields/CavrnusUIInputFieldValidated.h"
#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"
#include "CavrnusInputFieldDialog.generated.h"

/**
 * Abstract class representing a dialog widget with an input field.
 * Designed for use in the CavrnusConnector system, it extends the base dialog widget functionality to include input handling, text validation, and callback mechanisms.
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusInputFieldDialog : public UCavrnusBaseDialogWidget
{
	GENERATED_BODY()
public:
	UCavrnusInputFieldDialog* SetTitleText(const FString& InText);
	UCavrnusInputFieldDialog* SetBodyText(const FString& InText);
	UCavrnusInputFieldDialog* SetInputFieldHintText(const FString& InText);
	UCavrnusInputFieldDialog* SetConfirmButton(const FString& InText, const TFunction<void(const FString& SubmittedText)>& OnClicked);
	UCavrnusInputFieldDialog* SetDismissButton(const FString& InText, const TFunction<void()>& OnClicked = nullptr);
	UCavrnusInputFieldDialog* OnTextSubmit(const TFunction<void(const FString& Text)>& OnSubmit);
	
private:
	TFunction<void(const FString& Text)> OnSubmitDelegate;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIInputFieldValidated> InputField = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> BodyText = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> ConfirmButton = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> DismissButton = nullptr;

	FDelegateHandle InputFieldHandle = FDelegateHandle();
	FDelegateHandle ConfirmButtonHandle = FDelegateHandle();
	FDelegateHandle DismissButtonHandle = FDelegateHandle();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
