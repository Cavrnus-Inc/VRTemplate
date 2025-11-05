// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/SetupWizard/CavrnusSetupWizardWidgetBase.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

void UCavrnusSetupWizardWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (RequiredContainer)
		RequiredContainer->SetVisibility(ESlateVisibility::Collapsed);
	if (OptionalContainer)
		OptionalContainer->SetVisibility(ESlateVisibility::Collapsed);

	// Simple check to see if anything has changed
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldDelegate);
	WorldDelegate.Reset();
	WorldDelegate = FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](const UWorld* World, const UWorld::InitializationValues)
	{
		if (World && World->WorldType == EWorldType::Editor)
			OnPostWorldTriggered();
	});
}

void UCavrnusSetupWizardWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	Buttons.Empty();
}

void UCavrnusSetupWizardWidgetBase::OnPostWorldTriggered()
{
}

UCavrnusSetupWizardButton* UCavrnusSetupWizardWidgetBase::CreateButton(const EButtonTypeEnum& ButtonType)
{
	if (SetupButtonBlueprint)
	{
		auto* Button = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusSetupWizardButton>(SetupButtonBlueprint, GetWorld());

		switch (ButtonType)
		{
		case Required:
			if (ButtonContainerRequired) ButtonContainerRequired->AddChildToVerticalBox(Button);
			if (RequiredContainer) RequiredContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
		case Optional:
			if (ButtonContainerOptional) ButtonContainerOptional->AddChildToVerticalBox(Button);
			if (OptionalContainer) RequiredContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
		}
		
		Buttons.Add(Button);

		return Button;
	}

	return nullptr;
}
