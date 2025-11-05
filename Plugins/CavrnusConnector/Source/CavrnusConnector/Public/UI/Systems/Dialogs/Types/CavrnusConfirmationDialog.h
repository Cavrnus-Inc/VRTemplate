// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"
#include "CavrnusConfirmationDialog.generated.h"

class UCavrnusUITextButton;
class UCavrnusUITextBlock;
/**
 * Dialog w/ simple confirm or dismiss options
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusConfirmationDialog : public UCavrnusBaseDialogWidget
{
	GENERATED_BODY()
	
public:
	UCavrnusConfirmationDialog* SetTitleText(const FString& InText);
	UCavrnusConfirmationDialog* SetBodyText(const FString& InText);
	UCavrnusConfirmationDialog* SetConfirmButton(const FString& InText, const TFunction<void()>& OnClicked);
	UCavrnusConfirmationDialog* SetDismissButton(const FString& InText, const TFunction<void()>& OnClicked);
	
private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> BodyText = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> ConfirmButton = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextButton> DismissButton = nullptr;
	
	FDelegateHandle ConfirmButtonHandle = FDelegateHandle();
	FDelegateHandle DismissButtonHandle = FDelegateHandle();
	
	virtual void NativeDestruct() override;
};