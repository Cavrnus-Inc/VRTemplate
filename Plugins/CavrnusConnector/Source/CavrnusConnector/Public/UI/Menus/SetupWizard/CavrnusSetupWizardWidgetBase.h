// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSetupWizardButton.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "CavrnusSetupWizardWidgetBase.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusSetupWizardWidgetBase : public UUserWidget
{
	GENERATED_BODY()
protected:
	enum EButtonTypeEnum
	{
		Required,
		Optional,
	};
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void OnPostWorldTriggered();
	
	UCavrnusSetupWizardButton* CreateButton(const EButtonTypeEnum& ButtonType);
	
private:
	FDelegateHandle WorldDelegate = FDelegateHandle();
	
	UPROPERTY()
	TArray<UCavrnusSetupWizardButton*> Buttons;
	
	UPROPERTY(EditAnywhere, Category = "Cavrnus")
	TSubclassOf<UCavrnusSetupWizardButton> SetupButtonBlueprint = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> RequiredContainer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> OptionalContainer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ButtonContainerRequired = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ButtonContainerOptional = nullptr;
};
