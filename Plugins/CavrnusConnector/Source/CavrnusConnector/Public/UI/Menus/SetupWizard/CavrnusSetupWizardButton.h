// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusSetupWizardButton.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusSetupWizardButton : public UCavrnusUIButton
{
	GENERATED_BODY()
public:
	UCavrnusSetupWizardButton* SetPrimaryText(const FString& InPrimaryText);
	UCavrnusSetupWizardButton* SetSecondaryText(const FString& InPrimaryText);
	UCavrnusSetupWizardButton* SetCompletionState(const bool bIsComplete);
	UCavrnusSetupWizardButton* ShowCheckMarkContainer(const bool bShow);

protected:
	virtual void NativePreConstruct() override;

private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> PrimaryText = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> SecondaryText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> Checkmark = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> CheckmarkContainer = nullptr;
};
