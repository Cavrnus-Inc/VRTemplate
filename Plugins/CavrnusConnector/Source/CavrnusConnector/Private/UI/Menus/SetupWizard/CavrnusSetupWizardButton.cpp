// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/SetupWizard/CavrnusSetupWizardButton.h"
#include "Components/PanelWidget.h"

UCavrnusSetupWizardButton* UCavrnusSetupWizardButton::SetPrimaryText(const FString& InPrimaryText)
{
	if (PrimaryText)
		PrimaryText->SetText(FText::FromString(InPrimaryText));
	
	return this;
}

UCavrnusSetupWizardButton* UCavrnusSetupWizardButton::SetSecondaryText(const FString& InPrimaryText)
{
	if (SecondaryText)
		SecondaryText->SetText(FText::FromString(InPrimaryText));
	
	return this;
}

UCavrnusSetupWizardButton* UCavrnusSetupWizardButton::SetCompletionState(const bool bIsComplete)
{
	if (Checkmark)
	{
		ShowCheckMarkContainer(true);
		Checkmark->SetVisibility(bIsComplete ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	return this;
}

UCavrnusSetupWizardButton* UCavrnusSetupWizardButton::ShowCheckMarkContainer(const bool bShow)
{
	if (CheckmarkContainer)
		CheckmarkContainer->SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	return this;
}

void UCavrnusSetupWizardButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	ShowCheckMarkContainer(false);
	SetCompletionState(false);
}